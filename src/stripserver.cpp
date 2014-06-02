#include "stripserver.h"

#include <pthread.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include <cstdlib>
#include <cstring>
#include <cassert>
#include <sstream>
#include <cmath>
#include "utils.h"
#include "message.h"
#include "timer.h"
#include "dataserverthreadsafe.h"
#include "dbufferthreadsafe.h"
#include "log.h"
#include "diskmgr.h"


void *ThreadPerClient_(void *arg)
{
	Pthread_detach(pthread_self());
	struct RequestArgs *ptr = (struct RequestArgs *)arg;
	ptr->server->ThreadPerClient(ptr);
	free(ptr);
	return (void *)NULL;
}

void *ThreadEvent_(void *arg)
{
	Pthread_detach(pthread_self());
	((StripServer *)arg)->ThreadEvent();
	return (void *)NULL;
}

StripServer::StripServer(int serverid, ConfigStrip *config) : Server(serverid), m_config(config)
{
	m_take_sample_frequency = config->sampleFrequency;	// 采样周期，秒
	m_server_band = config->serverBand;					// 服务器带宽
	m_client_band = config->clientBand;					// 客户端带宽
	m_p2p = config->isP2POpen;							// p2p是否开启
	m_real_device = config->isUseRealDevice;			// 是否读取真实磁盘
	m_port = config->serverPort;						// 服务器端口
	m_clientport = config->clientPort;					// 客户端端口
	m_place_strategy = config->placeStrategy;			// 分块仿真策略
	m_disk_band = config->diskBand;						// 不同分片大小下的磁盘带宽，单线程
	assert(m_place_strategy == "rr" || m_place_strategy == "ram" || m_place_strategy == "fdrr");

	m_block_size = config->blockSize;					// 分块大小，MB
	m_block_num = config->serverBlockNum;				// 服务器端缓冲块数目
	m_period = config->period;							// 算法周期
	m_lrfulambda = config->lrfuLambda;					// lrfu算法的lambda
	m_buffer = new DBufferThreadSafe(m_block_size, m_block_num, m_period, m_lrfulambda, config->serverStrategy);

	m_filenum = config->sourceNums;		// 资源数目
	m_min_length = config->minLength;	// 文件最小长度
	m_max_length = config->maxLength;	// 文件最大长度
	m_min_bitrate = config->minBitRate;	// 文件最小码率
	m_max_bitrate = config->maxBitRate;	// 文件最大码率
	m_diskNum = config->diskNumber;		// 磁盘数目
	m_data_server = new DataServerThreadSafe(m_filenum, m_min_length, m_max_length, m_block_size, m_min_bitrate, m_max_bitrate, m_diskNum);

	m_total_request = 0;
	m_read_from_server = 0;
	m_buffer_hit = 0;
	m_buffer_miss = 0;
	m_linkcount = 0;

	m_current_load = 0;
	// m_real_load = 0;
	Pthread_mutex_init(&m_facktran_mutex, NULL);
	pthread_cond_init(&m_facktran_cond, NULL);

	for(int i = 0;i <= MAX_CLIENT_NUM;i++){
		m_clientinfo[i].recvFd = -1;
	}

	m_diskMgr = new DiskMgr(config);

	m_buffer_ofs.open("data/buffer.log", ios::out);
	m_access_balance_degree_ofs.open("data/balance_degree.log", ios::out);
}

void StripServer::Run()
{
	int socketid = Socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serveraddr;
	TimerEvent timer_event;
	pthread_t tid;

	m_event_fd = epoll_create(MAX_LISTEN_NUM);
	epoll_event ev;

	// 初始化 m_buffer_reset_fd
	Socketpair(AF_UNIX, SOCK_STREAM, 0, m_buffer_reset_fd);
	ev.data.fd = m_buffer_reset_fd[0];
	ev.events = EPOLLIN;
	epoll_ctl(m_event_fd, EPOLL_CTL_ADD, m_buffer_reset_fd[0], &ev);

	// 初始化 m_facktran_fd
	// Socketpair(AF_UNIX, SOCK_STREAM, 0, m_facktran_fd);
	// ev.data.fd = m_facktran_fd[0];
	// ev.events = EPOLLIN;
	// epoll_ctl(m_event_fd, EPOLL_CTL_ADD, m_facktran_fd[0], &ev);

	// 初始化 m_take_sample_fd
	Socketpair(AF_UNIX, SOCK_STREAM, 0, m_take_sample_fd);
	ev.data.fd = m_take_sample_fd[0];
	ev.events = EPOLLIN;
	epoll_ctl(m_event_fd, EPOLL_CTL_ADD, m_take_sample_fd[0], &ev);

	Pthread_create(&tid, NULL, ThreadEvent_, this);

	// 注册第一次buffer reset事件
	timer_event.sockfd = m_buffer_reset_fd[1];
	timer_event.left_time = m_period * TIMER_SCALE;
	Timer::GetTimer()->RegisterTimer(timer_event);

	// 注册第一次take sample事件
	timer_event.sockfd = m_take_sample_fd[1];
	timer_event.left_time = m_take_sample_frequency * TIMER_SCALE;
	Timer::GetTimer()->RegisterTimer(timer_event);

	// 监听服务器端口，开始服务客户端
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(m_port);

	// 设置套接字选项，避免出现地址正在使用而无法绑定的错误
	int val = 1;
	setsockopt(socketid,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));

	Bind(socketid, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	Listen(socketid, 3);
	struct RequestArgs* args;
	int connfd;
	socklen_t clilen;
	while (true) {
		args = (struct RequestArgs *)Malloc(sizeof(struct RequestArgs));
		bzero(&args->cliaddr, sizeof(args->cliaddr));
		clilen = sizeof(args->cliaddr);
		connfd = Accept(socketid, (struct sockaddr *)&(args->cliaddr), &clilen);
		args->server = this;
		args->connfd = connfd;
		Pthread_create(&tid, NULL, ThreadPerClient_, args);
	}
}

StripServer::~StripServer()
{
	for(int i = 0;i <= MAX_CLIENT_NUM;i++) {
		if(m_clientinfo[i].recvFd != -1){
			close(m_clientinfo[i].recvFd);
		}
	}
	delete m_buffer;
	delete m_data_server;
	m_buffer_ofs.close();
	m_access_balance_degree_ofs.close();
	Pthread_mutex_destroy(&m_facktran_mutex);
	pthread_cond_destroy(&m_facktran_cond);
}

void StripServer::ThreadPerClient(struct RequestArgs *arg)
{
	int length;
	char buffer[MESSAGELEN];
	char response[MESSAGELEN];
	int *ptr;
	int * const resptr = (int *)response;
	int clientid;
	int fileid, ofileid;
	int segid, osegid;
	int linkednum;
	double delay;
	int best_clientid;
	int connfd = arg->connfd;
	struct sockaddr_in cliaddr = arg->cliaddr;
	int port;
	double bitrate;
	int segnum;
	//long long left_time;
	//long long sleep_time;
	//timeval begin_time, end_time;
	//timespec tm;
	int total_request, read_from_server, buffer_hit, buffer_miss;
	//int ret;
	//pthread_mutex_t facktran_mutex_temp;
	//pthread_cond_t facktran_cond_temp;
	while (true) {
		length = read(connfd, buffer, MESSAGELEN);
		//assert(length == MESSAGELEN);
		ptr = (int *)buffer;
		switch(ptr[0]) {
		case MSG_CLIENT_JOIN:
			clientid = ptr[1];
			LOG_INFO("server receive MSG_CLIENT_JOIN from " << clientid << " and response MSG_JOIN_ACK");
			m_clientinfo[clientid].address.sin_addr = cliaddr.sin_addr;
			port = m_clientport + clientid - 1;
			m_clientinfo[clientid].address.sin_port = htons(port);
			m_clientinfo[clientid].recvFd = connfd;
			++m_linkcount;
			resptr[0] = MSG_JOIN_ACK;
			resptr[1] = 0;
			length = send(connfd, response, MESSAGELEN, 0);
			assert(length == MESSAGELEN);
			break;

		case MSG_CLIENT_LEAVE:
			clientid = ptr[1];
			LOG_INFO("server receive MSG_CLIENT_LEAVE from " << clientid);
			break;

		case MSG_DELETE_SEG: //客户端缓冲区删除某一缓存块
			clientid = ptr[1];
			fileid = ptr[2];
			segid = ptr[3];
			LOG_INFO("server receive MSG_DELETE_SEG from " << clientid <<
				" and delete the fileId:" << fileid << ",segId:" << segid <<
				" from database");
			m_data_server->DeleteFromIndex(fileid,segid,clientid);
			break;

		case MSG_ADD_SEG:
			clientid = ptr[1];
			fileid = ptr[2];
			segid = ptr[3];
			linkednum = ptr[4];
			LOG_INFO("server receive MSG_ADD_SEG from " << clientid <<
				" and add the fileId:" << fileid << ",segId:" << segid <<
				" into database");
			m_data_server->InsertIntoIndex(fileid,segid,clientid,linkednum);
			break;
		
		case MSG_CLIENT_DELAY:
			clientid = ptr[1];
			ptr += 2;
			delay = *((double *)ptr);
			LOG_INFO("server receive MSG_CLIENT_DELAY from " << clientid <<
				" delay time: " << delay);
			break;

		case MSG_SEG_ASK:
			clientid = ptr[1];
			fileid = ptr[2];
			segid = ptr[3];
			++m_total_request;
			m_data_server->IncreaseTotalRequest();
			LOG_INFO("server receive MSG_SEG_ASK from " << clientid << " ask file: " << fileid << " seg: " << segid);
			if (m_p2p) { // P2P开启
				best_clientid = m_data_server->SearchBestClient(fileid, segid);
				LOG_INFO("server find best client " << best_clientid);
				if(best_clientid != -1){
					resptr[0] = MSG_REDIRECT;
					resptr[1] = 0;
					resptr[2] = best_clientid;
					resptr[3] = m_clientinfo[best_clientid].address.sin_addr.s_addr;
					resptr[4] = m_clientinfo[best_clientid].address.sin_port;
					m_buffer->DeleteVistors(fileid, clientid);
					length = send(connfd, response, MESSAGELEN, 0);
					assert(length == MESSAGELEN);
					LOG_INFO("server response  MSG_REDIRECT to " << best_clientid);
					break;
				}
			}

			// 此时表示没有找到合适的P2P服务客户端，因此从服务器直接读取文件块
			++m_read_from_server;
			m_data_server->IncreaseReadFromServer();
			m_buffer->AddVistors(fileid, clientid);
			m_data_server->GetFileInfo(fileid, &bitrate, &segnum);
			if(segid > segnum){
				LOG_INFO("client " << clientid << " read segid " << segid << " while segnum is " << segnum);
				assert(0);
			}
			// 如果命中，则不需要读取磁盘
			if (m_buffer->Read(fileid, segid)) {
				// 命中缓冲区
				++m_buffer_hit;
				m_data_server->IncreaseBufferHit();
			}
			else {
				// 读取磁盘，增加相应磁盘访问计数，磁盘id从0开始计数
				int diskId = m_data_server->GetDiskId(fileid, segid, m_place_strategy);
				m_data_server->IncreaseDiskAccessCount(diskId);
				LOG_INFO("server read fileid " << fileid << " segid " << segid << " for client " << clientid << " start");
				LOG_INFO("server read diskid " << diskId << " start");

				if(m_real_device) { // 读取真实磁盘
					// 因为Readseg接口的文件id和段id是从0开始的
					m_diskMgr->ReadSeg(fileid - 1, segid -1);
				}else{				// 模拟读取磁盘,需要提供磁盘id
					m_diskMgr->ReadSeg(fileid - 1, segid - 1, diskId);
				}
				LOG_INFO("server read diskid " << diskId << " end");
				LOG_INFO("server read fileid " << fileid << " segid " << segid << " for client " << clientid << " end");
				m_buffer->Write(fileid, segid, ofileid, osegid);
				++m_buffer_miss;
				m_data_server->IncreaseBufferMiss();
				LOG_INFO("server DELETE " << ofileid << "," << osegid << " and ADD " << fileid << "," << segid);
				
			}

			resptr[0] = MSG_SEG_ACK;
			resptr[1] = 0;
			resptr[2] = segnum;
			*((double *)(resptr + 3)) = bitrate;
			send(connfd, response, MESSAGELEN, 0);
			total_request = m_data_server->GetTotalRequest();
			read_from_server = m_data_server->GetReadFromServer();
			buffer_hit = m_data_server->GetBufferHit();
			buffer_miss = m_data_server->GetBufferMiss();
			LOG_INFO("server response MSG_SEG_ACK segnum: " << segnum << " total: " << m_total_request << " fromServer: " << m_read_from_server << " bufMiss: " << m_buffer_miss << " bufHit: " << m_buffer_hit);
			LOG_INFO("server response MSG_SEG_ACK segnum: " << segnum << " total: " << total_request << " fromServer: " << read_from_server << " bufMiss: " << buffer_miss << " bufHit: " << buffer_hit);
			LOG_WRITE(clientid << "\t" << fileid << "\t" << segid << "\t" << m_total_request << "\t" << m_read_from_server << "\t" << m_buffer_hit << "\t" << m_buffer_miss, m_buffer_ofs);
			LOG_WRITE(clientid << "\t" << fileid << "\t" << segid << "\t" << total_request << "\t" << read_from_server << "\t" << buffer_hit << "\t" << buffer_miss, m_buffer_ofs);
			break;
			
		case MSG_REQUEST_SEG:
			// config->isSpecial 为false时下面才运行
			clientid = ptr[1];
			fileid = ptr[2];
			segid = ptr[3];
			LOG_INFO("server receive MSG_REQUEST_SEG from " << clientid << " fileid: " << fileid << " segid: " << segid);

			// 向客户端发送传送完毕消息
			resptr[0] = MSG_SEG_FIN;
			resptr[1] = 0;
			resptr[2] = fileid;
			resptr[3] = segid;
			length = send(m_clientinfo[clientid].recvFd, response, MESSAGELEN, 0);
			LOG_INFO("server send MSG_SEG_FIN to " << clientid << " fileid: " << fileid << " segid: " << segid);

			//LOG_INFO("server receive MSG_REQUEST_SEG from " << clientid << " fileid: " << fileid << " segid: " << segid);
			//m_data_server->IncreaseRealLoad();
			//LOG_INFO("current_load: " << m_current_load << ", real_load: " << m_data_server->GetRealLoad());
			//// 加锁
			//Pthread_mutex_lock(&m_facktran_mutex);
			//// m_real_load++;
			//while(m_current_load >= m_server_band / m_client_band){
			//	// LOG_INFO("current_load: " << m_current_load << ", real_load: " << m_data_server->GetRealLoad());
			//	LOG_INFO("pthread_cond_wait start. current_load: " << m_current_load << ", real_load: " << m_data_server->GetRealLoad());
			//	pthread_cond_wait(&m_facktran_cond, &m_facktran_mutex);
			//	LOG_INFO("pthread_cond_wait wake.");
			//}
			//LOG_INFO("server for " << clientid << " now.");
			//m_current_load++;
			//LOG_INFO("current_load: " << m_current_load << ", real_load: " << m_data_server->GetRealLoad());
			//// 此处计算时间时，忽略服务器负载，只考虑客户端负载
			//left_time = (long long)(m_block_size * 1.0 * 1024 * 1024 / 1000 / 1000 / (m_client_band * 1.0 / 8 ) * TIMER_SCALE);// 0.8s
			//// Pthread_mutex_init(&facktran_mutex_temp, NULL);
			//pthread_cond_init(&facktran_cond_temp, NULL);
			//gettimeofday(&begin_time, NULL);
			//tm.tv_nsec = (left_time % TIMER_SCALE + begin_time.tv_usec) * 1000;
			//tm.tv_sec = left_time / TIMER_SCALE + begin_time.tv_sec;
			//tm.tv_sec = tm.tv_nsec / 1000000000 + tm.tv_sec;
			//tm.tv_nsec = tm.tv_nsec % 1000000000;
			//ret = pthread_cond_timedwait(&facktran_cond_temp, &m_facktran_mutex, &tm);
			//if(ret == ETIMEDOUT){
			//	m_current_load--;
			//	// m_real_load--;
			//	pthread_cond_signal(&m_facktran_cond);
			//	Pthread_mutex_unlock(&m_facktran_mutex);
			//	m_data_server->DecreaseRealLoad();
			//	LOG_INFO("current_load: " << m_current_load << ", real_load: " << m_data_server->GetRealLoad());
			//	// 向客户端发送传送完毕消息
			//	resptr[0] = MSG_SEG_FIN;
			//	resptr[1] = 0;
			//	resptr[2] = fileid;
			//	resptr[3] = segid;
			//	length = send(m_clientinfo[clientid].recvFd, response, MESSAGELEN, 0);
			//	LOG_INFO("server send MSG_SEG_FIN to " << clientid << " fileid: " << fileid << " segid: " << segid);
			//	assert(length == MESSAGELEN);
			//	break;
			//}else{
			//	// 即pthread_cond_timedwait返回时必定是超时返回，否则认为出错
			//	LOG_INFO("pthread_cond_timedwait error");
			//	cout << "pthread_cond_timedwait error" << endl;
			//	assert(0);
			//}
			break;

		case MSG_SEG_FIN:
			clientid = ptr[1];
			fileid = ptr[2];
			segid = ptr[3];
			LOG_INFO("server receive MSG_SEG_FIN from " << clientid << " fileid: " << fileid << " segid: " << segid);
			break;

		default:
			LOG_INFO("server receive unknow message");
			//assert(0);
		} // switch
	} // while
}

void StripServer::ThreadEvent()
{
	epoll_event events[MAX_LISTEN_NUM];
	int nfds;
	int i;
	char buffer[MESSAGELEN];
	int length;
	while(true) {
		nfds = epoll_wait(m_event_fd, events, MAX_LISTEN_NUM, -1);
		for(i = 0; i < nfds; ++i) {
			if (events[i].data.fd == m_buffer_reset_fd[0]) {
				length = recv(events[i].data.fd, buffer, MESSAGELEN, 0);
				assert(length == MESSAGELEN);
				BufferReset();
			}
			else if(events[i].data.fd == m_facktran_fd[0]){
				// 表示一个段传输完毕
				// 1.更新负载（加锁）
				// 2.向客户端发送消息
				// 加锁
				// Pthread_mutex_lock(&m_facktran_mutex);
				// m_current_load--;
				// Pthread_mutex_unlock(&m_facktran_mutex);
				// // 释放锁
				// Timer::GetTimer()->ModifyTimers(m_current_load);

				// length = recv(events[i].data.fd, buffer, MESSAGELEN, 0);
				// assert(length == MESSAGELEN);
				// int *ptr = (int *)buffer;
				// int clientid = ptr[0];
				// int fileid = ptr[1];
				// int sedid = ptr[2];

				// ptr[0] = MSG_SEG_FIN;
				// ptr[1] = 0;
				// ptr[2] = fileid;
				// ptr[3] = sedid;
				// length = send(m_clientinfo[clientid].recvFd, buffer, MESSAGELEN, 0);
				// assert(length == MESSAGELEN);
			}
			else if(events[i].data.fd == m_take_sample_fd[0]){
				length = recv(events[i].data.fd, buffer, MESSAGELEN, 0);
				assert(length == MESSAGELEN);
				TakeSample();
			}
			else {
				assert(0);
			}
		}
	}
}

void StripServer::BufferReset() 
{
	LOG_INFO("Buffer reset");
	TimerEvent event;
	event.sockfd = m_buffer_reset_fd[1];
	event.left_time = m_period * TIMER_SCALE;
	Timer::GetTimer()->RegisterTimer(event);
	m_buffer->BlockReset();
}

void StripServer::TakeSample()
{
	//int real_load = m_data_server->GetRealLoad();
	//LOG_INFO("Take Sample, current_load: " << m_current_load << ", real_load: " << real_load);
	
	//list<int> accessCountList;
	//double balance_degree_variance = m_data_server->GetAccessBalanceDegree(accessCountList);
	//double balance_degree_standard_deviation = sqrt(balance_degree_variance);
	//stringstream sstreambalance;
	//string balanceString;
	//sstreambalance.str();
	//list<int>::iterator iter;
	//for(iter=accessCountList.begin(); iter!=accessCountList.end(); iter++){
	//	sstreambalance << *iter << "\t";
	//}
	//sstreambalance << balance_degree_variance  << "\t" << balance_degree_standard_deviation;
	//balanceString = sstreambalance.str();
	//LOG_INFO("access balance degress: " << balanceString);
	//LOG_WRITE(balanceString, m_access_balance_degree_ofs);

    m_diskMgr->PrintDiskInfo();

	TimerEvent event;
	event.sockfd = m_take_sample_fd[1];
	event.left_time = m_take_sample_frequency * TIMER_SCALE;
	Timer::GetTimer()->RegisterTimer(event);
}

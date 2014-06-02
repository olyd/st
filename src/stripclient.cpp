#include "stripclient.h"

#include "timer.h"

#include "utils.h"
#include "log.h"
#include "dbufferdw.h"
#include "dbufferdwk.h"
#include "dbufferfifo.h"
#include "dbufferlfru.h"
#include "dbufferlfu.h"
#include "dbufferlrfu.h"
#include "dbufferlru.h"

#include <sstream>
#include <netinet/tcp.h>
#include <cassert>

using namespace std;

// 
void *ThreadToClient(void *arg){
	StripClient *client = (StripClient *)arg;
	client->Run();
	return NULL;
}

// 
StripClient::StripClient(int id, ConfigStrip *config):Client(id), mConfig(config){
	mMyPort = mConfig->clientPort + ((m_clientid - 1) % (mConfig->clientNums / mConfig->devNums));

	mHitTimes = 0;			//
	mTotalTimes = 0;		//
	mLinkedNums = 0;		//

	mOldFileId = -1;		//
	mOldSegId = -1;			//

	mCurrentServerId = 0;	// 默认当前服务器id为0

	mIsFirstStart = true;	//

	mDelay = false;			//
	mDelayTime = 0.0;		// 播放延迟
	mIsPreFetchFail = false;

	mIsReadFin = true;		// 预取是否下载完毕
	mIsPlaying = false;		// 是否正在播放
	mPreFetchSegId = -1;	// 预取段id
	mReadSegId = 1;			// 下一播放段id

	mBlockSize = mConfig->blockSize;

	if(mConfig->clientStrategy == "dw"){
		mDbuffer = new DBufferDW(mBlockSize, mConfig->clientBlockNum, mConfig->period);
	}
	else if(mConfig->clientStrategy == "dwk"){
		mDbuffer = new DBufferDWK(mBlockSize, mConfig->clientBlockNum, mConfig->period);
	}
	else if(mConfig->clientStrategy == "fifo"){
		mDbuffer = new DBufferFIFO(mBlockSize, mConfig->clientBlockNum);
	}
	else if(mConfig->clientStrategy == "lfru"){
		mDbuffer = new DBufferLFRU(mBlockSize, mConfig->clientBlockNum, mConfig->period);
	}
	else if(mConfig->clientStrategy == "lfu"){
		mDbuffer = new DBufferLFU(mBlockSize, mConfig->clientBlockNum);
	}
	else if(mConfig->clientStrategy == "lrfu"){
		mDbuffer = new DBufferLRFU(mBlockSize, mConfig->clientBlockNum, mConfig->lrfuLambda / 1000.0);
	}
	else if(mConfig->clientStrategy == "lru"){
		mDbuffer = new DBufferLRU(mBlockSize, mConfig->clientBlockNum);
	}
	else{
		assert(0);
	}

	for(int i=0; i<= MAX_CLIENT_NUM; ++i){
		mClientInfo[i].recvFd = -1;
	}

	stringstream sstream;
	string fileName;

	sstream.str("");
	sstream << "data/clientHit" << m_clientid << ".log";	// 客户端缓存命中日志
	fileName = sstream.str();
	mHitFs.open(fileName.c_str(),ios::out);

	sstream.str("");
	sstream << "data/client" << m_clientid << ".log";		// 客户端播放日志
	fileName = sstream.str();
	mRecordFs.open(fileName.c_str(),ios::out);

	sstream.str("");
	sstream << "data/clientdelay" << m_clientid << ".log";	// 客户端播放延迟日志
	fileName = sstream.str();
	mDelayFs.open(fileName.c_str(), ios::out);

	// 初始化套接字信息 和 epoll连接池
	Init();

	Pthread_create(&mtid, NULL, ThreadToClient, this);
}

// 
StripClient::~StripClient(){
	delete mDbuffer;
	for(int i=0; i <= MAX_CLIENT_NUM; ++i){
		if(mClientInfo[i].recvFd != -1){
			close(mClientInfo[i].recvFd);
			mClientInfo[i].recvFd = -1;
		}
	}
	close(mNetSockFd);
	close(mEpollFd);

	mIOfs.close();
	mHitFs.close();
	mRecordFs.close();
	mDelayFs.close();
}

// 
bool StripClient::Init(){
	mNetSockFd = Socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in clientAddress;
	bzero(&clientAddress, sizeof(clientAddress));
	clientAddress.sin_addr.s_addr = INADDR_ANY;
	clientAddress.sin_port = htons(mMyPort);
	clientAddress.sin_family = AF_INET;

	int val = 1;
	setsockopt(mNetSockFd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	Bind(mNetSockFd, (struct sockaddr *)&clientAddress, sizeof(clientAddress));
	Listen(mNetSockFd, MESSAGELEN);

	// 客户端监听fd加入epoll池
	mEpollFd = epoll_create(MAX_LISTEN_NUM);
	epoll_event ev;
	ev.data.fd = mNetSockFd;
	ev.events = EPOLLIN;
	epoll_ctl(mEpollFd, EPOLL_CTL_ADD, mNetSockFd, &ev);

	Socketpair(AF_UNIX, SOCK_STREAM, 0, mFackTranFd);
	ev.data.fd = mFackTranFd[0];
	ev.events = EPOLLIN;
	epoll_ctl(mEpollFd, EPOLL_CTL_ADD, mFackTranFd[0], &ev);

	Socketpair(AF_UNIX, SOCK_STREAM, 0, mPlaySockFd);
	ev.data.fd = mPlaySockFd[0];
	ev.events = EPOLLIN;
	epoll_ctl(mEpollFd, EPOLL_CTL_ADD, mPlaySockFd[0], &ev);

	Socketpair(AF_UNIX, SOCK_STREAM, 0, mBufferResetFd);
	ev.data.fd = mBufferResetFd[0];
	ev.events = EPOLLIN;
	epoll_ctl(mEpollFd, EPOLL_CTL_ADD, mBufferResetFd[0], &ev);
	return true;
}

// 读取requestFile文件,注册启动client事件
// 或者重新生成requestFile
void StripClient::RegisterStartEvent(){
	TimerEvent tv;
	tv.sockfd = mPlaySockFd[1];// 任意一个都可以

	// 从文件中读取
	if(mConfig->isRepeat){
		stringstream sstream;
		sstream.str("");
		sstream << "data/requestFile" << m_clientid << ".log";
		string requestFileName = sstream.str();
		mIOfs.open(requestFileName.c_str(), ios::in);
		if(mIOfs.fail()){
			LOG_WRITE("client " << m_clientid << ":requestFile" << m_clientid << ".log doesn't exist.", mRecordFs);
			exit(1);
		}
		// requestFile文件的格式就是如此
		mIOfs >> mFileId;
		mIOfs >> mSegId;
		mIOfs >> tv.left_time;
		mIOfs >> mSegId;// 读取第一个段ID
	}
	// 重新创建新的requestFile.log
	else{
		LOG_WRITE("config's isRepeat = false,exit.", mRecordFs);
		exit(1);
	}
	// 如果是一起启动，则启动时间都为0
	if(mConfig->isStartTogether){
		tv.left_time = 0;
	}
	Timer::GetTimer()->RegisterTimer(tv);
	cout << "client " << m_clientid << " will start at " << tv.left_time << endl;
	LOG_WRITE("client " << m_clientid << " will start at " << tv.left_time, mRecordFs);
}

// 客户端开始启动
// 连接服务器，发送 MSG_CLIENT_JOIN 给服务器
void StripClient::StartClient(epoll_event event){
	cout << "client " << m_clientid << " threadid " << mtid << " start to run..." << endl;
	LOG_WRITE("client " << m_clientid << " threadid " << mtid << " start to run..." << endl, mRecordFs);
	char buffer[MESSAGELEN];
	read(event.data.fd, buffer, MESSAGELEN);

	int connectFd;// 与服务器通信的套接字
	connectFd = Socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serverAddress;
	bzero(&serverAddress, sizeof(serverAddress));
	serverAddress.sin_addr.s_addr = inet_addr((mConfig->serverAddress).c_str());
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(mConfig->serverPort);

	int flag = 1;
	setsockopt(connectFd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

	Connect(connectFd, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr));

	// 将与服务器通信的套接字加入epoll监听池
	epoll_event ev;
	ev.data.fd = connectFd;
	ev.events = EPOLLIN;
	epoll_ctl(mEpollFd, EPOLL_CTL_ADD, connectFd, &ev);

	// 保存与服务器通信的套接字到mClientInfo数组
	mClientInfo[0].recvFd = connectFd;
	mClientInfo[0].address = serverAddress;

	// 向服务器发送 MSG_CLIENT_JOIN 
	int *ptr = (int *)buffer;
	*ptr++ = MSG_CLIENT_JOIN;
	*ptr++ = m_clientid;
	send(connectFd, buffer, MESSAGELEN, 0);
	LOG_WRITE("client " << m_clientid << " send MSG_CLIENT_JOIN to server", mRecordFs);

	// buffer reset
	if(mDbuffer->IsBlockReset()){
		TimerEvent tv;
		tv.sockfd = mBufferResetFd[1];
		tv.left_time = mConfig->period * TIMER_SCALE;
		Timer::GetTimer()->RegisterTimer(tv);
		LOG_WRITE("client " << m_clientid << " reset buffer first time", mRecordFs);
	}
}


void StripClient::PlayInFackTran(){
	char buffer[MESSAGELEN];
	int *ptr;

	if(!mDelay){
		mDelay = true;
		gettimeofday(&mDelayBeginTime, NULL);
	}

	mPreFetchSegId = mSegId;
	mSegId = mReadSegId;
	bool isInBuffer = mDbuffer->Read(mFileId,mSegId);
	// 如果 在缓存中 且 不是预取成功的，则算 命中
	if(isInBuffer && mSegId != mPreFetchSegId && mPreFetchSegId != -1){
		mHitTimes++;
	}else if(!isInBuffer && mSegId != mPreFetchSegId){
		mTotalTimes--;
	}

	// 预取成功，开始播放
	if(isInBuffer){
		// 注册播放事件
		TimerEvent tv;
		tv.sockfd = mPlaySockFd[1];
		tv.left_time = mBlockSize * 1.0 * 1024 / 1000 * 1024 / 1000 * 8 * TIMER_SCALE / mBitRate;
		Timer::GetTimer()->RegisterTimer(tv);
		LOG_WRITE("client " << m_clientid << " play fileId:" << mFileId << ",segId:" << mSegId << ",playtime:" << tv.left_time << ",HitTimes:" << mHitTimes << ",TotalTimes:" << mTotalTimes, mRecordFs);
		mIsPlaying = true;
		mHitFs << mTotalTimes << "\t" << mHitTimes << endl;
		
		if(mIsPreFetchFail){
			gettimeofday(&mDelayEndTime,NULL);
			mDelayTime = ((mDelayEndTime.tv_sec - mDelayBeginTime.tv_sec) +
					(mDelayEndTime.tv_usec - mDelayBeginTime.tv_usec) / 1000000.0);			
		}else{
			mDelayTime = 0;
		}
		mDelay = false;
		mIsPreFetchFail = false;
		// if(mDelayTime < 0.1)
		// 	mDelayTime = 0;
		// double *dptr;
		// send MSG_CLIENT_DELAY to server or directly write to file
		// ptr = (int *)buffer;
		// *ptr++ = MSG_CLIENT_DELAY;
		// *ptr++ = m_clientid;
		// dptr = (double *)ptr;
		// *dptr = mDelayTime;
		// send(mClientInfo[0].recvFd, buffer, MESSAGELEN, 0);
		mDelayFs.setf(ios::fixed, ios::floatfield);
		mDelayFs.precision(2);
		mDelayFs << mDelayTime << " " << getRelativeTime() << endl;

		// 从文件中读取下一个段id
		bool toFileEnd =false;
		if(!(mIOfs >> mReadSegId)){
			LOG_WRITE("read to end, go to begin...", mRecordFs);
			mIOfs.close();
			stringstream sstream;
			sstream.str("");
			sstream << "data/requestFile" << m_clientid << ".log";
			string requestFileName = sstream.str();
			mIOfs.open(requestFileName.c_str(), ios::in);
			mIOfs >> mReadSegId;
			mIOfs >> mReadSegId;
			mIOfs >> mReadSegId;
			mIOfs >> mReadSegId;
			toFileEnd = true;
		}

		// 预取下一个段id
		if(toFileEnd){
			mSegId = mReadSegId;
		}else{
			mSegId++;			
		}
		if(mSegId > mMaxSegId){
			mSegId = 1;
		}
		isInBuffer = mDbuffer->Read(mFileId, mSegId);
		if(mIsReadFin && !isInBuffer){
			mTotalTimes++;
			mIsReadFin = false;
			ptr = (int *)buffer;
			*ptr++ = MSG_SEG_ASK;
			*ptr++ = m_clientid;
			*ptr++ = mFileId;
			*ptr++ = mSegId;
			send(mClientInfo[0].recvFd, buffer, MESSAGELEN, 0);
			LOG_WRITE("client " << m_clientid << " send MSG_SEG_ASK fileId:" << mFileId << ",segId:" << mSegId, mRecordFs);
		}else if(isInBuffer){
			mTotalTimes++;
			if(mSegId == mReadSegId){
				mHitTimes++;
			}
		}
	}else{
		// 预取fail, need MSG_SEG_ASK
		mIsPlaying = false;
		mIsPreFetchFail = true;
		if(mIsReadFin){
			mTotalTimes++;
			mIsReadFin = false;
			ptr = (int *)buffer;
			*ptr++ = MSG_SEG_ASK;
			*ptr++ = m_clientid;
			*ptr++ = mFileId;
			*ptr++ = mSegId;
			send(mClientInfo[0].recvFd, buffer, MESSAGELEN, 0);
			LOG_WRITE("client " << m_clientid << " send MSG_SEG_ASK fileId:" << mFileId << ",segId:" << mSegId, mRecordFs);
		}
	}
}

// 
void StripClient::Run(){
	RegisterStartEvent();
	bool firstTime = true;

	epoll_event events[MAX_LISTEN_NUM];
	while(true){
		int fds = epoll_wait(mEpollFd, events, MAX_LISTEN_NUM, -1);
		if (fds < 0) {
            if (errno == EINTR){
				LOG_INFO("EINTR error:ignore and continue");
                continue;
			}
            exit(1);
        }
		if(firstTime){
			StartClient(events[0]);
			firstTime = false;
			continue;
		}
		
		for(int i=0; i<fds; i++){
			// ...
			if(events[i].data.fd == mPlaySockFd[0]){
				char buffer[MESSAGELEN];
				recv(events[i].data.fd, buffer, MESSAGELEN, 0);

				if(!mConfig->isSpecial){
					// 虚拟传输
					PlayInFackTran();
					continue;
				}
				// mOldSegId == -1 说明缓存命中，不需要 MSG_SEG_ADD MSG_SEG_DEL
				if(mOldSegId != -1){
					// 向当前为其服务的服务器发送 MSG_SEG_FIN 消息
					// mOldFileId mOldSegId
					int *ptr = (int *)buffer;
					*ptr++ = MSG_SEG_FIN;
					*ptr++ = m_clientid;
					*ptr++ = mOldFileId;
					*ptr++ = mOldSegId;
					send(mClientInfo[mCurrentServerId].recvFd, buffer, MESSAGELEN, 0);
					mCurrentServerId = 0;
					
					// 将 mOldFileId mOldSegId 写入缓冲区
					int tempFileId, tempSegId;
					mDbuffer->Write(mOldFileId, mOldSegId, tempFileId, tempSegId);
					
					// 向服务器发送 MSG_ADD_SEG 消息
					ptr = (int *)buffer;
					*ptr++ = MSG_ADD_SEG;
					*ptr++ = m_clientid;
					*ptr++ = mOldFileId;
					*ptr++ = mOldSegId;
					*ptr++ = mLinkedNums;
					send(mClientInfo[0].recvFd, buffer, MESSAGELEN, 0);
					LOG_WRITE("client " << m_clientid << " send MSG_ADD_SEG fileId:" << mOldFileId << ",segId:" << mOldSegId << ",mLinkedNums:" << mLinkedNums, mRecordFs);
					
					// 判断是否发送缓冲区替换，涉及到 MSG_DELETE_SEG 
					if(tempFileId!= -1 && tempSegId != -1){
						ptr = (int *)buffer;
						*ptr++ = MSG_DELETE_SEG;
						*ptr++ = m_clientid;
						*ptr++ = tempFileId;
						*ptr++ = tempSegId;
						send(mClientInfo[0].recvFd, buffer, MESSAGELEN, 0);
						LOG_WRITE("client " << m_clientid << " send MSG_DELETE_SEG fileId:" << tempFileId << ",segId:" << tempSegId, mRecordFs);
					}
				}

				mSegId = mReadSegId;
				bool isInBuffer = mDbuffer->Read(mFileId,mSegId);
				mTotalTimes++;
				if(!isInBuffer){
					if(!mIsFirstStart){
						// MSG_SEG_ASK
						// type m_clientid mFileId mSegId
						int *ptr = (int *)buffer;
						*ptr++ = MSG_SEG_ASK;
						*ptr++ = m_clientid;
						*ptr++ = mFileId;
						*ptr++ = mSegId;
						send(mClientInfo[0].recvFd, buffer, MESSAGELEN, 0);
						LOG_WRITE("client " << m_clientid << " send MSG_SEG_ASK fileId:" << mFileId << ",segId:" << mSegId, mRecordFs);
					}
					mIsFirstStart = false;
					// 保存旧的 mFileId mSegId,以供下次写入缓冲区
					mOldFileId = mFileId;
					mOldSegId = mSegId;
				}else{
					mHitTimes++;
					mOldSegId = -1;
				}

				// 注册播放事件
				TimerEvent tv;
				tv.sockfd = mPlaySockFd[1];
				tv.left_time = mBlockSize * 8 * TIMER_SCALE / mBitRate;
				Timer::GetTimer()->RegisterTimer(tv);
				LOG_WRITE("client " << m_clientid << " play fileId:" << mFileId << ",segId:" << mSegId << ",playtime:" << tv.left_time << ",HitTimes:" << mHitTimes << ",TotalTimes:" << mTotalTimes, mRecordFs);
				// LOG_WRITE(mTotalTimes << "\t" << mHitTimes, mHitFs);// 保持与旧版输出格式相同
				mHitFs << mTotalTimes << "\t" << mHitTimes << endl;

				// 从文件中读取下一个段id
				mIOfs >> mReadSegId;
			}
			// P2P打开的情况下
			// 新的客户端连接进来，需要执行accept
			// 保存连接进来的客户端的套接字信息，加入epoll监听池
			// 收到连接进来的客户端的 MSG_CLIENT_JOIN 消息
			else if(events[i].data.fd == mNetSockFd){
				struct sockaddr_in clientAddress;
				socklen_t length;
				length = sizeof(clientAddress);
				int clientFd = Accept(mNetSockFd, (struct sockaddr *)&clientAddress, &length);

				epoll_event ev;
				ev.data.fd = clientFd;
				ev.events = EPOLLIN;
				epoll_ctl(mEpollFd, EPOLL_CTL_ADD, clientFd, &ev);

				char buffer[MESSAGELEN];
				recv(clientFd, buffer, MESSAGELEN, 0);

				int *ptr = (int *)buffer;
				ptr++;// type
				int clientId = *ptr++;
				int port = mConfig->clientPort + ((clientId - 1) % (mConfig->clientNums / mConfig->devNums));
				if(mClientInfo[clientId].recvFd == -1){
					mClientInfo[clientId].address.sin_addr = clientAddress.sin_addr;
					mClientInfo[clientId].address.sin_port = htons(port);
					mClientInfo[clientId].recvFd = clientFd;
				}
				// 处理 MSG_CLIENT_JOIN 消息
				DealWithMessage(buffer);
			}
			// 虚拟传输
			// 传输给 clientId 的 (fileId,segId) 传输完毕
			else if(events[i].data.fd == mFackTranFd[0]){
				mLinkedNums--;

				char buffer[MESSAGELEN];
				int length = recv(events[i].data.fd, buffer, MESSAGELEN, 0);
				assert(length == MESSAGELEN);
				int *ptr = (int *)buffer;
				int clientId = ptr[1];
				int fileId = ptr[2];
				int segId = ptr[3];

				ptr[0] = MSG_SEG_FIN;
				ptr[1] = m_clientid;
				ptr[2] = fileId;
				ptr[3] = segId;
				length = send(mClientInfo[clientId].recvFd, buffer, MESSAGELEN, 0);
				assert(length == MESSAGELEN);
				LOG_WRITE("client " << m_clientid << " server for " << clientId << "end, mLinkedNums: " << mLinkedNums, mRecordFs);
			}
			// 缓冲区重置
			else if(events[i].data.fd == mBufferResetFd[0]){
				char buffer[MESSAGELEN];
				int length = recv(events[i].data.fd, buffer, MESSAGELEN, 0);
				assert(length == MESSAGELEN);
				BufferReset();
				LOG_WRITE("client " << m_clientid << " reset buffer", mRecordFs);
			}
			// 收到服务器或客户端的其他消息
			else{
				char buffer[MESSAGELEN];
				int length = recv(events[i].data.fd, buffer, MESSAGELEN, 0);
				assert(length == MESSAGELEN);
				DealWithMessage(buffer);
			}
		}
	}
}

// 
void StripClient::DealWithMessage(char *buf){
	int *ptr = (int *)buf;
	int type = *ptr++;
	int length;
	switch(type){
		// 客户端收到服务器端的 MSG_JOIN_ACK 消息
		// (type, ackjoinclientid, )
		// 向服务器端发送 MSG_SEG_ASK 消息：查询段
		case MSG_JOIN_ACK:{
			int clientId = *ptr++;

			// first segid, need ++, else not ++
			if(!mConfig->isSpecial && mCurrentServerId == 0){
				mTotalTimes++;
			}

			char buffer[MESSAGELEN];
			ptr = (int *)buffer;
			*ptr++ = MSG_SEG_ASK;
			*ptr++ = m_clientid;
			*ptr++ = mFileId;
			*ptr++ = mSegId;
			length = send(mClientInfo[clientId].recvFd, buffer, MESSAGELEN, 0);
			assert(length == MESSAGELEN);
			LOG_WRITE("client " << m_clientid << " recv MSG_JOIN_ACK from " << clientId << " and send MSG_SEG_ASK ,ask fileId:" << mFileId << ",segId:" << mSegId, mRecordFs);
			break;
		}
		// 客户端收到服务器端/其他客户端的 MSG_SEG_ACK 消息
		// (type, ackclientid, maxsegid, bitrate)
		// 向服务器端发送 MSG_REQUEST_SEG 消息
		case MSG_SEG_ACK:{
			int clientId = *ptr++;
			mMaxSegId = *ptr++;
			mBitRate = *((double *)ptr);

			// p2p开启时
			if(clientId != 0){
				mHitTimes++;
			}

			if(!mConfig->isSpecial){
				char buffer[MESSAGELEN];
				ptr = (int *)buffer;
				*ptr++ = MSG_REQUEST_SEG;
				*ptr++ = m_clientid;
				*ptr++ = mFileId;
				*ptr++ = mSegId;
				length = send(mClientInfo[clientId].recvFd, buffer, MESSAGELEN, 0);
				assert(length == MESSAGELEN);
				LOG_WRITE("client " << m_clientid << " recv MSG_SEG_ACK from " << clientId << " and send MSG_REQUEST_SEG ,ask fileId:" << mFileId << ",segId:" << mSegId, mRecordFs);
				break;
			}
			// 暂时没有虚拟传输的话
			if(mIsFirstStart){
				// mIsFirstStart = false;
				char buffer[MESSAGELEN];
				length = send(mPlaySockFd[1], buffer, MESSAGELEN, 0);
				assert(length == MESSAGELEN);
				break;
			}
			break;
		}
		// 收到服务器或者其他客户端的 MSG_REDIRECT 消息
		// (type, fromclient, bestclient, bestclient.s_addr, bestclient.sin_port)
		case MSG_REDIRECT:{
			int fromClientId = *ptr++;
			int bestClientId = *ptr++;
			int address = *ptr++;
			int port = *ptr++;

			// 修改当前为其服务的客户端id
			mCurrentServerId = bestClientId;

			// 说明未与最好的客户端连接
			// 建立连接，保存套接字信息，加入epoll监听池
			// 发送 MSG_CLIENT_JOIN 消息
			if(mClientInfo[bestClientId].recvFd == -1){
				struct sockaddr_in clientAddress;
				mClientInfo[bestClientId].address.sin_addr.s_addr = address;
				mClientInfo[bestClientId].address.sin_port = port;
				clientAddress.sin_addr = mClientInfo[bestClientId].address.sin_addr;
				clientAddress.sin_port = mClientInfo[bestClientId].address.sin_port;
				clientAddress.sin_family = AF_INET;
				int connectFd = Socket(AF_INET, SOCK_STREAM, 0);
				Connect(connectFd, (struct sockaddr *)&clientAddress, sizeof(struct sockaddr));

				mClientInfo[bestClientId].recvFd = connectFd;

				epoll_event ev;
				ev.data.fd = connectFd;
				ev.events = EPOLLIN;
				epoll_ctl(mEpollFd, EPOLL_CTL_ADD, connectFd, &ev);

				char buffer[MESSAGELEN];
				ptr = (int *)buffer;
				*ptr++ = MSG_CLIENT_JOIN;
				*ptr++ = m_clientid;
				length = send(connectFd, buffer, MESSAGELEN, 0);
				assert(length == MESSAGELEN);
				LOG_WRITE("client " << m_clientid << " recv MSG_REDIRECT from " << fromClientId << " and send MSG_CLIENT_JOIN to " <<  bestClientId, mRecordFs);
			}
			// 说明已经建立连接了，发送 MSG_SEG_ASK 消息
			else{
				char buffer[MESSAGELEN];
				ptr = (int *)buffer;
				*ptr++ = MSG_SEG_ASK;
				*ptr++ = m_clientid;
				*ptr++ = mFileId;
				*ptr++ = mSegId;
				length = send(mClientInfo[bestClientId].recvFd, buffer, MESSAGELEN, 0);
				assert(length == MESSAGELEN);
				LOG_WRITE("client " << m_clientid << " recv MSG_REDIRECT from " << fromClientId << " and send MSG_SEG_ASK to " <<  bestClientId << ",ask fileId:" << mFileId << ",segId:" << mSegId, mRecordFs);
			}
			break;
		}
		// 收到其他客户端的 MSG_CLIENT_JOIN 消息
		// (type, joinclientid, )
		// 已经在连接时保存了套接字信息，直接发送 MSG_JOIN_ACK 消息
		case MSG_CLIENT_JOIN:{
			int clientId = *ptr++;

			char buffer[MESSAGELEN];
			ptr = (int *)buffer;
			*ptr++ = MSG_JOIN_ACK;
			*ptr++ = m_clientid;
			length = send(mClientInfo[clientId].recvFd, buffer, MESSAGELEN, 0);
			assert(length == MESSAGELEN);
			LOG_WRITE("client " << m_clientid << " recv MSG_CLIENT_JOIN from " << clientId << " and send MSG_JOIN_ACK to " <<  clientId, mRecordFs);
			break;
		}
		// 收到其他客户端的 MSG_SEG_ASK 消息
		// (type, requestclientid, fileid, segid )
		// 判断当前客户端是否满载 或者 其他客户端请求段是否在buffer中
		case MSG_SEG_ASK:{
			int clientId = *ptr++;
			int fileId = *ptr++;
			int segId = *ptr++;

			LOG_WRITE("client " << m_clientid << " recv MSG_SEG_ASK from " << clientId << " ask file: " << fileId << " seg: " << segId, mRecordFs);
			bool isInBuffer = mDbuffer->Read(fileId, segId);
			if(mLinkedNums > MAX_CLIENT_LINKS || !isInBuffer){
				char buffer[MESSAGELEN];
				ptr = (int *)buffer;
				*ptr++ = MSG_REDIRECT;
				*ptr++ = m_clientid;
				*ptr++ = 0;
				*ptr++ = mClientInfo[0].address.sin_addr.s_addr;
				*ptr++ = mClientInfo[0].address.sin_port;
				length = send(mClientInfo[clientId].recvFd, buffer, MESSAGELEN, 0);
				assert(length == MESSAGELEN);
				LOG_WRITE("client " << m_clientid << " send MSG_REDIRECT to " <<  clientId << ",now mLinkedNums: " << mLinkedNums, mRecordFs);
				if(!isInBuffer){
					LOG_WRITE("client " << m_clientid << " do no have fileId:" << fileId
						<< " segId:" << segId, mRecordFs);
				}
				break;
			}
			else{
				mLinkedNums++;
				char buffer[MESSAGELEN];
				ptr = (int *)buffer;
				*ptr++ = MSG_SEG_ACK;
				*ptr++ = m_clientid;
				*ptr++ = mMaxSegId;
				double *dbptr = (double *)ptr;
				*dbptr = mBitRate;
				length = send(mClientInfo[clientId].recvFd, buffer, MESSAGELEN, 0);
				assert(length == MESSAGELEN);
				LOG_WRITE("client " << m_clientid << " send MSG_SEG_ACK to " <<  clientId << ", after this, mLinkedNums: " << mLinkedNums, mRecordFs);
			}
			break;
		}
		// p2p开启后收到其他客户端的 MSG_REQUEST_SEG 消息
		// config->isSpecial 为false时下面才运行
		case MSG_REQUEST_SEG:{

			int clientId = *ptr++;
			int fileId = *ptr++;
			int segId = *ptr++;

			long long leftTime = (long long)(mBlockSize / (mConfig->clientBand * 1.0 / 8 ) * TIMER_SCALE);// 0.8s
			TimerEvent tv;
			tv.sockfd = mFackTranFd[1];
			tv.left_time = leftTime;
			ptr = (int *)(tv.buffer);
			*ptr++ = MSG_SEG_FIN;
			*ptr++ = clientId;
			*ptr++ = fileId;
			*ptr++ = segId;
			Timer::GetTimer()->RegisterTimer(tv);
			LOG_WRITE("client " << m_clientid << " recv MSG_REQUEST_SEG from " << clientId << ", start to facktran fileId: " << fileId << " segId: " << segId, mRecordFs);
			break;
		}
		// 收到其他客户端发送的 文件块传输完毕 消息
		// (type 其他客户端id fileid segid)
		// 在没有虚拟传输下，指的是 收到该服务器为其他客户端服务完毕
		// 在虚拟传输下，指的是 该客户端收到为其服务的client 的传输完毕消息
		case MSG_SEG_FIN:{
			int clientId = *ptr++;
			int fileId = *ptr++;
			int segId = *ptr++;

			if(!mConfig->isSpecial){
				int oldFileId = -1;
				int oldSegId = -1;
				char buffer[MESSAGELEN];

				mIsReadFin = true;
				mDbuffer->Write(fileId, segId, oldFileId, oldSegId);
				if(!mIsPlaying){
					send(mPlaySockFd[1], buffer, MESSAGELEN, 0);
					mIsPlaying = true;
				}
				LOG_WRITE("client " << m_clientid << " recv MSG_SEG_FIN fileId:" << fileId << ",segId: " << segId << " from " << clientId, mRecordFs);

				// 向服务器发送 MSG_ADD_SEG 消息
				ptr = (int *)buffer;
				*ptr++ = MSG_ADD_SEG;
				*ptr++ = m_clientid;
				*ptr++ = fileId;
				*ptr++ = segId;
				*ptr++ = mLinkedNums;
				send(mClientInfo[0].recvFd, buffer, MESSAGELEN, 0);
				LOG_WRITE("client " << m_clientid << " send MSG_ADD_SEG fileId:" << fileId << ",segId:" << segId << ",mLinkedNums:" << mLinkedNums, mRecordFs);
					
				// 判断是否发送缓冲区替换，涉及到 MSG_DELETE_SEG 
				if(oldFileId!= -1 && oldSegId != -1){
					ptr = (int *)buffer;
					*ptr++ = MSG_DELETE_SEG;
					*ptr++ = m_clientid;
					*ptr++ = oldFileId;
					*ptr++ = oldSegId;
					send(mClientInfo[0].recvFd, buffer, MESSAGELEN, 0);
					LOG_WRITE("client " << m_clientid << " send MSG_DELETE_SEG fileId:" << oldFileId << ",segId:" << oldSegId, mRecordFs);
				}
				break;
			}else{
				mLinkedNums--;
				LOG_WRITE("client " << m_clientid << " recv MSG_SEG_FIN from " << clientId << " and delete mLinkedNums, after this, mLinkedNums: " <<  mLinkedNums, mRecordFs);
				break;
			}
		}
		// 
		default:{
			LOG_WRITE("client recv unknown message,exit...", mRecordFs);
			assert(0);
		}
	}
}


void StripClient::BufferReset(){
	TimerEvent tv;
	tv.sockfd = mBufferResetFd[1];
	tv.left_time = mConfig->period * TIMER_SCALE;
	Timer::GetTimer()->RegisterTimer(tv);
	mDbuffer->BlockReset();
}


void StripClient::Exit(){
	
}

#include "lbserver.h"
#include <cassert>
#include <iostream>
#include <algorithm>

#include <sys/epoll.h>
#include <sys/socket.h>

#include "message.h"
#include "utils.h"
#include "spreadserver.h"
#include "spread.h"
#include "timer.h"
#include "log.h"

using namespace std;

void FileInfo::AddServer(int serverId)
{
	vector<int>::iterator iter = find(mServerList.begin(), mServerList.end(), serverId);
	assert(iter == mServerList.end());
	mServerList.push_back(serverId);
}

void FileInfo::DeleteServer(int serverId)
{
	vector<int>::iterator iter = find(mServerList.begin(), mServerList.end(), serverId);
	assert(iter != mServerList.end());
	mServerList.erase(iter);
}

void FileInfo::PrintServerList()
{
	vector<int>::iterator iter;
	LOG_INFO_NOENDL("serverlist for file " << setw(3) << mFileId << ": ");
	for(iter = mServerList.begin(); iter != mServerList.end(); ++iter)
	{
		LOG_NOTIME_NOENDL(setw(2) << *iter << " ");
	}
	LOG_NOTIME_NOENDL(endl);
}

LBServer::LBServer(int serverId, ConfigType *config):Server(serverId),mConfig(config)
{
	mSpreadTimes = 0;
	mRefuseTimes = 0;
	mRequestTimes = 0;
	mDeleteTimes = 0;
	pthread_mutex_init(&mMutex,NULL);
	mSpreadServerList.clear();
	mFileInfoList.clear();
	mSpreadList.clear();
	InitSpreadServer();
	InitResource();
}

LBServer::~LBServer()
{
	for(unsigned int i=0; i<mConfig->subServerNum; i++)
		delete mSpreadServerList[i];

	for(unsigned int i=0; i<mConfig->resourceNumber; i++)
		delete mFileInfoList[i];

	for(unsigned int i=0; i<mSpreadList.size(); i++)
		delete mSpreadList[i];

	pthread_mutex_destroy(&mMutex);
}

bool LBServer::Init(ConfigType *config)
{
	return true;
}

void LBServer::InitSpreadServer()
{
	mSpreadServerList.clear();
	for(unsigned int i=0; i<mConfig->subServerNum; i++)
	{
		SpreadServer *server = new SpreadServer(i, mConfig);		
		if(!server)
		{
			// 初始扩散服务器失败，退出
			LOG_INFO("init SrpeadServer " << i << "fail, exit.");
			exit(1);
		}
		mSpreadServerList.push_back(server);
	}
}

void LBServer::InitResource()
{
	// 初始文件放置
	int ret, target;
	mFileInfoList.clear();
	for(unsigned int i=0; i<mConfig->resourceNumber; i++)
	{
		target =i % mConfig->subServerNum;
		ret = AddFile(i, target, true);// true表示第一次添加文件
		if(ret < 0)
		{
			LOG_INFO("init file " << i << "\t to spreadserver " << target << "\t fail, exit.");
			exit(1);
		}
		mFileInfoList.push_back(new FileInfo(i));
		mFileInfoList[i]->AddServer(target);
	}
	LOG_INFO("init file success.");
	// 打印此时的文件所在服务器列表
	PrintFileList();
}

void LBServer::Run()
{
	int listen_fd = tcp_listen(mConfig->serverPort.c_str());
    int epfd, i;
    int clientSize = 1000;
    struct epoll_event listen_ev, events[1000];
    epfd = epoll_create(10);
    if (epfd < 0) {
        LOG_INFO("epoll_create error.");
        exit(1);
    }
	LOG_INFO("epoll_create success.");
    listen_ev.events = EPOLLIN;
    listen_ev.data.fd = listen_fd;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &listen_ev);
	if(ret < 0)
	{
		LOG_INFO("epoll_ctl error, exit");
		exit(1);
	}
	epoll_event ev;

	Socketpair(AF_UNIX, SOCK_STREAM, 0, m_eventfd);
	ev.data.fd = m_eventfd[0];
	ev.events = EPOLLIN;
	epoll_ctl(epfd, EPOLL_CTL_ADD, m_eventfd[0], &ev);
	
	Socketpair(AF_UNIX, SOCK_STREAM, 0, m_strategy_resetfd);
	ev.data.fd = m_strategy_resetfd[0];
	ev.events = EPOLLIN;
	epoll_ctl(epfd, EPOLL_CTL_ADD, m_strategy_resetfd[0], &ev);
    
	Socketpair(AF_UNIX, SOCK_STREAM, 0, m_exitfd);
	ev.data.fd = m_exitfd[0];
	ev.events = EPOLLIN;
	epoll_ctl(epfd, EPOLL_CTL_ADD, m_exitfd[0], &ev);

	TimerEvent tv;
	tv.left_time = (long long)mConfig->period * TIMER_SCALE;
	tv.sockfd = m_strategy_resetfd[1];
	Timer::GetTimer()->RegisterTimer(tv);

	tv.left_time = (long long)mConfig->runTime * TIMER_SCALE;
	LOG_INFO("Run Time: " << tv.left_time);
	tv.sockfd = m_exitfd[1];
	Timer::GetTimer()->RegisterTimer(tv);

    int nready;
    SpreadNode spreadNode;
    char buff[MESSAGELEN];
    for (;;) {
		// LOG_INFO("start to epoll_wait...");
        nready = epoll_wait(epfd, events, clientSize, -1);
        if (nready < 0) {
            if (errno == EINTR){
				LOG_INFO("EINTR error:ignore and continue");
                continue;
			}
            exit(1);
        }
        for (i = 0; i < nready; i++) {
            if (events[i].data.fd == listen_fd) {
                AddNewClient(epfd, listen_fd);
           	} else if (events[i].data.fd == m_eventfd[0]) {
           		int length = readn(m_eventfd[0], (void *)&spreadNode, sizeof(SpreadNode));
           		assert(length == sizeof(SpreadNode));
           		FinishSpread(spreadNode.fileId, spreadNode.srcServerId, spreadNode.targetServerId, spreadNode.startTime, spreadNode.endTime);
			} else if (events[i].data.fd == m_strategy_resetfd[0]) {
				readn(m_strategy_resetfd[0], buff, MESSAGELEN);
				LOG_INFO("Spread strategy reset");
				StrategyReset();
			} else if (events[i].data.fd == m_exitfd[0]) {
				readn(m_exitfd[0], buff, MESSAGELEN);
				LOG_INFO("Exit");
				Exit();
				goto out;
			} else if (events[i].events && EPOLLIN) {
                int flag = ReadMessage(events[i].data.fd);
                if (flag == 0) {
                    close(events[i].data.fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, events + i);
                }
            } else if (events[i].events && EPOLLERR) {
                close(events[i].data.fd);
                epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, events + i);
            }
        }
    }
out:
	return;
}

// 新的客户端连接进来
int LBServer::AddNewClient(int epfd, int listenfd) {
	struct epoll_event ev;
	struct sockaddr_in cliaddr;
	int len = sizeof(struct sockaddr_in);

	int connfd = accept(listenfd, (struct sockaddr *)&cliaddr, (socklen_t *)&len);
	if(connfd < 0)
	{
		LOG_INFO("accept new client error in LBServer::AddNewClient.");
		return -1;
	}
	ev.data.fd = connfd;
	ev.events = EPOLLIN;
	// 函数应该需要判断执行成功与否
	epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
	return connfd;
}

// 
int LBServer::ReadMessage(int connfd)
{
	char buf[MESSAGELEN];
	int len = readn(connfd, (void *)buf, sizeof(Message));
	if(len < 0)
	{
		LOG_INFO("readn error in LBServer::ReadMessage.");
		return -1;
	}
	else if(len == 0)
	{
		return 0;
	}
	else
	{
		MessageProcess(connfd, buf);
		return 1;
	}
	return 0;
}

// 回复客户端消息
int LBServer::WriteMessage(int connfd, void *message)
{
	int len = writen(connfd, message, sizeof(Message));
	if(len < 0)
	{
		LOG_INFO("writen error in LBServer::WriteMessage.");
		exit(1);
	}
	return len;
}

void LBServer::MessageProcess(int connfd, void *buf)
{
	Message *recvMessage = (Message *)buf;
	int type = recvMessage->type;
	int clientId = recvMessage->clientId;
	int serverId = recvMessage->serverId;
	int fileId = recvMessage->info.fileId;
	int playLen = 0;

	Message sendMessage;
	sendMessage.clientId = clientId;

	switch(type)
	{
		case RESOURCEREQUEST:
		{
			++mRequestTimes;// 增加请求计数器
			LOG_NOTIME_NOENDL(endl);
			LOG_INFO("client " << clientId << " request file " << fileId);
			vector<int> serverList;
			// playLen 暂时未用到
			GetServerList(clientId, fileId, playLen, serverList);
			// LOG_INFO_NOENDL("serverlist for client request sorted by load:");
			serverId = SortServerByLoad(serverList);
			if(serverId >= 0)
			{
				// 找到可以服务的最佳扩散服务器
				LOG_INFO("found best server " << serverId);
				HasFoundBestServer(fileId, serverId);
				sendMessage.serverId = serverId;
			}
			else
			{
				// 未找到最佳扩散服务器，被拒绝
				LOG_INFO("cannot find a proper server, request deny.");
				sendMessage.serverId = LBSERVER;
				++mRefuseTimes;// 增加拒绝计数器
			}
			sendMessage.type = REQUESTACK;
			sendMessage.info.playLen = playLen;
			WriteMessage(connfd, (void *)&sendMessage);
			PrintTimes();
			break;
		}
		case EXIT: //客户端暂时未发送该消息
		{
			DecreaseLoad(fileId, serverId);
			sendMessage.type = EXITACK;
			sendMessage.serverId = LBSERVER;
			WriteMessage(connfd, (void *)&sendMessage);
			break;
		}
		default:
		{
			LOG_ERR("unknown message type.");
			exit(1);
		}
	}
}

// 资源初始化时调用，将 fileId 放在 serverId 的文件列表中
bool LBServer::AddFile(int fileId, int serverId, bool firsttime)
{
	return mSpreadServerList[serverId]->AddFile(fileId, firsttime);
}

bool LBServer::DeleteFile(int fileId, int serverId)
{
	mFileInfoList[fileId]->DeleteServer(serverId);
	mSpreadServerList[serverId]->DeleteFile(fileId);
	return true;
}

// SortServerByLoad所需的比较函数
bool Compare(SpreadServer *a, SpreadServer *b)
{
	if(a->GetCurrentLoad() < b->GetCurrentLoad())
		return true;
	else if(a->GetCurrentLoad() == b->GetCurrentLoad() && a->GetServerId() > b->GetServerId())
		return true;
	else return false;
}
int LBServer::SortServerByLoad( vector<int> &serverList)
{
	vector<SpreadServer *> list;
	vector<SpreadServer *>::iterator iter;

	if(serverList.size() == 0)
	{
		return -1;
	}

	for(iter = mSpreadServerList.begin(); iter != mSpreadServerList.end(); ++iter)
	{
		if(!(find(serverList.begin(), serverList.end(), (*iter)->GetServerId()) 
			== serverList.end()))
		{
			list.push_back(*iter);
		}
	}
	sort(list.begin(), list.end(), Compare);

	serverList.clear();
	LOG_INFO_NOENDL("SortServerByLoad[serverid, load]:  ");
	for(iter = list.begin(); iter != list.end(); ++iter)
	{
		LOG_NOTIME_NOENDL("[" << (*iter)->GetServerId() << "," << (*iter)->GetCurrentLoad() << "] ");
		serverList.push_back((*iter)->GetServerId());
	}
	LOG_NOTIME_NOENDL(endl);
	return serverList[0];
}

int LBServer::GetServerList(int clientId, int fileId, int &fileLen, vector<int> &serverList)
{
	LOG_INFO_NOENDL("GetServerList for file " << fileId << ":\t");
	vector<int> list = mFileInfoList[fileId]->mServerList;
	for(vector<int>::iterator iter = list.begin();
			iter != list.end(); ++iter)
	{
		if(!mSpreadServerList[*iter]->IsOverLoad()){
			serverList.push_back(*iter);
			LOG_NOTIME_NOENDL(*iter << " ");
		}
	}
	LOG_NOTIME_NOENDL(endl);
	return 0;
}

// 负载变化，检查相关的扩散线程是否需要唤醒
void LBServer::NotifySpread(int serverId, int load)
{
	for(vector<Spread *>::iterator iter = mSpreadList.begin(); iter != mSpreadList.end(); ++iter)
	{
		(*iter)->NotifySpread(serverId, load);
	}
}

// 找到存有文件且负载最小的扩散服务器
void LBServer::HasFoundBestServer(int fileId, int serverId)
{
	IncreaseLoad(fileId, serverId);// 增加服务器负载
	if( IsNeedSpread(serverId))// 检查是否需要扩散
	{
		StartSpread(serverId);
	}
}

int LBServer::IncreaseLoad(int fileId, int serverId)
{
	int ret = mSpreadServerList[serverId]->IncreaseLoad(fileId);
	NotifySpread(serverId, GetCurrentLoad(serverId));// 唤醒扩散线程
	return ret;
}

int LBServer::DecreaseLoad(int fileId, int serverId)
{
	int ret = mSpreadServerList[serverId]->DecreaseLoad(fileId);
	NotifySpread(serverId, GetCurrentLoad(serverId));// 唤醒扩散线程
	return ret;
}

int LBServer::GetCurrentLoad(int serverId)
{
	return mSpreadServerList[serverId]->GetCurrentLoad();
}

int LBServer::IncreaseCopyFlow(int fileId, int serverId)
{
	int ret = mSpreadServerList[serverId]->IncreaseCopyFlow(fileId);
	NotifySpread(serverId, GetCurrentLoad(serverId));// 唤醒扩散线程
	return ret;
}

int LBServer::DecreaseCopyFlow(int fileId, int serverId)
{
	int ret = mSpreadServerList[serverId]->DecreaseCopyFlow(fileId);
	NotifySpread(serverId, GetCurrentLoad(serverId));// 唤醒扩散线程
	return ret;
}

int LBServer::GetCurrentCopyFlow(int serverId)
{
	return mSpreadServerList[serverId]->GetCopyFlow();
}

int LBServer::IncreaseInFlow(int fileId, int serverId)
{
	int ret = mSpreadServerList[serverId]->IncreaseInFlow(fileId);
	NotifySpread(serverId, GetCurrentLoad(serverId));// 唤醒扩散线程
	return ret;
}

int LBServer::DecreaseInFlow(int fileId, int serverId)
{
	int ret = mSpreadServerList[serverId]->DecreaseInFlow(fileId);
	NotifySpread(serverId, GetCurrentLoad(serverId));// 唤醒扩散线程
	return ret;
}

bool LBServer::IsOverCapacity(int serverId)
{
	return mSpreadServerList[serverId]->IsOverCapacity();
}

bool LBServer::IsNeedSpread(int serverId)
{
	return mSpreadServerList[serverId]->IsNeedSpread();
}

bool LBServer::IsCanBeTarget(int serverId)
{
	bool ret = mSpreadServerList[serverId]->IsCanBeTarget();
	if(!ret){
		return false;
	}else{
		if(IsOverCapacity(serverId))// 如果服务器文件已满
		{
			LOG_INFO("target server " << serverId << " is over capacity.");
			vector<int> deleteList;
			LOG_INFO("get filelist that can be deleted in server " << serverId);
			GetDeleteFileList(serverId, deleteList);
			LOG_INFO("erase no-copy files from filelist in server " << serverId);
			EraseNoCopyFile(serverId, deleteList);
			if(deleteList.size() > 0)
			{
				DeleteFile(deleteList[0], serverId);
				++mDeleteTimes;
				return true;
			}
		}else
		{
			return true;
		}
	}
	return false;
}

void LBServer::StartSpread(int serverId)
{
	vector<int> fileList;
	LOG_INFO("Get need-spread filelist in server " << serverId);
	GetSpreadFile(serverId, fileList);
	for(vector<int>::iterator iter1 = fileList.begin(); iter1 != fileList.end(); ++iter1)
	{
		int fileId = *iter1;
		vector<int> noServerList;

		LOG_INFO("******file:" << fileId << "******");
		LOG_INFO("get serverlist that don't have file " << fileId);
		GetNoServerList(fileId, noServerList);
		SortServerByLoad(noServerList);
		int targetServerId = -1;
		for(vector<int>::iterator iter2 = noServerList.begin(); iter2 != noServerList.end(); ++iter2)
		{
			targetServerId = *iter2;
			LOG_INFO("try server " << setw(2) << targetServerId);
			// if(targetServerId != -1 && IsCanBeTarget(targetServerId))
			int srcLoad = GetCurrentLoad(serverId);
			int targetLoad = GetCurrentLoad(targetServerId);
				// srcLoad > targetLoad && 
			if(targetServerId != -1 && 
				IsCanBeTarget(targetServerId))
			{
				LOG_INFO("target server is " << setw(2) << targetServerId);
				BeforeSpread(fileId, serverId, targetServerId);
				CreateSpread(fileId, serverId, targetServerId);
				LOG_INFO("start to spread file " << setw(3) << fileId << \
						 " from server " << setw(2) << serverId << \
						  "(" << setw(3) << srcLoad << \
						   ") to " << setw(2) << targetServerId << \
							"(" << setw(3) << targetLoad << ")");
				return;
			}
		}
	}
	LOG_INFO("no spread happen.");
}

void LBServer::BeforeSpread(int fileId, int srcServerId, int targetServerId)
{
	LOG_INFO("before spread, do something...");
	// 扩散前
	// 将扩散文件先添加到扩散服务器上
	AddFile(fileId, targetServerId, false);
	IncreaseInFlow(fileId, targetServerId);// 增加目的端的进入流
	IncreaseCopyFlow(fileId, srcServerId);// 增加源端的复制流
}

void LBServer::CreateSpread(int fileId, int srcServerId, int targetServerId)
{
	LOG_INFO("create spread thread and run it.");
	// 创建扩散流
	SpreadNode spreadNode;
	spreadNode.fileId = fileId;
	spreadNode.srcServerId = srcServerId;
	spreadNode.targetServerId = targetServerId;
	spreadNode.srcServerLoad = GetCurrentLoad(srcServerId);
	spreadNode.targetServerLoad = GetCurrentLoad(targetServerId);
	spreadNode.currentLoad = spreadNode.srcServerLoad > spreadNode.targetServerLoad ? spreadNode.srcServerLoad : spreadNode.targetServerLoad;
	spreadNode.diskBand = mConfig->maxDiskBand;
	spreadNode.fileLength = mConfig->fileLength;
	spreadNode.lb = this;
	spreadNode.isFinished = false;
	Spread *spread = new Spread(spreadNode);
	mSpreadList.push_back(spread);
	++mSpreadTimes;// 增加扩散计数器
}

void LBServer::FinishSpread(int fileId, int srcServerId, int targetServerId, double startTime, double endTime)
{
	// 扩散后
	// mFileInfoList应该是在扩散完成后再添加，因为mFileInfoList用于找含有fileId的服务器列表
	mFileInfoList[fileId]->AddServer(targetServerId);
	LOG_INFO("file " << setw(3) << fileId << " has spreaded from " << setw(2) << srcServerId << " to " << setw(2) << targetServerId << " start " << setw(8) << startTime << " end " << setw(8) << endTime);
	DecreaseCopyFlow(fileId, srcServerId);// 减少源端的复制流
	DecreaseInFlow(fileId, targetServerId);// 减少目的端的进入流
}

int LBServer::GetNoServerList(int fileId, vector<int> &noServerList)
{
	for(unsigned int i=0; i<mConfig->subServerNum; i++)
	{
		if(!SearchFile(i, fileId))
		{
			noServerList.push_back(i);
		}
	}
	return 0;
}

bool LBServer::SearchFile(int serverId, int fileId)
{
	return mSpreadServerList[serverId]->SearchFile(fileId);
}

int LBServer::GetSpreadFile(int serverId, vector<int> &fileList)
{
	return mSpreadServerList[serverId]->GetSpreadFile(fileList);
}

int LBServer::GetDeleteFileList(int serverId, vector<int> &fileList)
{
	return mSpreadServerList[serverId]->GetDeleteFileList(fileList);
}

// 从待删除的文件列表中去除没有副本的文件
int LBServer::EraseNoCopyFile(int serverId, vector<int> &fileList)
{
	vector<int>::iterator iter;
	LOG_INFO_NOENDL("before erase, filelist which will be deleted is: ")
	for(iter=fileList.begin(); iter!=fileList.end(); ++iter)
	{
		LOG_NOTIME_NOENDL(*iter << " ");
	}
	LOG_NOTIME_NOENDL(endl);


	for(iter = fileList.begin(); iter != fileList.end(); ++iter)
	{
		vector<int> serverList = mFileInfoList[*iter]->mServerList;
		// 如果文件所在服务器列表中没有serverId,出错
		vector<int>::iterator iter2 = find(serverList.begin(), serverList.end(), serverId);
		assert(iter2 != serverList.end());
		if(serverList.size() < 2 )// 没有副本
		{
			iter = fileList.erase(iter);
			--iter;
		}
	}

	LOG_INFO_NOENDL("after erase, filelist which will be deleted is: ")
	for(iter=fileList.begin(); iter!=fileList.end(); ++iter)
	{
		LOG_NOTIME_NOENDL(*iter << " ");
	}
	LOG_NOTIME_NOENDL(endl);

	return 0;
}

inline int LBServer::GetEventFd()
{
	return m_eventfd[1];
}	

void LBServer::FinishSpreadSendMsg(void *arg)
{
	SpreadNode *spreadNode = (SpreadNode *)arg;
	pthread_mutex_lock(&mMutex);
	writen(m_eventfd[1], (void *)spreadNode, sizeof(*spreadNode));
	pthread_mutex_unlock(&mMutex);
}

void LBServer::PrintTimes()
{
	//LOG_INFO("req:" << mRequestTimes << " ref:" << mRefuseTimes << " spd:" << mSpreadTimes << " del:" << mDeleteTimes);
	LOG_INFO(mRequestTimes << " " << mRefuseTimes << " " << mSpreadTimes << " " << mDeleteTimes);
}

void LBServer::PrintFileList()
{
	LOG_INFO("current filelist in LBServer: ");
	for(unsigned int i=0; i<mConfig->resourceNumber; i++)
		mFileInfoList[i]->PrintServerList();
	
	LOG_INFO("current filelist in RBServers: ");
	for(unsigned int i=0; i<mConfig->subServerNum; i++)
		mSpreadServerList[i]->PrintFileListInServer();
}

void LBServer::PrintSpreadingFileList()
{
	LOG_INFO("spreading filelist in RBServers: ");
	for(unsigned int i=0; i<mConfig->subServerNum; i++)
		mSpreadServerList[i]->PrintSpreadingFileListInServer();

	LOG_INFO("not-finished filelist in Spreads: ");
	for(vector<Spread *>::iterator iter = mSpreadList.begin(); iter != mSpreadList.end(); ++iter)
	{
		if(!(*iter)->mSpreadNode.isFinished){
			LOG_INFO("file " << (*iter)->mSpreadNode.fileId << " is spreading from " << (*iter)->mSpreadNode.srcServerId << " to " << (*iter)->mSpreadNode.targetServerId);
		}
	}
}

void LBServer::StrategyReset()
{
	vector<SpreadServer *>::iterator iter;
	
	TimerEvent tv;
	tv.left_time = mConfig->period * TIMER_SCALE;
	tv.sockfd = m_strategy_resetfd[1];
	Timer::GetTimer()->RegisterTimer(tv);

	for(iter = mSpreadServerList.begin(); iter != mSpreadServerList.end(); ++iter)
	{
		(*iter)->Reset();
	}
}


void LBServer::Exit()
{
	PrintFileList();
	PrintSpreadingFileList();
	PrintTimes();
}

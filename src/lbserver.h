#ifndef LBServer_H
#define LBServer_H

#include <vector>

#include "server.h"
#include "config.h"


class FileInfo {
public:
	FileInfo(int fileId):mFileId(fileId){};
	void AddServer(int serverId);
	void DeleteServer(int serverId);
	void PrintServerList();
public:
	int mFileId;
	vector<int> mServerList;
};

class Spread;
class SpreadServer;

class LBServer:public Server
{
public:
	LBServer(int serverId, ConfigType *config);
	~LBServer();

	// 读取配置
	// 初始化扩散服务器mSpreadserverList
	bool Init(ConfigType *);
	void InitResource();
	void InitSpreadServer();
	// 通讯协议接口 epoll
	// 收到连接进来的客户端的消息时->消息处理
	// 消息1、客户端请求 选择服务器(->增加服务器负载->检查是否需要扩散->开始扩散)--回复
	// 消息2、客户端退出 减少服务器负载--回复
	void Run();

	int AddNewClient(int epfd, int listenfd);
	int ReadMessage(int connfd);
	int WriteMessage(int connfd, void *message);
	void MessageProcess(int connfd, void *buf);

	void HasFoundBestServer(int fileId, int serverId);
	// 
	int GetCurrentLoad(int serverId);
	int IncreaseLoad(int fileId, int serverId);
    int DecreaseLoad(int fileId, int serverId);

    int IncreaseCopyFlow(int fileId, int serverId);
    int DecreaseCopyFlow(int fileId, int serverId);
    int GetCurrentCopyFlow(int serverId);

    int IncreaseInFlow(int fileId, int serverId);
    int DecreaseInFlow(int fileId, int serverId);

    void NotifySpread(int serverId, int load);
    // 增加某个serverId的负载时检查该serverId是否需要扩散
	bool IsNeedSpread(int serverId);
	bool IsCanBeTarget(int serverId);
	bool IsOverCapacity(int serverId);
	void StartSpread(int serverId);
	void BeforeSpread(int fileId, int srcServerId, int targetServerId);
	void CreateSpread(int fileId, int srcServerId, int targetServerId);
	void FinishSpread(int fileId, int srcServerId, int targetServerId, double startTime, double endTime);

	int GetServerList(int clientId, int fileId, int &fileLen, vector<int> &serverList);
	int GetSpreadFile(int serverId, vector<int> &fileList);
	int GetNoServerList(int fileId, vector<int> &noServerList);
    // 获取最小负载的扩散服务器id
	int SortServerByLoad(vector<int> &serverList);

	// 文件 fileId 放在 serverId 上
    bool AddFile(int fileId, int serverId, bool firsttime);
    bool DeleteFile(int fileId, int serverId);
    int GetDeleteFileList(int serverId, vector<int> &fileList);
    int EraseNoCopyFile(int serverId, vector<int> &fileList);

	bool SearchFile(int serverId, int fileId);
	int GetEventFd();
	void FinishSpreadSendMsg(void *);

	void PrintTimes();
	void PrintFileList();
	void PrintSpreadingFileList();
	void Exit();
	
private:
	void StrategyReset();

	/* data */
private:
	ConfigType *mConfig;						// 服务器配置
	vector<SpreadServer *> mSpreadServerList;	// 扩散服务器数组
	vector<Spread *> mSpreadList;				// 所有的扩散流
	vector<FileInfo *> mFileInfoList;			// 负载均衡服务器上保存的文件与服务器的对应列表
												// 主要用于寻找确定含有文件fileid的服务器列表
												// 所以，在一次扩散完成时更新
												// 子服务器上的文件列表在扩散前更新
												// 防止再次向该服务器扩散该文件

	int mSpreadTimes;							// 扩散计数器
	int mRefuseTimes;							// 拒绝计数器
	int mRequestTimes;							// 请求计数器
	int mDeleteTimes;							// 删除计数器

	int m_eventfd[2];							// 资源扩散完成后向此套接字发信息
	int m_strategy_resetfd[2];					// 资源扩散周期策略重置
	int m_exitfd[2];							// 负载均衡服务器退出
	pthread_mutex_t mMutex;						// 资源扩散线程调用完成扩散接口时需要加锁
};

#endif

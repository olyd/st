#ifndef SPREADSERVER_H_
#define SPREADSERVER_H_

#include <vector>

#include "server.h"

using namespace std;

class SpreadStrategy;

const int NORMAL = 0;		// 没有扩散的文件
const int SPREADING = 1;	// 扩散中
const int SPREADED = 2;		// 扩散完

typedef struct FileNode {
	FileNode(){
		// hitnew = 1; // comment by mjq@2014.03.27
		hitnew = 0;
		hitold = 0;
		// vtime = getRelativeTime(); // comment by mjq@2014.03.27
		// 我觉得一个文件扩散到目标服务器上后，它的最近访问时间清零
		// 同刚开始放置到其上的文件的访问时间一样
		vtime = 0.0;
		status = NORMAL;
		load = 0;
	};
	unsigned int mServerId;		// 服务器id
	int fileid;					// 文件id
	// bool hadSpread;			// 文件是否
	double vtime;				// 访问时间
	int hitold;					// 前一周期的访问计数
	int hitnew;					// 当前周期的访问计数
	int weight;					// 权重
	int status;					// 文件状态，NORMAL SPREADING SPREADED
	int load;					// 负载
} FileNode;

class SpreadServer : public Server
{
public:
	SpreadServer(int serverid) : Server(serverid){ };
	SpreadServer(int serverid, ConfigType *config);
	~SpreadServer();
	bool Init(ConfigType *);					// 读取配置，初始化扩散服务器
	void Run();									// 开始运行

	int IncreaseLoad(int fileid);				// 增加负载		
	int DecreaseLoad(int fileid);				// 减少负载
	int GetCurrentLoad();						// 获取当前负载

	int IncreaseCopyFlow(int fileid);			// 增加复制出流
	int DecreaseCopyFlow(int fileid);			// 减少复制出流
	int GetCopyFlow();							// 得到复制出流
	int IncreaseInFlow(int fileid);				//　增加复制入流
	int DecreaseInFlow(int fileid);				// 减少复制入流

	bool IsNeedSpread();						// 是否需要扩散
	bool IsCanBeTarget();						// 是否可以作为扩散的目的服务器
	bool IsOverCapacity();						// 是否超容量
	bool IsOverLoad();							// 是否负载达到最大值

	void PrintFileListInServer();				// 打印服务器上的文件列表
	void PrintSpreadingFileListInServer();		// 打印服务器上的文件列表
	bool AddFile(int fileid, bool firsttime);	// 添加文件
	void DeleteFile(int fileid);				// 删除文件
	bool SearchFile(int fileid);				// 搜索文件，是否含有该文件
	int GetSpreadFile(vector<int> &output);		// 获取需要扩散的文件列表
	int GetDeleteFileList(vector<int> &output);	// 返回没有客户端访问 且 不是正在扩散 的文件列表
												// 可删除的文件列表按照访问时间从小到大排序
	void Reset();								// 对服务器的文件列表进行重置
												// 重置文件统计信息，一些周期算法需要

private:
	int mMaxLoad;						// 最大负载
	int mCurrentLoad;					// 当前负载
	int mLastPeriodLoad;				// 上一个周期的负载
	int mCurrentPeriodLoad;				// 当前周期的负载

	int mMaxCapacity;					// 最大容量
	int mCurrentCapacity;				// 当前容量

	double mThreshold;					// 允许扩散的最小阈值
	double m_load_thresh_high;			// 允许扩散的最大阈值
	int mFileLength;					// 文件长度
	int mMaxDiskBand;					// 服务器的磁盘带宽

	int m_max_copy_flow;				// 最大复制出流
	int m_copy_flow;					// 当前复制出流
	int m_max_in_flow;					// 最大复制入流
	int m_in_flow;						// 当前复制入流
	vector<FileNode> m_filelist;		// 该子服务器上的文件列表，
										// 如果要向本服务器扩散某个文件，必须在扩散开始前更新列表
										// 防止再次向该服务器上扩散某文件
	SpreadStrategy *m_spread_strategy;	// 扩散策略
	string mStrategyName;				// 扩散策略名称
	int mPeriod;						// 扩散策略周期
	double mPeriodStartTime;			// 新的周期开始时间,reset重置时更新，相对系统运行时间
	double mTimeInPeriod;				// 周期内的时间
};



#endif

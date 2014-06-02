#ifndef STRIPCLIENT_H
#define STRIPCLIENT_H

#include "client.h"
#include "config.h"
#include "timer.h"
#include "message.h"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <pthread.h>

#include <iostream>
#include <fstream>

class DBuffer;

class StripClient : public Client{
public:
	StripClient(int id, ConfigStrip *config);
	~StripClient();
	virtual bool Init();
	virtual void Run();
	virtual void Exit();
	// 获取客户端线程id
	pthread_t GetTid(){return mtid;}
	void RegisterStartEvent();
	void StartClient(epoll_event event);
	void DealWithMessage(char *buffer);
	void BufferReset();
	void PlayInFackTran();
private:

	int mEpollFd;			// 客户端epoll套接字
	int mNetSockFd;			// 该客户端作为服务器时(P2P下)的套接字
	int mPlaySockFd[2];		// 
	int mBufferResetFd[2];	// 缓冲区算法周期重置socketpair
	int mFackTranFd[2];		// 
	int mMyPort;			// 客户端端口号
	pthread_t mtid;			// 客户端线程id
	int mLinkedNums;		// p2p开启下连接到该客户端的连接数
							// 不得超过 message.h 中的 MAX_CLIENT_LINKS 4
	int mHitTimes;			// 
	int mTotalTimes;		// 

	DBuffer *mDbuffer;		// buffer
	ConfigStrip *mConfig;	// 配置文件
	
	int mBlockSize;			// 段大小KB   10240KB

	int mFileId;			// 当前ask文件id
	int mSegId;				// 当前ask段id
	int mMaxSegId;			// 最大段号
	int mBitRate;			// 文件码率
	int mOldFileId;
	int mOldSegId;
	int mPreFetchSegId;		// 预取段id
	bool mIsReadFin;		// 预取是否下载完毕
	bool mIsPlaying;		// 是否正在播放
	int mReadSegId;			// 下一播放段id
	bool mIsPreFetchFail;	// 是否预取失败

	struct timeval mDelayBeginTime;	// 
	struct timeval mDelayEndTime;	//
	double mDelayTime;				// 播放延迟时间
	bool mDelay;					// 

	int mCurrentServerId;	// 当前为其服务的客户端id 或者 服务器0
							// 由于p2p的存在，所以可能是其他客户端
	bool mIsFirstStart;		// 第一次收到MSG_SEG_ACK消息时


	fstream mIOfs;			// requestFile*.log
	ofstream mHitFs;		// 客户端缓冲区命中日志
	ofstream mRecordFs;		// 客户端运行日志
	ofstream mDelayFs;		// 客户端播放延迟日志
	ClientInfoBlock mClientInfo[MAX_CLIENT_NUM + 1];	// 保存所有客户端+服务器的套接字存储数组
};



#endif
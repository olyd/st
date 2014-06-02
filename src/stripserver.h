#ifndef STRIPSERVER_H
#define STRIPSERVER_H

#include <netinet/in.h>
#include <fstream>
#include "server.h"
#include "config.h"
#include "message.h"
#include <pthread.h>

class StripServer;
class DBufferThreadSafe;
class DataServerThreadSafe;
class DiskMgr;

struct RequestArgs{
	StripServer *server;     /**< 指向Server */
	struct sockaddr_in cliaddr; /**< 客户端的地址结构*/
	int connfd;                 /**< 客户端的socket套接字*/
};

class StripServer : public Server
{
public:
	StripServer(int serverid) : Server(serverid){ };
	StripServer(int serverid, ConfigStrip *config);
	~StripServer();
	void ThreadPerClient(struct RequestArgs *ptr);
	void ThreadEvent();
	void Run();
private:
	void BufferReset(); 
	void TakeSample(); 

	ConfigStrip *m_config;	// 配置文件
	bool m_p2p;				// 是否使用P2P
	bool m_real_device;
	int m_port;				// 服务器与客户端通信的端口
	int m_clientport;
	int m_linkcount;		// 当前已连接的客户端数
	int m_period;			// 服务器缓冲区重置周期
	int m_lrfulambda;		// 服务器lrfu算法所需
	int m_block_size;
	int m_block_num;
	int m_filenum;
	int m_min_length;
	int m_max_length;
	double m_min_bitrate;
	double m_max_bitrate;
	int m_diskNum;			// 服务器磁盘数目
	string m_place_strategy;// 分块放置策略，随机 轮转等
	int m_disk_band;		// 对于不同分片的磁盘读取速度MB/s

	int m_event_fd;
	int m_buffer_reset_fd[2];
	DBufferThreadSafe *m_buffer;
	DataServerThreadSafe *m_data_server;
	ClientInfoBlock m_clientinfo[MAX_CLIENT_NUM + 1];

	int m_total_request;	// 客户端总请求数
	int m_read_from_server;	// 没有找到P2P，直接从服务器读取的请求个数
	int m_buffer_hit;
	int m_buffer_miss;
// string m_buffer_strategy;

	ofstream m_buffer_ofs;
	ofstream m_access_balance_degree_ofs;	// 磁盘负载均衡度写入文件 balance_degree.log

	int m_facktran_fd[2];
	int m_take_sample_fd[2];
	int m_take_sample_frequency;	// 取样频率，单位为second
	int m_current_load;				// 
	int m_real_load;				// 某一瞬间向服务器请求的客户端的数目
	int m_server_band;
	int m_client_band;
	pthread_mutex_t m_facktran_mutex;
	pthread_cond_t m_facktran_cond;

	DiskMgr *m_diskMgr;
};

#endif
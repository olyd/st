/*
 * DataServer.h
 *
 *  Created on: 2013-2-20
 *      Author: zhaojun
 */

#ifndef DATASERVER_H_
#define DATASERVER_H_

#include <list>
#include <string>
#include "message.h"

using namespace std;

struct FileInfoBlock{
	int segId;
//	double linkedNum;
	int clientNum;
};

struct DataBlock{
	int fileId;
	int segNum;
	double bitRate;
	list<FileInfoBlock> info;
};

class BlockPosition{
public:
	BlockPosition(int _fileId, int _segId, int _diskId):fileId(_fileId), segId(_segId), diskId(_diskId){};
	int fileId;
	int segId;
	int diskId;
};

class DataServer{
public:
	DataServer(int fileNum,int minLength,int maxLength,int blockSize,double minBitRate,double maxBitRate, int diskNum);
	~DataServer();
	// P2P相关
	int SearchBestClient(int fileId,int segId);
	void GetFileInfo(int fileId,double *bitRate,int *segNum);
	void InsertIntoIndex(int fileId,int segId,int clientNum,int linkedNum);
	void DeleteFromIndex(int fileId,int segId,int clientNum);

	// 统计相关

	void IncreaseRealLoad(){ 			++m_real_load; }
	void DecreaseRealLoad(){ 			--m_real_load; }
	void IncreaseTotalRequest(){		++m_total_request;	}
	void IncreaseReadFromServer(){		++m_read_from_server;	}
	void IncreaseBufferHit(){			++m_buffer_hit;	}
	void IncreaseBufferMiss(){			++m_buffer_miss;	}

	int GetRealLoad(){					return m_real_load;  }
	int GetTotalRequest(){				return m_total_request;	}
	int GetReadFromServer(){			return m_read_from_server;	}
	int GetBufferHit(){					return m_buffer_hit;	}
	int GetBufferMiss(){				return m_buffer_miss;	}

	// 磁盘负载相关
	int GetDiskId(int fileId, int segId, string placeStrategy);// 根据段id和放置策略获取磁盘id
	void IncreaseDiskAccessCount(int diskId);
	int GetDiskAccessCount(int diskId);
	double GetAccessBalanceDegree(list<int> &accessCountList);

private:
	DataBlock mFileInfo[MAX_FILE_NUM + 1];
	int mClientLinks[MAX_CLIENT_NUM + 1];
	int mFileNum;

	int m_real_load;				// 服务器端的负载数，来就增加，不能服务的等待
	int m_total_request;			// 总请求数
	int m_read_from_server;			// 服务器端总请求数目（p2p开启时小于m_total_request）
	int m_buffer_hit;				// 服务器端缓冲区命中数目
	int m_buffer_miss;				// 服务器端从磁盘读取数目
									// m_buffer_hit + m_buffer_miss = m_read_from_server

	int mDiskLoad[MAX_DISK_NUM+1];	// 统计每个磁盘的负载数目,比如6个磁盘，
									// 则0~5为磁盘负载，6为磁盘不均衡度
	int mDiskNum;					// 每个服务器上的磁盘个数(暂时只支持同构服务器)
	list<BlockPosition> positionList;
};


#endif /* DATASERVER_H_ */

#ifndef DATASERVERTHREADSAFE_H
#define DATASERVERTHREADSAFE_H
#include <string>
#include <list>
#include <pthread.h>
using namespace std;
class DataServer;

class DataServerThreadSafe {
public:
	DataServerThreadSafe(int fileNum,int minLength,int maxLength,int blockSize,double minBitRate,double maxBitRate, int diskNum);
	~DataServerThreadSafe();
	// P2P相关
	int SearchBestClient(int fileId,int segId);
	void GetFileInfo(int fileId,double *bitRate,int *segNum);
	void InsertIntoIndex(int fileId,int segId,int clientNum,int linkedNum);
	void DeleteFromIndex(int fileId,int segId,int clientNum);

	// 统计相关
	void IncreaseRealLoad();
	void DecreaseRealLoad();
	void IncreaseTotalRequest();
	void IncreaseReadFromServer();
	void IncreaseBufferHit();
	void IncreaseBufferMiss();
	int GetRealLoad();
	int GetTotalRequest();
	int GetReadFromServer();
	int GetBufferHit();
	int GetBufferMiss();

	// 磁盘负载相关
	int GetDiskId(int fileId, int segId, string placeStrategy);
	void IncreaseDiskAccessCount(int diskId);
	int GetDiskAccessCount(int diskId);
	double GetAccessBalanceDegree(list<int> &accessCountList);
private:
	DataServer *m_data_server;
	pthread_mutex_t m_mutex;
};


#endif /* DATASERVER_H_ */

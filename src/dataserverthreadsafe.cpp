#include "dataserverthreadsafe.h"
#include "dataserver.h"
#include "utils.h"

DataServerThreadSafe::DataServerThreadSafe(int fileNum,int minLength,int maxLength,int blockSize,
		double minBitRate,double maxBitRate, int diskNum)
{
	m_data_server = new DataServer(fileNum, minLength, maxLength, blockSize, minBitRate, maxBitRate, diskNum);
	Pthread_mutex_init(&m_mutex, NULL);
}

DataServerThreadSafe::~DataServerThreadSafe()
{
	Pthread_mutex_destroy(&m_mutex);
	delete m_data_server;
}

int DataServerThreadSafe::SearchBestClient(int fileId,int segId)
{
	int ret;
	Pthread_mutex_lock(&m_mutex);
	ret = m_data_server->SearchBestClient(fileId, segId);
	Pthread_mutex_unlock(&m_mutex);
	return ret;
}

void DataServerThreadSafe::GetFileInfo(int fileId,double *bitRate,int *segNum)
{
	Pthread_mutex_lock(&m_mutex);
	m_data_server->GetFileInfo(fileId, bitRate, segNum);
	Pthread_mutex_unlock(&m_mutex);
}

void DataServerThreadSafe::InsertIntoIndex(int fileId,int segId,int clientNum,int linkedNum)
{
	Pthread_mutex_lock(&m_mutex);
	m_data_server->InsertIntoIndex(fileId, segId, clientNum, linkedNum);
	Pthread_mutex_unlock(&m_mutex);
}

void DataServerThreadSafe::DeleteFromIndex(int fileId,int segId,int clientNum)
{
	Pthread_mutex_lock(&m_mutex);
	m_data_server->DeleteFromIndex(fileId, segId, clientNum);
	Pthread_mutex_unlock(&m_mutex);
}


// 磁盘负载相关
int DataServerThreadSafe::GetDiskId(int fileId, int segId, string placeStrategy)
{
	int ret;
	Pthread_mutex_lock(&m_mutex);
	ret = m_data_server->GetDiskId(fileId, segId, placeStrategy);
	Pthread_mutex_unlock(&m_mutex);
	return ret;
}
void DataServerThreadSafe::IncreaseDiskAccessCount(int diskId)
{
	Pthread_mutex_lock(&m_mutex);
	m_data_server->IncreaseDiskAccessCount(diskId);
	Pthread_mutex_unlock(&m_mutex);
}
int DataServerThreadSafe::GetDiskAccessCount(int diskId)
{
	int ret;
	Pthread_mutex_lock(&m_mutex);
	ret = m_data_server->GetDiskAccessCount(diskId);
	Pthread_mutex_unlock(&m_mutex);
	return ret;
}
double DataServerThreadSafe::GetAccessBalanceDegree(list<int> &accessCountList)
{
	double ret;
	Pthread_mutex_lock(&m_mutex);
	ret = m_data_server->GetAccessBalanceDegree(accessCountList);
	Pthread_mutex_unlock(&m_mutex);
	return ret;
}

// 统计相关
void DataServerThreadSafe::IncreaseRealLoad(){
	Pthread_mutex_lock(&m_mutex);
	m_data_server->IncreaseRealLoad();
	Pthread_mutex_unlock(&m_mutex);
}
void DataServerThreadSafe::DecreaseRealLoad(){
	Pthread_mutex_lock(&m_mutex);
	m_data_server->DecreaseRealLoad();
	Pthread_mutex_unlock(&m_mutex);
}
void DataServerThreadSafe::IncreaseTotalRequest(){
	Pthread_mutex_lock(&m_mutex);
	m_data_server->IncreaseTotalRequest();
	Pthread_mutex_unlock(&m_mutex);
}
void DataServerThreadSafe::IncreaseReadFromServer(){
	Pthread_mutex_lock(&m_mutex);
	m_data_server->IncreaseReadFromServer();
	Pthread_mutex_unlock(&m_mutex);
}
void DataServerThreadSafe::IncreaseBufferHit(){
	Pthread_mutex_lock(&m_mutex);
	m_data_server->IncreaseBufferHit();
	Pthread_mutex_unlock(&m_mutex);
}
void DataServerThreadSafe::IncreaseBufferMiss(){
	Pthread_mutex_lock(&m_mutex);
	m_data_server->IncreaseBufferMiss();
	Pthread_mutex_unlock(&m_mutex);
}
int DataServerThreadSafe::GetRealLoad(){
	int ret;
	Pthread_mutex_lock(&m_mutex);
	ret = m_data_server->GetRealLoad();
	Pthread_mutex_unlock(&m_mutex);
	return ret;
}
int DataServerThreadSafe::GetTotalRequest(){
	int ret;
	Pthread_mutex_lock(&m_mutex);
	ret = m_data_server->GetTotalRequest();
	Pthread_mutex_unlock(&m_mutex);
	return ret;
}
int DataServerThreadSafe::GetReadFromServer(){
	int ret;
	Pthread_mutex_lock(&m_mutex);
	ret = m_data_server->GetReadFromServer();
	Pthread_mutex_unlock(&m_mutex);
	return ret;
}
int DataServerThreadSafe::GetBufferHit(){
	int ret;
	Pthread_mutex_lock(&m_mutex);
	ret = m_data_server->GetBufferHit();
	Pthread_mutex_unlock(&m_mutex);
	return ret;
}
int DataServerThreadSafe::GetBufferMiss(){
	int ret;
	Pthread_mutex_lock(&m_mutex);
	ret = m_data_server->GetBufferMiss();
	Pthread_mutex_unlock(&m_mutex);
	return ret;
}

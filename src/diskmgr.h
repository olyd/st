#ifndef DISKMGR_H
#define DISKMGR_H

#include "config.h"

#include <string>
#include <vector>

#include <pthread.h>

using namespace std;

struct FileInfoDisk {
	int diskid;
	// int fileid;
	// int segid;
	string filename;
};

struct DiskInfo {
	DiskInfo(int _diskid);
	~DiskInfo();
	int diskid;
	int waitingThread;
	int readingThread;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
};

class DiskMgr {
public:
	DiskMgr(ConfigStrip *config);
	~DiskMgr();
	void ReadSeg(int fileid, int segid);
	void ReadSeg(int fileid, int segid, int diskid);
    void PrintDiskInfo();
	int GetTotalRead()
	{
		int ret;
		pthread_mutex_lock(&m_mutex);
		ret = m_totalRead;
		pthread_mutex_unlock(&m_mutex);
		return ret;
	}
	vector<DiskInfo> &GetDiskInfo()
	{
		return m_diskInfo;
	}
private:
	void GetFileNamesInDir(const string strDir, vector<string> &vecFileName);
	off_t GetFileLength(const string filename);

	int m_blockSize;
	int m_fileNumber;
	int m_diskNumber;
	int m_diskBand;							// 不同分片下的磁盘读取速度
	int m_fileLength;
	int m_totalRead;
	int m_curReadThreadNum;
	pthread_mutex_t m_mutex;
	// vector<vector<string> > m_fileIndex;
	vector<vector<FileInfoDisk> > m_fileIndex;
	vector<DiskInfo> m_diskInfo;
};

#endif

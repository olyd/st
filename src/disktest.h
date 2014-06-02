#ifndef DISKTEST_H
#define DISKTEST_H

#include "config.h"

class DiskMgr;

const int MSG_NEWCLIENT = 0;
const int MSG_STATISTICS = 1;

class DiskTest {
public:
	DiskTest(ConfigStrip *config);
	~DiskTest();
	void Run();
	void ThreadClient(int clientid);
	void ThreadServer(int clientid);
private:
	int m_clientNum;
	int m_listen[2];
	int *m_buffer;
	int *m_delay;
	int m_preFetch;
	int m_fileLength;
	int m_lockMemory;
	int m_diskNum;
	DiskMgr *m_diskMgr;
	pthread_mutex_t *m_mutexs;
	pthread_cond_t *m_conds;
};

#endif
#include "disktest.h"

#include <cassert>

#include "timer.h"
#include "message.h"
#include "utils.h"
#include "diskmgr.h"
#include "log.h"

struct RequestArgsDiskTest{
	DiskTest *diskTest;
	int clientid;
};

static void *ThreadClient_(void *arg)
{
	Pthread_detach(pthread_self());
	struct RequestArgsDiskTest *ptr = (struct RequestArgsDiskTest *)arg;
	int clientid = ptr->clientid;
	ptr->diskTest->ThreadClient(clientid);
	return (void *)NULL;
}

static void *ThreadServer_(void *arg)
{
	Pthread_detach(pthread_self());
	struct RequestArgsDiskTest *ptr = (struct RequestArgsDiskTest *)arg;
	int clientid = ptr->clientid;
	ptr->diskTest->ThreadServer(clientid);
	return (void *)NULL;
}

DiskTest::DiskTest(ConfigStrip *config)
{
	m_clientNum = config->clientNums;
	Socketpair(AF_UNIX, SOCK_STREAM, 0, m_listen);
	m_buffer = new int[m_clientNum];
	m_delay = new int[m_clientNum];
	m_fileLength = config->maxLength;
	m_preFetch = config->preFetch;
	m_lockMemory = config->lockMemory;
	m_diskMgr = new DiskMgr(config);
	m_diskNum = config->diskNumber;
	m_mutexs = (pthread_mutex_t *)Malloc(sizeof(pthread_mutex_t) * m_clientNum);
	m_conds = (pthread_cond_t *)Malloc(sizeof(pthread_cond_t) * m_clientNum);
	for (int i = 0; i < m_clientNum; ++i) {
		m_buffer[i] = 0;
		m_delay[i] = 0;
		Pthread_mutex_init(&m_mutexs[i], NULL);
		Pthread_cond_init(&m_conds[i], NULL);
	}
}

DiskTest::~DiskTest()
{
	delete[] m_buffer;
	delete[] m_delay;
	for (int i = 0; i < m_clientNum; ++i) {
		Pthread_mutex_destroy(&m_mutexs[i]);
		pthread_cond_destroy(&m_conds[i]);
	}
	free(m_mutexs);
	free(m_conds);
}

void DiskTest::Run()
{
	char buffer[MESSAGELEN];
	// char response[MESSAGELEN];
	int *ptr;
	// int type;
	pthread_t tid;
	TimerEvent timerEvent;
	LockMemory(m_lockMemory);
	for (int i = 0; i < m_clientNum; ++i) {
		timerEvent.sockfd = m_listen[0];
		timerEvent.left_time = (i + 1) * TIMER_SCALE;
		ptr = (int *)timerEvent.buffer;
		ptr[0] = MSG_NEWCLIENT;
		Timer::GetTimer()->RegisterTimer(timerEvent);
	}
	timerEvent.sockfd = m_listen[0];
	timerEvent.left_time = 10 * TIMER_SCALE;
	ptr = (int *)timerEvent.buffer;
	ptr[0] = MSG_STATISTICS;
	Timer::GetTimer()->RegisterTimer(timerEvent);

	int id = -1;
	int lastread = 0;
	int newread = 0;
	vector<DiskInfo> diskInfo;
	while (true) {
		read(m_listen[1], buffer, MESSAGELEN);
		ptr = (int *)buffer;
		switch (ptr[0]) {
		case MSG_NEWCLIENT:
			++id;
			struct RequestArgsDiskTest arg;
			arg.diskTest = this;
			arg.clientid = id;
			Pthread_create(&tid, NULL, ThreadClient_, &arg);
			break;
		case MSG_STATISTICS:
			newread = m_diskMgr->GetTotalRead();
			diskInfo = m_diskMgr->GetDiskInfo();
			LOG_INFO("speed: " << ((newread - lastread) * 1.0) << "MB/s");
			for (int i = 0; i < m_diskNum; ++i) {
				cout << "reading: ";
				cout << diskInfo[i].readingThread << " ";
			}
			cout << endl;
			for (int i = 0; i < m_diskNum; ++i) {
				cout << "waiting: ";
				cout << diskInfo[i].waitingThread << " ";
			}
			cout << endl;
			lastread = newread;
			timerEvent.sockfd = m_listen[0];
			timerEvent.left_time = 10 * TIMER_SCALE;
			ptr = (int *)timerEvent.buffer;
			ptr[0] = MSG_STATISTICS;
			Timer::GetTimer()->RegisterTimer(timerEvent);
			break;
		default:
			assert(0);
		}

	}
}

void DiskTest::ThreadClient(int clientid)
{
	LOG_INFO("Client " << clientid << " has started.");
	pthread_t tid;
	int count = 0;
	struct RequestArgsDiskTest arg;
	TimerEvent timerEvent;
	char buffer[MESSAGELEN];
	arg.diskTest = this;
	arg.clientid = clientid;
	Pthread_create(&tid, NULL, ThreadServer_, &arg);
	int fd[2];
	Socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
	pthread_mutex_lock(&m_mutexs[clientid]);
	if (m_buffer[clientid] == 0) { //没有缓冲区
		pthread_cond_wait(&m_conds[clientid], &m_mutexs[clientid]);
	}
	--m_buffer[clientid];
	pthread_cond_signal(&m_conds[clientid]);
	pthread_mutex_unlock(&m_mutexs[clientid]);
	timerEvent.sockfd = fd[0];
	timerEvent.left_time = 4 * TIMER_SCALE;
	Timer::GetTimer()->RegisterTimer(timerEvent);
	while (true) {
		read(fd[1], buffer, MESSAGELEN);
		pthread_mutex_lock(&m_mutexs[clientid]);
		if (m_buffer[clientid] == 0) { //没有缓冲区
			++m_delay[clientid];
			LOG_INFO("Client " << clientid << " has delayed " << m_delay[clientid] << "times.");
			pthread_cond_wait(&m_conds[clientid], &m_mutexs[clientid]);
		}
		assert(m_buffer[clientid] > 0);
		--m_buffer[clientid];
		pthread_cond_signal(&m_conds[clientid]);
		pthread_mutex_unlock(&m_mutexs[clientid]);
		++count;
		//LOG_INFO("Client " << clientid << " played " << count << " segments.");
		timerEvent.sockfd = fd[0];
		timerEvent.left_time = 4 * TIMER_SCALE;
		Timer::GetTimer()->RegisterTimer(timerEvent);
	}
}

void DiskTest::ThreadServer(int clientid)
{
	int index = 0;
	while (true) {
		pthread_mutex_lock(&m_mutexs[clientid]);
		if (m_buffer[clientid] >= m_preFetch) {
			pthread_cond_wait(&m_conds[clientid], &m_mutexs[clientid]);
		}
		pthread_mutex_unlock(&m_mutexs[clientid]);
		m_diskMgr->ReadSeg(clientid, index);
	    //usleep(100000);
        pthread_mutex_lock(&m_mutexs[clientid]);
		++m_buffer[clientid];
		pthread_cond_signal(&m_conds[clientid]);
		pthread_mutex_unlock(&m_mutexs[clientid]);
		++index;
		if (index >= m_fileLength) index = 0;
	}
}

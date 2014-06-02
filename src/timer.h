#ifndef TIMER_H_
#define TIMER_H_

#include <pthread.h>

#include <list>

#include "message.h"

using namespace std;

const long long TIMER_SCALE = 1000000;

struct TimerEvent{
	long long left_time;
	int sockfd;
	bool isnew;
	// int load;
	char buffer[MESSAGELEN];
};

class Timer {
public:
	static Timer * GetTimer();
	~Timer();
	void RegisterTimer(TimerEvent &event);
	void TimerThread();
	void ModifyTimers(int load);
private:
	Timer();
	void WakeTimer(TimerEvent &event);
	static Timer *m_timer;
	list<TimerEvent> m_timerlist;
	pthread_mutex_t m_timer_mutex;
	pthread_cond_t m_timer_cond;
};

#endif
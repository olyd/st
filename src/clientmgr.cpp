#include <sys/epoll.h>
#include <sys/socket.h>

#include <sstream>
#include <fstream>
#include <cassert>

#include "clientmgr.h"
#include "message.h"
#include "poisson.h"
#include "zipf.h"
#include "spreadclient.h"
#include "timer.h"
#include "log.h"

#include "stripclient.h"

ClientManager::ClientManager(bool isStrip, void *config):mIsStrip(isStrip)
{
	// 启动分条客户端
	if(isStrip){
		mConfigStrip = (ConfigStrip *)config;
		cout << "clientmanager init strip client..." << endl;
		// cout << "serverBand:"<< mConfigStrip->serverBand << endl;
		// cout << "isSpecial:"<< mConfigStrip->isSpecial << endl;
	}
	// 启动扩散客户端
	else{
		m_config = (ConfigType *)config;
		m_clientnum = m_config->clientNumber;
		m_poisson = new Poisson(m_config->poissonLambda / 1000.0);
		m_zipf = new Zipf(m_config->clientNumber, 
		m_config->resourceNumber, m_config->zipfParameter / 1000.0);
		m_clientnum = m_config->clientNumber;
		m_request_file = m_config->requestListFile;
		m_poisson_lambda = m_config->poissonLambda;
		m_zipf_sita = m_config->zipfParameter;
		m_epollfd = epoll_create(MAX_LISTEN_NUM);
		epoll_event ev;
		Socketpair(AF_UNIX, SOCK_STREAM, 0, m_eventfd);
		ev.data.fd = m_eventfd[0];
		ev.events = EPOLLIN;
		epoll_ctl(m_epollfd, EPOLL_CTL_ADD, m_eventfd[0], &ev);

		Socketpair(AF_UNIX, SOCK_STREAM, 0, m_exitfd);
		ev.data.fd = m_exitfd[0];
		ev.events = EPOLLIN;
		epoll_ctl(m_epollfd, EPOLL_CTL_ADD, m_exitfd[0], &ev);
	}
}

ClientManager::~ClientManager()
{
	delete m_poisson;
	delete m_zipf;
}

void ClientManager::CreateStripClient()
{
	cout << "clientmanager create strip client..." << endl;
	assert(mConfigStrip != NULL);
	// fstream iofs;
	// if(mConfigStrip->isRepeat){
	// 	stringstream sstream;
	// 	sstream.str("");
	// 	sstream << "data/requestFile" << i << ".log";
	// 	string requestFilename = sstream.str();
	// 	iofs.open(requestFilename.c_str(), ios::in);
	// }

	m_clientlist.clear();
	m_clientlist.push_back(NULL);
	int startClient = (mConfigStrip->clientNums / mConfigStrip->devNums) * mConfigStrip->clusterNums + 1;
	int endClient = startClient + (mConfigStrip->clientNums / mConfigStrip->devNums);
	for(int i = startClient;i < endClient;i++){
		StripClient *client = new StripClient(i, mConfigStrip);
		m_clientlist.push_back(client);
		// LOG_INFO("create thread " << i << " " << ((StripClient *)m_clientlist[i - startClient + 1])->GetTid());
	}
}

void ClientManager::CreateSpreadClient()
{
	vector<int> filelist;
	vector<int> interval;
	int index = m_request_file.find('.');
	ostringstream oss;
	oss << m_request_file.substr(0, index) << "_"
		<< m_zipf_sita << "_"
		<< m_poisson_lambda << "_"
		<< m_clientnum << m_request_file.substr(index);
	string request_file = oss.str();
	ifstream ifs(request_file.c_str());
	if (!ifs) {
		cout << request_file << " does not exist, create one" << endl;
		m_zipf->CreateZipfList(filelist);
		m_poisson->GetPoissonList(m_clientnum, interval);
		ofstream ofs(request_file.c_str());
		for(int i = 0; i < m_clientnum; ++i) {
			ofs << filelist.at(i) << " " << interval.at(i) << endl;
		}
		ofs.close();
	} else {
		int a, b;
		while (!ifs.eof()) {
			ifs >> a >> b;
			filelist.push_back(a);
			interval.push_back(b);
		}
		ifs.close();
	}

	for (int i = 0; i < m_clientnum; ++i) {
		Client *client = new SpreadClient(i,m_config);
		// client->Init(m_config);
		client->SetFileID(filelist[i]);
		m_clientlist.push_back(client);
	}
	m_clientlist[0]->Run();
	TimerEvent timer_event;
	assert(m_clientnum > 1);
	timer_event.left_time = interval[1];
	timer_event.sockfd = m_eventfd[1];
	Timer::GetTimer()->RegisterTimer(timer_event);

	timer_event.left_time = m_config->runTime * TIMER_SCALE;
	timer_event.sockfd = m_exitfd[1];
	Timer::GetTimer()->RegisterTimer(timer_event);

	char buffer[MESSAGELEN];
	int cur_clientid = 1;
	int nfds;
	int length;
	epoll_event events[MAX_LISTEN_NUM];
	while (true) {
		nfds = epoll_wait(m_epollfd, events, MAX_LISTEN_NUM, -1);
		for (int i = 0; i < nfds; ++i) {
			if (events[i].data.fd == m_eventfd[0]) {
				length = recv(events[i].data.fd, buffer, MESSAGELEN ,0);
				m_clientlist[cur_clientid]->Run();
				++cur_clientid;
				if (cur_clientid < m_clientnum) {
					timer_event.left_time = interval[cur_clientid];
					// timer_event.left_time = 100;
					timer_event.sockfd = m_eventfd[1];
					Timer::GetTimer()->RegisterTimer(timer_event);
				}
			} else if (events[i].data.fd == m_exitfd[0]) {
				recv(events[i].data.fd, buffer, MESSAGELEN ,0);
				LOG_INFO("Exit");
				goto out;
			}
			assert(length == MESSAGELEN);
		}
	}
out:
	return;
}

void ClientManager::Run()
{
	if(mIsStrip){
		CreateStripClient();
		sleep(1);
		pthread_join(((StripClient *)m_clientlist[1])->GetTid(), NULL);
	}else{
		CreateSpreadClient();
	}
}

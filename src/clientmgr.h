#ifndef CLIENTMGR_H
#define CLIENTMGR_H

#include <vector>

#include "config.h"

using namespace std;

class Poisson;
class Zipf;
class Client;

class ClientManager {
public:
	ClientManager(bool isStrip, void *config);
	~ClientManager();
	void CreateStripClient();
	void CreateSpreadClient();
	void Run();
private:
	int m_clientnum;
	int m_epollfd;
	int m_poisson_lambda;
	int m_zipf_sita;
	Poisson *m_poisson;
	Zipf *m_zipf;
	vector<Client *> m_clientlist;
	string m_request_file;
	ConfigType *m_config;
	int m_eventfd[2];
	int m_exitfd[2];
	// 分条配置
	ConfigStrip *mConfigStrip;
	// 是否启动分条客户端
	bool mIsStrip;
};

#endif
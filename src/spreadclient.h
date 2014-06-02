#ifndef SPREADCLIENT_H
#define SPREADCLIENT_H

#include "config.h"
#include "client.h"

class SpreadClient : public Client {
public:
	SpreadClient(int id, ConfigType *config);
	~SpreadClient() {}

	virtual bool Init();
	// 向服务器发送视频文件请求信息，结束
	virtual void Run();
	virtual void Exit();
private:
	string m_serverip;
	string m_serverport;
};

#endif
#include "servermgr.h"
#include "config.h"
#include "lbserver.h"
#include "stripserver.h"

ServerManager::ServerManager(bool isStrip, void *config):mIsStrip(isStrip)
{
	// 启动分条服务器端
	if(isStrip){
		m_server = new StripServer(0, (ConfigStrip *)config);
	}
	// 启动扩散服务器端
	else{
		m_server = new LBServer(0, (ConfigType *)config);
	}
}

ServerManager::~ServerManager()
{
	delete m_server;
}

void ServerManager::Run()
{
	m_server->Run();
}

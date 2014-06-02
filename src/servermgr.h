#ifndef SERVERMGR_H
#define SERVERMGR_H

#include "server.h"


class ServerManager
{
public:
	ServerManager(bool isStrip, void *config);
	~ServerManager();

	void Run();
	/* data */
public:
	Server *m_server;
	bool mIsStrip;
};



#endif

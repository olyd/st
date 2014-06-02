#include "dbufferthreadsafe.h"

#include <cassert>

#include "utils.h"

#include "dbufferdw.h"
#include "dbufferdws.h"

#include "dbufferdwk.h"

#include "dbufferfifo.h"
#include "dbufferfifos.h"

#include "dbufferlfru.h"

#include "dbufferlfu.h"
#include "dbufferlfus.h"

#include "dbufferlrfu.h"

#include "dbufferlru.h"
#include "dbufferlrus.h"

DBufferThreadSafe::DBufferThreadSafe(int blocksize, int blocknum, int period, int lrfulambda, string strategy)
{
	if (strategy == "dw") {
		m_buffer = new DBufferDW(blocksize, blocknum, period);
	}
	else if (strategy == "dws") {
		m_buffer = new DBufferDWS(blocksize, blocknum, period);
	}
	else if (strategy == "dwk") {
		m_buffer = new DBufferDWK(blocksize, blocknum, period);
	}
	else if (strategy == "fifo") {
		m_buffer = new DBufferFIFO(blocksize, blocknum);
	}
	else if (strategy == "fifos") {
		m_buffer = new DBufferFIFOS(blocksize, blocknum);
	}
	else if (strategy == "lfru") {
		m_buffer = new DBufferLFRU(blocksize, blocknum, period);
	}
	else if (strategy == "lfu") {
		m_buffer = new DBufferLFU(blocksize, blocknum);
	}
	else if (strategy == "lfus") {
		m_buffer = new DBufferLFUS(blocksize, blocknum);
	}
	else if (strategy == "lrfu") {
		m_buffer = new DBufferLRFU(blocksize, blocknum, lrfulambda / 1000.0);
	}
	else if (strategy == "lru") {
		m_buffer = new DBufferLRU(blocksize, blocknum);
	}
	else if (strategy == "lrus") {
		m_buffer = new DBufferLRUS(blocksize, blocknum);
	}
	else {
		assert(0);
	}
	Pthread_mutex_init(&m_mutex, NULL);
}

DBufferThreadSafe::~DBufferThreadSafe()
{
	Pthread_mutex_destroy(&m_mutex);
	delete m_buffer;
}

bool DBufferThreadSafe::Read(int fileid,int segid)
{
	bool ret;
	Pthread_mutex_lock(&m_mutex);
	ret = m_buffer->Read(fileid, segid);
	Pthread_mutex_unlock(&m_mutex);
	return ret;
}

void DBufferThreadSafe::Write(int fileid,int segid,int &ofileid,int &osegid)
{
	Pthread_mutex_lock(&m_mutex);
	m_buffer->Write(fileid, segid, ofileid, osegid);
	Pthread_mutex_unlock(&m_mutex);
}

void DBufferThreadSafe::BlockReset()
{
	Pthread_mutex_lock(&m_mutex);
	m_buffer->BlockReset();
	Pthread_mutex_unlock(&m_mutex);
}

void DBufferThreadSafe::AddVistors(int fileid, int clientid)
{
	Pthread_mutex_lock(&m_mutex);
	m_buffer->AddVistors(fileid, clientid);
	Pthread_mutex_unlock(&m_mutex);
}

void DBufferThreadSafe::DeleteVistors(int fileid, int clientid)
{
	Pthread_mutex_lock(&m_mutex);
	m_buffer->DeleteVistors(fileid, clientid);
	Pthread_mutex_unlock(&m_mutex);
}

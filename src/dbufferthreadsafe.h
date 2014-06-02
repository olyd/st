#ifndef DBUFFERTHREADSAFE_H
#define DBUFFERTHREADSAFE_H

#include <string>

#include <pthread.h>

using namespace std;

class DBuffer;

class DBufferThreadSafe {
public:
	DBufferThreadSafe(int blocksize, int blocknum, int period, int lrfulambda, string strategy);
	~DBufferThreadSafe();
	bool Read(int fileid,int segid);
	void Write(int fileid,int segid,int &ofileid,int &osegid);
	void BlockReset();
	void AddVistors(int fileid, int clientid);
	void DeleteVistors(int fileid, int clientid);
protected:
	DBuffer *m_buffer;
	pthread_mutex_t m_mutex;
};
#endif
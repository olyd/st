#ifndef BUFMANAGERBYLFRU_H
#define BUFMANAGERBYLFRU_H

#include "dbuffer.h"
#include <sys/time.h>

#include <list>

using namespace std;

class DBufferLFRU : public DBuffer{
public:
	DBufferLFRU(int blockSize,int blockNums,int period);
	virtual ~DBufferLFRU();
	virtual bool Read(int fileId,int segId);
	virtual void Write(int fileId,int segId,int &ofileId,int &osegId);
	virtual void Strategy(int fileId,int segId,int &ofileId,int &osegId);
	virtual bool FindBlock(int fileId,int segId);
	int AddBlock(int fileId,int segId);
	void BlockReset();
private:
	list<Block> buf;
	double recallTime;
	double t0;
	unsigned int _period;
};

#endif

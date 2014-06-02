#ifndef BUFMANAGERBYLRFU_H
#define BUFMANAGERBYLRFU_H

#include "dbuffer.h"

#include <list>
#include <sys/time.h>

using namespace std;

class DBufferLRFU: public DBuffer{
public:
	DBufferLRFU(int blockSize,int blockNums,float lambda);
	virtual ~DBufferLRFU();
	virtual bool Read(int fileId,int segId);
	virtual void Write(int fileId,int segId,int &ofileId,int &osegId);
	virtual void Strategy(int fileId,int segId,int &ofileId,int &osegId);
	virtual bool FindBlock(int fileId,int segId);
	int AddBlock(int fileId,int segId);
	virtual void BlockReset(){}
private:
	list<Block> lrfuBuf;
	int initial_parameter();
	float _lambda;
	unsigned int timeslot;
};

#endif

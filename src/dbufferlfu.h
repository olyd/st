#ifndef DBUFFERLFU_H
#define DBUFFERLFU_H

#include "dbuffer.h"
#include <list>

using namespace std;

class DBufferLFU: public DBuffer{
public:
	DBufferLFU(int blockSize,int blockNums);
	virtual ~DBufferLFU();
	virtual bool Read(int fileId,int segId);
	virtual void Write(int fileId,int segId,int &ofileId,int &osegId);
	virtual void Strategy(int fileId,int segId,int &ofileId,int &osegId);
	virtual bool FindBlock(int fileId,int segId);
	int AddBlock(int fileId,int segId);
	virtual void BlockReset(){}
private:
	list<Block> buf;

	int initial_parameter();
};

#endif

#ifndef DBUFFERLFUS_H
#define DBUFFERLFUS_H

#include "dbuffer.h"
#include <list>

using namespace std;

class DBufferLFUS: public DBuffer{
public:
	DBufferLFUS(int blockSize,int blockNums);
	virtual ~DBufferLFUS();
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

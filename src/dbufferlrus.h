#ifndef __D_BUFFERLRUS_H__
#define __D_BUFFERLRUS_H__

#include "dbuffer.h"

#include <list>

using namespace std;

class DBufferLRUS : public DBuffer{
public:
	DBufferLRUS(int blockSize,int blockNum);
	virtual ~DBufferLRUS();
	virtual bool Read(int fileId,int segId);
	virtual void Write(int fileId,int segId,int &ofileId,int &osegId);
	bool FindAndAdjustBlock(int fileId,int segId);
	virtual void Strategy(int fileId,int segId,int &ofileId,int &osegId);
	// void PrintBuffer();
	virtual bool FindBlock(int fileId,int segId);
protected:
	list<Block> m_blockList;
	unsigned int m_curBlockNum;
};


#endif

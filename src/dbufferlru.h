#ifndef __D_BUFFERLRU_H__
#define __D_BUFFERLRU_H__

#include "dbuffer.h"

#include <list>

using namespace std;

class DBufferLRU : public DBuffer{
public:
	DBufferLRU(int blockSize,int blockNum);
	virtual ~DBufferLRU();
	virtual bool Read(int fileId,int segId);
	virtual void Write(int fileId,int segId,int &ofileId,int &osegId);
	bool FindAndAdjustBlock(int fileId,int segId);
	virtual void Strategy(int fileId,int segId,int &ofileId,int &osegId);
	void PrintBuffer();
	virtual bool FindBlock(int fileId,int segId);
protected:
	list<Block> m_blockList;
	unsigned int m_curBlockNum;
};


#endif

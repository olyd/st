#ifndef DBUFFERFIFOS_H_
#define DBUFFERFIFOS_H_

#include "dbuffer.h"

#include <list>

using namespace std;

class DBufferFIFOS : public DBuffer{
public:
	DBufferFIFOS(int blockSize,int blockNum);
	virtual ~DBufferFIFOS();
	virtual bool Read(int fileId,int segId);
	virtual void Write(int fileId,int segId,int &ofileId,int &osegId);
	virtual void Strategy(int fileId,int segId,int &ofileId,int &osegId);
	virtual bool FindBlock(int fileId,int segId);
protected:
	list<Block> m_blockList;
};


#endif /* DBUFFERFIFOS_H_ */
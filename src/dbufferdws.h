#ifndef DBUFFERDWS_H
#define DBUFFERDWS_H

#include "dbuffer.h"

#include <map>
#include <set>
#include <list>

using namespace std;

class DBufferDWS: public DBuffer {
public:
	DBufferDWS(int blockSize, int blockNum, int period);
	virtual ~DBufferDWS();
	virtual bool Read(int fileId, int segId);
	virtual void Write(int fileId, int segId, int &ofileId, int &osegId);
	virtual void Strategy(int fileId, int segId, int &ofileId, int &osegId);
	virtual void BlockReset();
	virtual bool FindBlock(int fileId,int segId);
private:
	void AddBlock(int fileId, int segId);
	list<Block> mDWSQueue;
};

#endif

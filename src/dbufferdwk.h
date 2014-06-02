#ifndef DBUFFERDWK_H
#define DBUFFERDWK_H

#include "dbuffer.h"
#include <cmath>
#include <sys/time.h>

#include <list>

using namespace std;

class DBufferDWK: public DBuffer {
public:
	DBufferDWK(int blockSize, int blockNum, int period);
	virtual ~DBufferDWK();
	virtual bool Read(int fileId, int segId);
	virtual void Write(int fileId, int segId, int &ofileId, int &osegId);
	virtual void Strategy(int fileId, int segId, int &ofileId, int &osegId);
	virtual void BlockReset();
	virtual bool FindBlock(int fileId,int segId);
private:
	void AddBlock(int fileId, int segId, double hitTime);

	int mPeriod;
	list<Block> mDWKueue;
};

#endif

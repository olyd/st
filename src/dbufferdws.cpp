#include "dbufferdws.h"
#include <cassert>

DBufferDWS::DBufferDWS(int blockSize, int blockNum, int period): DBuffer(blockSize,blockNum)
{
	// dws need refresh time window to execute BlockReset()
	// so set true
	m_isblockReset = true;
	mDWSQueue.clear();
}

DBufferDWS::~DBufferDWS()
{
	mDWSQueue.clear();
}

bool DBufferDWS::Read(int fileId, int segId)
{
	assert(mDWSQueue.size() <= mBlockNums);
	for(list<Block>::iterator it = mDWSQueue.begin(); it != mDWSQueue.end(); ++it) {
		if (it->fileId == fileId && it->segId == segId) {
			++(it->hitNew);
			Block block = *it;
			mDWSQueue.erase(it);
			mDWSQueue.push_back(block);
			return true;
		}
	}
	return false;
}

void DBufferDWS::Write(int fileId, int segId, int &ofileId, int &osegId)
{
	ofileId = -1;
	osegId = -1;
	assert(mDWSQueue.size() <= mBlockNums);
	if (mDWSQueue.size() == mBlockNums)
		Strategy(fileId, segId, ofileId, osegId);
	else
		AddBlock(fileId, segId);
}

void DBufferDWS::Strategy(int fileId, int segId, int &ofileId, int &osegId)
{
	assert(mDWSQueue.size() == mBlockNums);
	const int MAX = 99999999;
	int minWeight = MAX;
	list<Block>::iterator iter, minIter;

	for(iter = mDWSQueue.begin(); iter != mDWSQueue.end(); ++iter){
		iter->weight = (iter->hitNew + iter->hitOld) * GetVistorNum(iter->fileId);
		if (iter->weight < minWeight) {
			minWeight = iter->weight;
			minIter = iter;
		}
	}
	ofileId = minIter->fileId;
	osegId = minIter->segId;
	mDWSQueue.erase(minIter);
	AddBlock(fileId,segId);
}

/*
 * time window end, block reset
 */
void DBufferDWS::BlockReset()
{
	for(list<Block>::iterator it = mDWSQueue.begin(); it != mDWSQueue.end(); ++it) {
		it->hitOld = it->hitNew;
		it->hitNew = 0;
	}
}

bool DBufferDWS::FindBlock(int fileId,int segId)
{
	for(list<Block>::iterator it = mDWSQueue.begin(); it != mDWSQueue.end(); ++it) {
		if (it->fileId == fileId && it->segId == segId) {
			return true;
		}
	}
	return false;
}

void DBufferDWS::AddBlock(int fileId, int segId)
{
	Block newBlock(fileId, segId);
	mDWSQueue.push_back(newBlock);
}


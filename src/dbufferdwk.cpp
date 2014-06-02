#include "dbufferdwk.h"
#include <cassert>
#include "utils.h"

DBufferDWK::DBufferDWK(int blockSize, int blockNum, int period): DBuffer(blockSize,blockNum)
{
	// dws need refresh time window to execute BlockReset()
	// so set true
	m_isblockReset = true;
	mPeriod = period;
	// mResetTime = getRelativeTime() * 1.0 / 1000000;
	mDWKueue.clear();
}

DBufferDWK::~DBufferDWK()
{
	mDWKueue.clear();
}

bool DBufferDWK::Read(int fileId, int segId)
{
	assert(mDWKueue.size() <= mBlockNums);
	for(list<Block>::iterator it = mDWKueue.begin(); it != mDWKueue.end(); ++it) {
		if (it->fileId == fileId && it->segId == segId) {
			++(it->hitNew);
			it->hitTime = getRelativeTime() * 1.0 / 1000000;
	/*		Block block = *it;
			mDWKueue.erase(it);
			mDWKueue.push_back(block);*/
			return true;
		}
	}
	return false;
}

void DBufferDWK::Write(int fileId, int segId, int &ofileId, int &osegId)
{
	ofileId = -1;
	osegId = -1;
	assert(mDWKueue.size() <= mBlockNums);
	if (mDWKueue.size() == mBlockNums)
	{
		Strategy(fileId, segId, ofileId, osegId);
	}
	else
	{
		AddBlock(fileId, segId, getRelativeTime() * 1.0 / 1000000);
	}
}

void DBufferDWK::Strategy(int fileId, int segId, int &ofileId, int &osegId)
{
	assert(mDWKueue.size() == mBlockNums);
	const int MAX = 99999999;
	double minWeight = MAX;
	double strategyTime = getRelativeTime() * 1.0 / 1000000;


	list<Block>::iterator iter, minIter;

	for(iter = mDWKueue.begin(); iter != mDWKueue.end(); ++iter){
		iter->weight = (iter->hitNew + iter->hitOld) * (1 - (strategyTime - iter->hitTime ) / mPeriod );
		if (iter->weight < minWeight) {
			minWeight = iter->weight;
			minIter = iter;
		}
	}
	ofileId = minIter->fileId;
	osegId = minIter->segId;
	mDWKueue.erase(minIter);
	AddBlock(fileId,segId, strategyTime);
}

/*
 * time window end, block reset
 */
void DBufferDWK::BlockReset()
{
	// mResetTime = getRelativeTime() * 1.0 / 1000000;
	for(list<Block>::iterator it = mDWKueue.begin(); it != mDWKueue.end(); ++it) {
		it->hitOld = it->hitNew;
		it->hitNew = 0;
		// it->hitTime = mResetTime;
	}
}

bool DBufferDWK::FindBlock(int fileId,int segId)
{
	for(list<Block>::iterator it = mDWKueue.begin(); it != mDWKueue.end(); ++it) {
		if (it->fileId == fileId && it->segId == segId) {
			return true;
		}
	}
	return false;
}

void DBufferDWK::AddBlock(int fileId, int segId, double hitTime)
{
	Block newBlock(fileId, segId, hitTime);
	mDWKueue.push_back(newBlock);
}

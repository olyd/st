#ifndef DBUFFER_H_
#define DBUFFER_H_

struct Block {
	Block(){ };
	Block(int _fileid, int _segid): fileId(_fileid), segId(_segid), hitNew(1),
		hitOld(0), weight(0), counts(0), hitTime(0.0) {}
	Block(int _fileid, int _segid, double _hitTime): fileId(_fileid), segId(_segid), hitNew(1),
		hitOld(0), weight(0), counts(0), hitTime(_hitTime) {}
	int fileId;
	int segId;
	int hitNew;
	int hitOld;
	int weight;
	int counts;
	double hitTime;
};

#include <iostream>
#include <map>
#include <set>
#include <cassert>

using namespace std;

class DBuffer{
public:
	DBuffer(int blocksize,int blocknum): 
		mBlockSize(blocksize), 
		mBlockNums(blocknum),
		m_isblockReset(false) {}
	virtual ~DBuffer() {}
	virtual bool Read(int fileid,int segid) = 0;
	virtual void Write(int fileid,int segid,int &ofileid,int &osegid) = 0;
	virtual void Strategy(int fileid,int segid,int &ofileid,int &osegid) = 0;
	inline bool IsBlockReset()
	{
		return m_isblockReset;
	}
	virtual void BlockReset() {}
	virtual bool FindBlock(int fileid,int segid) = 0;
	inline int GetVistorNum(int fileid){
		return mFileVistors[fileid].size();
	}
	inline void AddVistors(int fileid, int clientid) {
		mFileVistors[fileid].insert(clientid);
	}
	inline void DeleteVistors(int fileid, int clientid) {
		mFileVistors[fileid].erase(clientid);
	}
protected:
	int mBlockSize;
	unsigned int mBlockNums;
	bool m_isblockReset;
	map<int, set<int> > mFileVistors;
};
#endif
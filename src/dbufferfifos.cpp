#include "dbufferfifos.h"

DBufferFIFOS::DBufferFIFOS(int blockSize,int blockNum)
	: DBuffer(blockSize,blockNum)
{
	m_blockList.clear();
}

DBufferFIFOS::~DBufferFIFOS(){
	m_blockList.clear();
}

bool DBufferFIFOS::FindBlock(int fileId,int segId){//,bool locked){
	if(!m_blockList.empty()){
		std::list<Block>::iterator listIter = m_blockList.begin();
		while(listIter != m_blockList.end()){
			if(listIter->fileId == fileId && listIter->segId == segId){
				return true;
			}
			listIter++;
		}
	}
	return false;
}

bool DBufferFIFOS::Read(int fileId,int segId){
	return FindBlock(fileId,segId);
}

void DBufferFIFOS::Write(int fileId,int segId,int &ofileId,int &osegId){
	ofileId = -1;
	osegId = -1;
	assert(m_blockList.size() <= mBlockNums);
	if(m_blockList.size() < mBlockNums){
		m_blockList.push_back(Block(fileId,segId));
	}
	else{
		Strategy(fileId,segId,ofileId,osegId);
	}
}


void DBufferFIFOS::Strategy(int fileId,int segId,int &ofileId,int &osegId){
	const int MAX = 99999999;
	int minWeight = MAX;
	int tempWeight;

	list<Block>::iterator iter, minIter;
	for(iter = m_blockList.begin(); iter != m_blockList.end(); iter++){
		tempWeight = GetVistorNum(iter->fileId);
		if(tempWeight < minWeight){
			minWeight = tempWeight;
			minIter = iter;
		}
	}
	ofileId = minIter->fileId;
	osegId = minIter->segId;
	m_blockList.erase(minIter);
	m_blockList.push_back(Block(fileId,segId));
}
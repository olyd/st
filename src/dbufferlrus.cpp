#include "dbufferlrus.h"

DBufferLRUS::DBufferLRUS(int blockSize,int blockNum)
	: DBuffer(blockSize,blockNum)
{
	m_blockList.clear();
	m_curBlockNum = 0;
}

DBufferLRUS::~DBufferLRUS(){
	m_blockList.clear();
}

bool DBufferLRUS::FindBlock(int fileId,int segId){//,bool locked){
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

bool DBufferLRUS::FindAndAdjustBlock(int fileId,int segId){
	if(!m_blockList.empty()){
		std::list<Block>::iterator listIter = m_blockList.begin();
		while(listIter != m_blockList.end()){
			if(listIter->fileId == fileId && listIter->segId == segId){
				Block block = *listIter;
				m_blockList.erase(listIter);
				m_blockList.push_front(block);
				return true;
			}
			listIter++;
		}
	}
	return false;
}

bool DBufferLRUS::Read(int fileId,int segId){
	if(m_blockList.empty()){
		return false;
	}
	if(!(m_blockList.front().fileId == fileId && m_blockList.front().segId == segId)){
		if(!FindAndAdjustBlock(fileId,segId)){
			return false;
		}
	}
	return true;
}

void DBufferLRUS::Write(int fileId,int segId,int &ofileId,int &osegId){
	ofileId = -1;
	osegId = -1;
	if(m_curBlockNum < mBlockNums){
		m_blockList.push_front(Block(fileId,segId));
		m_curBlockNum++;
	}
	else{
		Strategy(fileId,segId,ofileId,osegId);
	}
}

//LRUS算法
void DBufferLRUS::Strategy(int fileId,int segId,int &ofileId,int &osegId){
	const int MAX = 99999999;
	int minWeight = MAX;
	list<Block>::iterator iter, minIter;

	for(iter = m_blockList.begin(); iter != m_blockList.end(); ++iter){
		iter->weight = GetVistorNum(iter->fileId);
		if (iter->weight <= minWeight) {
			minWeight = iter->weight;
			minIter = iter;
		}
	}

	ofileId = minIter->fileId;
	osegId = minIter->segId;
	m_blockList.erase(minIter);
	m_blockList.push_front(Block(fileId,segId));
}

/*void DBufferLRUS::PrintBuffer(){
	std::list<Block>::iterator listIter = m_blockList.begin();
	while(listIter != m_blockList.end()){
		std::cout << listIter->fileId << " " << listIter->segId << ":";
		listIter++;
	}
	std::cout << std::endl;
}*/

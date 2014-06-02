#include "dbufferlru.h"

DBufferLRU::DBufferLRU(int blockSize,int blockNum)
	: DBuffer(blockSize,blockNum)
{
	m_blockList.clear();
	m_curBlockNum = 0;
}

DBufferLRU::~DBufferLRU(){
	m_blockList.clear();
}

bool DBufferLRU::FindBlock(int fileId,int segId){//,bool locked){
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

bool DBufferLRU::FindAndAdjustBlock(int fileId,int segId){
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

bool DBufferLRU::Read(int fileId,int segId){
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

void DBufferLRU::Write(int fileId,int segId,int &ofileId,int &osegId){
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

//LRU算法
void DBufferLRU::Strategy(int fileId,int segId,int &ofileId,int &osegId){
	list<Block>::iterator listIter = m_blockList.end();
	listIter--;
	ofileId = listIter->fileId;
	osegId = listIter->segId;
	m_blockList.erase(listIter);
	m_blockList.push_front(Block(fileId,segId));
}

/*void DBufferLRU::PrintBuffer(){
	std::list<Block>::iterator listIter = m_blockList.begin();
	while(listIter != m_blockList.end()){
		std::cout << listIter->fileId << " " << listIter->segId << ":";
		listIter++;
	}
	std::cout << std::endl;
}*/

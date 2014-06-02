#include "dbufferlfus.h"
#include <cassert>

DBufferLFUS::DBufferLFUS(int blockSize,int blockNums)
: DBuffer(blockSize,blockNums)
{
	buf.clear();
	initial_parameter();
}

DBufferLFUS::~DBufferLFUS(){
	buf.clear();
}

int DBufferLFUS::initial_parameter(){
	list<Block>::iterator it;
	for(it = buf.begin();it!=buf.end();it++){
		it->counts = 0;
	}
	return 0;
}

void DBufferLFUS::Write(int fileId,int segId,int &ofileId,int &osegId){
	ofileId = -1;
	osegId = -1;
	if(buf.size() >= mBlockNums){
		Strategy(fileId,segId,ofileId,osegId);
	}
	else{
		AddBlock(fileId,segId);
	}
}

bool DBufferLFUS::Read(int fileId,int segId){
	Block temp;
	temp.fileId = fileId;
	temp.segId = segId;
	list<Block>::iterator it;
	int flag = 0;

	for(it = buf.begin();it!=buf.end();it++){
		if( it->fileId ==temp.fileId && it->segId == temp.segId){
			it->counts++;
			flag = 1;
			break;
		}
	}

	if(flag == 0){
		cout<<"in dbufferlfus: can't find the segment <"<<fileId<<","<<segId<<">"<<endl;
		return false;
	}
	return true;
}

bool DBufferLFUS::FindBlock(int fileId,int segId){
	list<Block>::iterator it;
	for(it = buf.begin();it !=buf.end();it++){
		if(it->fileId == fileId && it->segId == segId){
			return true;
		}
	}
	return false;
}

void DBufferLFUS::Strategy(int fileId,int segId,int &ofileId,int &osegId){
	list<Block>::iterator it, minIt;
	int minWeight = 0x7fffffff;

	for(it = buf.begin();it !=buf.end();it++){
		int tempWeight = it->counts * GetVistorNum(it->fileId);
		if(tempWeight < minWeight){
			minWeight = tempWeight;
			minIt=it;
		}
	}

	ofileId = minIt->fileId;
	osegId = minIt->segId;

	buf.erase(minIt);
	AddBlock(fileId,segId);
}

int DBufferLFUS::AddBlock(int fileId,int segId){
	Block temp;
	temp.fileId = fileId;
	temp.segId = segId;
	temp.counts = 1;
	buf.push_back(temp);
	return 0;
}

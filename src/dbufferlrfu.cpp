#include "dbufferlrfu.h"
#include <float.h>
#include <math.h>

const double PARA = 0.5;// for p = 2

DBufferLRFU::DBufferLRFU(int blockSize,int blockNums,float lambda)
:DBuffer(blockSize,blockNums)
{
	_lambda = lambda;
	timeslot = 0;
	lrfuBuf.clear();
}

DBufferLRFU::~DBufferLRFU(){
	lrfuBuf.clear();
}

int DBufferLRFU::initial_parameter(){
	list<Block>::iterator it;
	for(it = lrfuBuf.begin();it!=lrfuBuf.end();it++){
		it->weight = 0.0;
	}
	return 0;
}

void DBufferLRFU::Write(int fileId,int segId,int &ofileId,int &osegId){
	ofileId = -1;
	osegId = -1;
	if(lrfuBuf.size() >= mBlockNums){
		Strategy(fileId,segId,ofileId,osegId);
	}
	else{
		AddBlock(fileId,segId);
	}
}

bool DBufferLRFU::Read(int fileId,int segId){
	Block temp;
	temp.fileId = fileId;
	temp.segId = segId;
	list<Block>::iterator it;
	int flag = 0;
	for(it = lrfuBuf.begin();it!=lrfuBuf.end();it++){
		if( it->fileId ==temp.fileId && it->segId == temp.segId){
			it->weight = (it->weight)*pow(PARA,((timeslot - (it->hitTime))*_lambda)) + 1.0;
			it->hitTime = timeslot;
			timeslot++;
			flag = 1;
			break;
		}
	}
	if(flag == 0){
		cout<<"in DBufferLRFU: can't find the segment <"<<fileId<<","<<segId<<">"<<endl;
		return false;
	}
	return true;
}

bool DBufferLRFU::FindBlock(int fileId,int segId){
	list<Block>::iterator it;
	for(it = lrfuBuf.begin();it !=lrfuBuf.end();it++){
		if(it->fileId == fileId && it->segId == segId){
			return true;
		}
	}
	return false;
}

void DBufferLRFU::Strategy(int fileId,int segId,int &ofileId,int &osegId){
	list<Block>::iterator it, minIt;
	float minWeight = 111111111.0;
	double temp;
	for(it = lrfuBuf.begin();it !=lrfuBuf.end();it++){
		temp=(it->weight)*pow(PARA,((timeslot - (it->hitTime))*_lambda)) ;
		if(temp < minWeight){
			minWeight = temp;
			minIt=it;
		}
		//cout << fileId << "," << segId << " weight:" << it->weight << ",timeslot" << timeslot << ",lasttime:" << it->hitTime << ",lambda:" << _lambda << endl;

//		if(it->weight < minWeight){
//			minWeight = it->weight;
//			minIt=it;
//		}
	}
	ofileId = minIt->fileId;
	osegId = minIt->segId;
	lrfuBuf.erase(minIt);
	timeslot++;

	AddBlock(fileId,segId);
}

int DBufferLRFU::AddBlock(int fileId,int segId){
	Block temp;
	temp.fileId = fileId;
	temp.segId = segId;
	temp.hitTime =timeslot;
	temp.weight = 1.0;
	lrfuBuf.push_back(temp);
	return 0;
}

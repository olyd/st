#include "dbufferlfru.h"
#include <cassert>
#include "utils.h"

long long int GetTimeIntervallfru(struct timeval *a,struct timeval *b){
	long long int usec;
	usec = a->tv_sec - b->tv_sec;
	usec = 1000000*usec;
	usec += a->tv_usec - b->tv_usec;
	return usec;
}

DBufferLFRU::DBufferLFRU(int blockSize,int blockNums,int period)
: DBuffer(blockSize,blockNums)
{
	t0 = getRelativeTime();	
	assert(period !=0);
	_period = period;

	buf.clear();
	m_isblockReset = true;
}

DBufferLFRU::~DBufferLFRU(){
	buf.clear();
}

void DBufferLFRU::BlockReset(){
    list<Block>::iterator it;
	t0 = getRelativeTime();	
    for(it = buf.begin();it!=buf.end();it++){
        it->counts = 1;
    }
}

void DBufferLFRU::Write(int fileId,int segId,int &ofileId,int &osegId){
	ofileId = -1;
	osegId = -1;
	if(buf.size() >= mBlockNums){
		Strategy(fileId,segId,ofileId,osegId);
	}
	else{
		AddBlock(fileId,segId);
	}
}

bool DBufferLFRU::Read(int fileId,int segId){
	list<Block>::iterator it;
	int flag = 0;

	for(it = buf.begin();it!=buf.end();it++){
		if( it->fileId ==fileId && it->segId == segId){
			it->counts++;
			it->hitTime = getRelativeTime();	
			flag = 1;
			break;
		}
	}

	if(flag == 0){
		cout<<"in bufManagerByLFRU: can't find the segment <"<<fileId<<","<<segId<<">"<<endl;
		return false;
	}
	return true;
}

bool DBufferLFRU::FindBlock(int fileId,int segId){
	list<Block>::iterator it;
	for(it = buf.begin();it !=buf.end();it++){
		if(it->fileId == fileId && it->segId == segId){
			return true;
		}
	}
	return false;
}

void DBufferLFRU::Strategy(int fileId,int segId,int &ofileId,int &osegId){
	double Fk;
	double  Rk;
	double tt,tr;
	double weight;
	list<Block>::iterator it, maxIt;
	double maxWeight =0.0;
	recallTime = getRelativeTime();
	maxIt = buf.begin();
	tt = recallTime - t0;
	if( tt < 0 ) tt= 0;
	if( tt > (double) _period) tt = _period;
	for(it = buf.begin();it !=buf.end();it++){
		tr = recallTime - it->hitTime;
		if( tr < 0 ) tr = 0;
		Fk = tt/it->counts;
		Rk = tr;
		weight =(double )_period- tt;
		weight = weight * Rk /(double)_period ;
		weight += tt*Fk/ _period ;
	//	cout << fileId << "," << it->segId << "Fk:" << Fk << ",Rk:" << Rk << ",period:" << _period << ",recallTime:" << recallTime << ",weight:" << it->weight << ",t0:" << t0 << ",counts:" << it->counts << ",accessTime:" << it->hitTime << endl;
		if(weight > maxWeight){
			maxWeight = weight;
			maxIt=it;
		}
	}
	ofileId = maxIt->fileId;
	osegId = maxIt->segId;
	buf.erase(maxIt);
	//cout<<"eliminate "<<ofileId<<","<<osegId<<endl;
	AddBlock(fileId,segId);
}

int DBufferLFRU::AddBlock(int fileId,int segId){
	Block temp;
	temp.fileId = fileId;
	temp.segId = segId;
	temp.counts = 1;
	temp.hitTime = getRelativeTime();
	buf.push_back(temp);
	return 0;
}

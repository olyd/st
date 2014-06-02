/*
 * DataServer.cpp
 *
 *  Created on: 2013-2-20
 *      Author: zhaojun
 */
#include <cassert>
#include <cmath>
#include "dataserver.h"
#include "utils.h"
#include "log.h"

DataServer::DataServer(int fileNum,int minLength,int maxLength,int blockSize,
		double minBitRate,double maxBitRate, int diskNum){
	mFileNum = fileNum;
	assert(mFileNum <= MAX_FILE_NUM);
	mDiskNum = diskNum;
	assert(mDiskNum <= MAX_DISK_NUM);

	for(int i=0; i < mDiskNum; i++){
		mDiskLoad[i] = 0;
	}
	
	for(int i = 1;i <= mFileNum;i++){
		mFileInfo[i].bitRate = Randomf(minBitRate,maxBitRate);
		mFileInfo[i].fileId = i;
		mFileInfo[i].segNum = 0;
		mFileInfo[i].info.clear();
	}

	for(int i = 1;i <= mFileNum;i++){
		int length = Randomi(minLength,maxLength);
		mFileInfo[i].segNum = length / blockSize;
	}

	m_real_load = 0;
	m_total_request = 0;
	m_read_from_server = 0;
	m_buffer_hit = 0;
	m_buffer_miss = 0;
}

DataServer::~DataServer(){
	for(int i = 1;i <= mFileNum;i++){
		mFileInfo[i].info.clear();
	}
}

int DataServer::SearchBestClient(int fileId,int segId){
	int bestClient = -1;
	int minLinked = 1000000;
	if(!mFileInfo[fileId].info.empty()){
		list<FileInfoBlock>::iterator iter = mFileInfo[fileId].info.begin();
		while(iter != mFileInfo[fileId].info.end()){
			if(iter->segId == segId && mClientLinks[iter->clientNum] < minLinked &&
					mClientLinks[iter->clientNum] <= (MAX_CLIENT_LINKS * 2)){
				minLinked = mClientLinks[iter->clientNum];
				bestClient = iter->clientNum;
			}
			iter++;
		}
	}
	if(bestClient != -1){
		mClientLinks[bestClient]++;
	}
	return bestClient;
}

void DataServer::GetFileInfo(int fileId,double *bitRate,int *segNum){
	*bitRate = mFileInfo[fileId].bitRate;
	*segNum = mFileInfo[fileId].segNum;
}

void DataServer::InsertIntoIndex(int fileId,int segId,int clientNum,int linkedNum){
	DeleteFromIndex(fileId,segId,clientNum);

	FileInfoBlock fileInfoBlock;
//	fileInfoBlock.linkedNum = linkedNum;
	fileInfoBlock.clientNum = clientNum;
	mClientLinks[clientNum] = linkedNum;
	fileInfoBlock.segId = segId;

	mFileInfo[fileId].info.push_back(fileInfoBlock);
}

void DataServer::DeleteFromIndex(int fileId,int segId,int clientNum){
	list<FileInfoBlock>::iterator iter = mFileInfo[fileId].info.begin();
	while(iter != mFileInfo[fileId].info.end()){
		if(iter->segId == segId && iter->clientNum == clientNum){
			list<FileInfoBlock>::iterator tmpIter = iter;
			iter++;
			mFileInfo[fileId].info.erase(tmpIter);
			continue;
		}
		iter++;
	}
}

// return:	0 ~ mDiskNum-1
int DataServer::GetDiskId(int fileId, int segId, string placeStrategy){
	int ret;
	int i;
	int temp = 0;
	list<BlockPosition>::iterator iter = positionList.begin();
	if(placeStrategy == "rr"){// rr
		for(i=1; i<fileId; i++){
			temp += mFileInfo[fileId].segNum;
		}
		temp += segId;
		ret = temp % mDiskNum;
	}else if(placeStrategy == "ram"){// random
		for(;iter!=positionList.end();++iter){
			if(iter->fileId == fileId && iter->segId == segId){
				return iter->diskId;
			}
		}
		ret = Randomi(0, mDiskNum - 1);
		positionList.push_back(BlockPosition(fileId, segId, ret));
	}else if(placeStrategy == "fdrr"){// first segid different rr
		temp = (fileId - 1) % mDiskNum;
		ret = ( temp + (segId -1)% mDiskNum ) % mDiskNum;
	}
	else{
		assert(0);
	}
	assert(ret >=0 && ret < mDiskNum);
	return ret;
}

void DataServer::IncreaseDiskAccessCount(int diskId){
	++mDiskLoad[diskId];
}

int DataServer::GetDiskAccessCount(int diskId){
	return mDiskLoad[diskId];
}

double DataServer::GetAccessBalanceDegree(list<int> &accessCountList){
	double average = 0;
	double variance;
	for(int i=0; i < mDiskNum; i++){
		average += mDiskLoad[i];
		accessCountList.push_back(mDiskLoad[i]);
	}
	average /= mDiskNum;
	for(int i=0; i <mDiskNum; i++){
		double temp = fabs(average - mDiskLoad[i]);
		variance += temp * temp;
	}
	variance /= mDiskNum;
	return variance;
}
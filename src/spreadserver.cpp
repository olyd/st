#include "spreadserver.h"
#include <algorithm>
#include <cassert>

#include "spreadstrategy.h"
#include "utils.h"
#include "log.h"

// FileNode::FileNode()
// {
// 	hitnew = 1;
// 	hitold = 0;
// 	vtime = getRelativeTime();
// 	status = NORMAL;
// 	load = 0;
// }

SpreadServer::SpreadServer(int serverid, ConfigType *config):Server(serverid)
{
	mFileLength = config->fileLength;
	mMaxDiskBand = config->maxDiskBand;

	mMaxLoad = config->maxLoad;
	mCurrentLoad = 0;
	mLastPeriodLoad = 0;
	mCurrentPeriodLoad = 0;

	mMaxCapacity = config->maxCapacity;
	mCurrentCapacity = 0;

	mThreshold = config->loadThresh;
	m_load_thresh_high = config->loadThreshHigh;
	LOG_INFO("server " << setw(2) << mServerId << " loadthresh " << mThreshold << " : " << m_load_thresh_high);	

	m_max_copy_flow = config->maxCopyFlow;
	m_copy_flow = 0;
	m_max_in_flow = config->maxInFlow;
	m_in_flow = 0;
	assert(m_max_copy_flow >= 0);

	mStrategyName = config->spreadAlgorithm;
	if (config->spreadAlgorithm == "dw")
		m_spread_strategy = new DWSpreadStrategy();
	else if (config->spreadAlgorithm == "lru")
		m_spread_strategy = new LRUSpreadStrategy();
	else if (config->spreadAlgorithm == "fifo")
		m_spread_strategy = new FIFOSpreadStrategy();
	else if (config->spreadAlgorithm == "lfu")
		m_spread_strategy = new LFUSpreadStrategy();
	else {
		LOG_INFO("doesn't exist spread algorithm: " << config->spreadAlgorithm);
		assert(0);
	}

	mPeriod = config->period;
	mPeriodStartTime = getRelativeTime();
}
SpreadServer::~SpreadServer()
{
	delete m_spread_strategy;
}
bool SpreadServer::Init(ConfigType *config)
{
	return true;
}
void SpreadServer::Run(){}


void SpreadServer::PrintFileListInServer(){
	vector<FileNode>::iterator iter;
	LOG_INFO_NOENDL("current filelist in server " << setw(2) << mServerId << ":\t");
	for (iter = m_filelist.begin(); iter != m_filelist.end(); ++iter) {
		if(iter->status != SPREADING )
		{
			LOG_NOTIME_NOENDL(setw(3) << iter->fileid << " ");
		}
	}
	LOG_NOTIME_NOENDL(endl);
}
void SpreadServer::PrintSpreadingFileListInServer(){
	vector<FileNode>::iterator iter;
	LOG_INFO_NOENDL("spreading filelist in server " << setw(2) << mServerId << ":\t");
	for (iter = m_filelist.begin(); iter != m_filelist.end(); ++iter) {
		if(iter->status == SPREADING )
		{
			LOG_NOTIME_NOENDL(setw(3) << iter->fileid << " ");
		}
	}
	LOG_NOTIME_NOENDL(endl);
}
bool SpreadServer::AddFile(int fileid, bool firsttime)
{
	LOG_INFO("server " << setw(2) << mServerId << " add file: " << setw(3) << fileid);
	assert(mCurrentCapacity < mMaxCapacity && mCurrentCapacity >= 0);
	FileNode newfile;
	newfile.fileid = fileid;
	if(firsttime){
		newfile.vtime = 0.0;
	}
	m_filelist.push_back(newfile);
	++mCurrentCapacity;
	assert((unsigned int)mCurrentCapacity == m_filelist.size());
	return true;
}
void SpreadServer::DeleteFile(int fileid)
{
	LOG_INFO("server " << setw(2) << mServerId << " delete file: " << setw(3) << fileid);
	assert(mCurrentCapacity == mMaxCapacity);
	vector<FileNode>::iterator iter;
	for (iter = m_filelist.begin(); iter != m_filelist.end(); ++iter) {
		if (iter->fileid == fileid) break;
	}
	assert(iter != m_filelist.end());
	m_filelist.erase(iter);
	--mCurrentCapacity;;
}
bool SpreadServer::SearchFile(int fileid)
{
	vector<FileNode>::iterator iter;
	for (iter = m_filelist.begin(); iter != m_filelist.end(); ++iter) {
		if (iter->fileid == fileid) return true;
	}
	return false;
}
int SpreadServer::GetSpreadFile(vector<int> &output)
{
	return m_spread_strategy->GetSpreadFile(m_filelist, output);
}
bool VTimeCompare(FileNode a, FileNode b)
{
	if(a.vtime < b.vtime)
		return true;
	else if(a.vtime == b.vtime && a.fileid > b.fileid)
		return true;
	else 
		return false;
}
// 子服务器返回 没有客户端访问 且 不是正在扩散 的文件
// 按照访问时间从小到大排序
int SpreadServer::GetDeleteFileList(vector<int> &output)
{
	return m_spread_strategy->GetDeleteFile(m_filelist, output);
}
void SpreadServer::Reset()
{
	mLastPeriodLoad = mCurrentPeriodLoad;
	mCurrentPeriodLoad = mCurrentLoad;
	// 新的周期开始时间
	mPeriodStartTime = getRelativeTime();

	m_spread_strategy->Reset(m_filelist);
}



bool SpreadServer::IsOverCapacity()
{
	LOG_INFO("server " << setw(2) << mServerId << " (currentCapacity, maxCapacity): (" << mCurrentCapacity << ", " << mMaxCapacity << ")");
	return (mCurrentCapacity >= mMaxCapacity);
}
bool SpreadServer::IsNeedSpread()
{
	double tempTime = getRelativeTime();
	mTimeInPeriod = tempTime - mPeriodStartTime;
	if (mStrategyName == "dw"){
		LOG_INFO("mTimeInPeriod: " << mTimeInPeriod << " period: " << mPeriod);
	}

	vector<FileNode>::iterator iter;
	int spreaded_load = 0;
	for (iter = m_filelist.begin(); iter != m_filelist.end(); ++iter) {
		// 扩散进来的文件的初始load=0,所以没有影响
		if (iter->status == SPREADED || iter->status == SPREADING) {
			spreaded_load += iter->load;
		}
	}
	double threshhold = (mCurrentLoad - spreaded_load) * 1.0 / (mMaxLoad - spreaded_load);
	LOG_INFO("IsNeedSpread (thresh > startT ?)  thresh:" << threshhold << ", startT:" << mThreshold); 
	LOG_INFO("IsNeedSpread (COF < COFmax ?)  COF:" << m_copy_flow << ", COFmax:" << m_max_copy_flow); 
	LOG_INFO("IsNeedSpread  N:" << mCurrentLoad << ", Nmax:" << mMaxLoad); 
	LOG_INFO("IsNeedSpread (N - COF <= Nmax ?) " << (mCurrentLoad - m_copy_flow <= mMaxLoad)); 
	
	double left, right;
	if (mStrategyName == "dw"){
		// 这是新的条件，add by mjq @2014.04.30
		// N*(D/M + (N-N0)/(T+dt)) < D*Nmax/M
		// N*(1 + (N-N0)*M/(T+dt)/D) < Nmax
		left = mCurrentLoad * ( 1 + (mCurrentLoad - mLastPeriodLoad) * mFileLength * 1.0 / (mPeriod + mTimeInPeriod) / mMaxDiskBand);
		right = mMaxLoad;
	}else{
		// 这是旧的条件
		left = mCurrentLoad * (mCurrentLoad - spreaded_load) * 1.0 / (mMaxLoad - mCurrentLoad) / getRelativeTime();
		right = mMaxDiskBand * 1.0 / mFileLength;
	}
	LOG_INFO("IsNeedSpread (left < right ?)  left:" << left << ", right:" << right); 

	if (threshhold >= mThreshold && 
			(m_copy_flow < m_max_copy_flow) && 
				(mCurrentLoad - m_copy_flow <= mMaxLoad ) && 
					(left < right)){
		LOG_INFO("IsNeedSpread return true for server " << mServerId); 
		return true;		
	}
	LOG_INFO("IsNeedSpread return false for server " << mServerId); 
	return false;
}
bool SpreadServer::IsCanBeTarget()
{
	//if (IsOverLoad()) return false;
	if (m_in_flow >= m_max_in_flow){
		LOG_INFO("IsCanBeTarget return false,  CIF:" << m_in_flow << ", CIFmax:" << m_max_in_flow); 
		return false;
	}

	// 目标服务器负载满足
	double left, right;
	if(mStrategyName == "dw"){
		// 这是新的条件，add by mjq @2014.04.30
		// N*(D/M + (N-N0)/(T+dt)) < D*Nmax/M
		// N*(1 + (N-N0)*M/(T+dt)/D) < Nmax
		left = mCurrentLoad * ( 1 + (mCurrentLoad - mLastPeriodLoad) * mFileLength * 1.0 / (mPeriod + mTimeInPeriod) / mMaxDiskBand);
		right = mMaxLoad;
		if(left >= right){
			LOG_INFO("IsCanBeTarget return false, left:" << left << ", right:" << right); 
			return false;
		}
	}else{
		// 这是旧的条件
		left = (mMaxLoad - mCurrentLoad) * 1.0 * getRelativeTime() / (mCurrentLoad * mCurrentLoad);
		right = mFileLength * 1.0 / mMaxDiskBand;
		if(left <= right) return false;
	}
	return true;
}
bool SpreadServer::IsOverLoad()
{
	// old
	return (mCurrentLoad - m_copy_flow) >= mMaxLoad;

	// 新的条件 add by mjq @2014.04.30
	// if(mCurrentLoad + m_copy_flow + m_in_flow >= mMaxLoad)
	// 	return true;
}



int SpreadServer::IncreaseLoad(int fileid)
{
	++mCurrentLoad;
	// assert(mCurrentLoad <= mMaxLoad);
	vector<FileNode>::iterator iter;
	for (iter = m_filelist.begin(); iter != m_filelist.end(); ++iter) {
		if (iter->fileid == fileid) break;
	}
	assert(iter != m_filelist.end());
	++iter->load;
	m_spread_strategy->ReadFile(*iter, getRelativeTime());
	return mCurrentLoad;
}
int SpreadServer::DecreaseLoad(int fileid)
{
	// assert(mCurrentLoad <= mMaxLoad && mCurrentLoad > 0);
	--mCurrentLoad;
	vector<FileNode>::iterator iter;
	for (iter = m_filelist.begin(); iter != m_filelist.end(); ++iter) {
		if (iter->fileid == fileid) break;
	}
	assert(iter != m_filelist.end());
	return mCurrentLoad;
}
int SpreadServer::GetCurrentLoad()
{
	return mCurrentLoad;
}



int SpreadServer::IncreaseCopyFlow(int fileid)
{
	LOG_INFO("before increase: m_copy_flow: " << m_copy_flow << " m_max_copy_flow: "<< m_max_copy_flow); 
	//assert(m_copy_flow <= m_max_copy_flow);
	++m_copy_flow;
	++mCurrentLoad;
	vector<FileNode>::iterator iter;
	for (iter = m_filelist.begin(); iter != m_filelist.end(); ++iter) {
		if (iter->fileid == fileid) break;
	}
	assert(iter != m_filelist.end());
	iter->status = SPREADING;
	return m_copy_flow;
}
int SpreadServer::DecreaseCopyFlow(int fileid)
{
	LOG_INFO("before decrease: m_copy_flow: " << m_copy_flow << " m_max_copy_flow: "<< m_max_copy_flow); 
	//assert(m_copy_flow <= m_max_copy_flow && m_copy_flow > 0);
	--m_copy_flow;
	--mCurrentLoad;
	vector<FileNode>::iterator iter;
	for (iter = m_filelist.begin(); iter != m_filelist.end(); ++iter) {
		if (iter->fileid == fileid) break;
	}
	assert(iter != m_filelist.end());
	// assert(iter->status == SPREADING);
	iter->status = SPREADED;
	return m_copy_flow;
}
int SpreadServer::GetCopyFlow()
{
	return m_copy_flow;
}
int SpreadServer::IncreaseInFlow(int fileid)
{
	++mCurrentLoad;
	++m_in_flow;
	vector<FileNode>::iterator iter;
	for (iter = m_filelist.begin(); iter != m_filelist.end(); ++iter) {
		if (iter->fileid == fileid) break;
	}
	assert(iter != m_filelist.end());
	iter->status = SPREADING;
	// assert(mCurrentLoad <= mMaxLoad);
	return mCurrentLoad;
}
int SpreadServer::DecreaseInFlow(int fileid)
{
	--mCurrentLoad;
	--m_in_flow;
	vector<FileNode>::iterator iter;
	for (iter = m_filelist.begin(); iter != m_filelist.end(); ++iter) {
		if (iter->fileid == fileid) break;
	}
	assert(iter != m_filelist.end());
	// assert(iter->status == SPREADING);
	iter->status = NORMAL;
	assert(mCurrentLoad >= 0);
	return mCurrentLoad;
}


#include "spreadstrategy.h"
#include "log.h"
#include <algorithm>

void DWSpreadStrategy::ReadFile(FileNode &file, double vtime)
{
	++file.hitnew;
	// file.vtime = vtime;
}

void DWSpreadStrategy::Reset(vector<FileNode> &filelist)
{
	vector<FileNode>::iterator iter;
	for (iter = filelist.begin(); iter != filelist.end(); ++iter) {
		iter->hitold = iter->hitnew;
		iter->hitnew = 0;
	}
}

bool DWCompare(FileNode a, FileNode b)
{
	if (a.weight > b.weight) return true;
	else if (a.weight == b.weight && a.fileid < b.fileid) return true;
	else return false;
}
int DWSpreadStrategy::GetSpreadFile(vector<FileNode> &filelist, vector<int> &output)
{
	vector<FileNode>::iterator iter;
	for (iter = filelist.begin(); iter != filelist.end(); ++iter) {
		iter->weight = iter->hitnew + iter->hitold;
	}
	sort(filelist.begin(), filelist.end(), DWCompare);
	LOG_INFO_NOENDL("GetSpreadFile(dw), fileid[weight]: ");
	for (iter = filelist.begin(); iter != filelist.end(); ++iter) {
		if (iter->status == NORMAL){
			output.push_back(iter->fileid);
			LOG_NOTIME_NOENDL(iter->fileid << "[" << iter->weight << "] ");
		}
	}
	LOG_NOTIME_NOENDL(endl);
	return 0;
}
// 返回 没有客户端访问 且 不是正在扩散 的文件
// 按照 weight 从小到大排序
int DWSpreadStrategy::GetDeleteFile(vector<FileNode> &filelist, vector<int> &output)
{
	// 文件按照最后 weight 排序，大的在前，后面需倒序遍历添加
	vector<FileNode>::iterator iter;
	for (iter = filelist.begin(); iter != filelist.end(); ++iter) {
		iter->weight = iter->hitnew + iter->hitold;
	}
	sort(filelist.begin(), filelist.end(), DWCompare);
	LOG_INFO_NOENDL("GetDeleteFile(dw), fileid[weight]: ");
	for(int i = filelist.size() - 1; i >= 0; --i)
	{
		if(filelist[i].load == 0 && filelist[i].status != SPREADING )
		{
			output.push_back(filelist[i].fileid);
			LOG_NOTIME_NOENDL(filelist[i].fileid << "[" << filelist[i].weight << "] ");
		}
	}
	LOG_NOTIME_NOENDL(endl);
	return 0;
}


void LRUSpreadStrategy::ReadFile(FileNode &file, double vtime) 
{
	file.vtime = vtime;
}

bool LRUCompare(FileNode a, FileNode b)
{
	if (a.vtime > b.vtime) return true;
	else if (a.vtime == b.vtime && a.fileid < b.fileid) return true;
	else return false;
}
int LRUSpreadStrategy::GetSpreadFile(vector<FileNode> &filelist, vector<int> &output) 
{
	vector<FileNode>::iterator iter;
	sort(filelist.begin(), filelist.end(), LRUCompare);
	LOG_INFO_NOENDL("GetSpreadFile(lru), fileid[vtime]: ");
	for (iter = filelist.begin(); iter != filelist.end(); ++iter) {
		if (iter->status == NORMAL){
			output.push_back(iter->fileid);
			LOG_NOTIME_NOENDL(iter->fileid << "[" << iter->vtime << "] ");
		}
	}
	LOG_NOTIME_NOENDL(endl);
	return 0;
}
// 返回 没有客户端访问 且 不是正在扩散 的文件
// 按照 vtime 从小到大排序
int LRUSpreadStrategy::GetDeleteFile(vector<FileNode> &filelist, vector<int> &output)
{
	// 文件按照最后 vtime 排序，大的在前，后面需倒序遍历添加
	sort(filelist.begin(), filelist.end(), LRUCompare);
	LOG_INFO_NOENDL("GetDeleteFile(lru), fileid[weight]: ");
	for(int i = filelist.size() - 1; i >= 0; --i)
	{
		if(filelist[i].load == 0 && filelist[i].status != SPREADING )
		{
			output.push_back(filelist[i].fileid);
			LOG_NOTIME_NOENDL(filelist[i].fileid << "[" << filelist[i].weight << "] ");
		}
	}
	LOG_NOTIME_NOENDL(endl);
	return 0;
}


void FIFOSpreadStrategy::ReadFile(FileNode &file, double vtime) 
{
	if(file.vtime == 0.0){
		file.vtime = vtime;	
	}
}
bool FIFOCompare(FileNode a, FileNode b)
{
	if (a.vtime > b.vtime) return true;
	else if (a.vtime == b.vtime && a.fileid < b.fileid) return true;
	else return false;
}
int FIFOSpreadStrategy::GetSpreadFile(vector<FileNode> &filelist, vector<int> &output) 
{
	vector<FileNode>::iterator iter;
	sort(filelist.begin(), filelist.end(), FIFOCompare);
	LOG_INFO_NOENDL("GetSpreadFile(fifo), fileid[vtime]: ");
	for (iter = filelist.begin(); iter != filelist.end(); ++iter) {
		if (iter->status == NORMAL){
			output.push_back(iter->fileid);
			LOG_NOTIME_NOENDL(iter->fileid << "[" << iter->vtime << "] ");
		}
	}
	LOG_NOTIME_NOENDL(endl);
	return 0;
}
// 返回 没有客户端访问 且 不是正在扩散 的文件
// 按照 vtime 从小到大排序
int FIFOSpreadStrategy::GetDeleteFile(vector<FileNode> &filelist, vector<int> &output)
{
	// 文件按照最后 vtime 排序，大的在前，后面需倒序遍历添加
	sort(filelist.begin(), filelist.end(), FIFOCompare);
	LOG_INFO_NOENDL("GetDeleteFile(fifo), fileid[weight]: ");
	for(int i = filelist.size() - 1; i >= 0; --i)
	{
		if(filelist[i].load == 0 && filelist[i].status != SPREADING )
		{
			output.push_back(filelist[i].fileid);
			LOG_NOTIME_NOENDL(filelist[i].fileid << "[" << filelist[i].weight << "] ");
		}
	}
	LOG_NOTIME_NOENDL(endl);
	return 0;
}


void LFUSpreadStrategy::ReadFile(FileNode &file, double vtime) 
{
	++file.hitnew;
}

bool LFUCompare(FileNode a, FileNode b)
{
	if (a.hitnew > b.hitnew) return true;
	else if (a.hitnew == b.hitnew && a.fileid < b.fileid) return true;
	else return false;
}
int LFUSpreadStrategy::GetSpreadFile(vector<FileNode> &filelist, vector<int> &output) 
{
	vector<FileNode>::iterator iter;
	sort(filelist.begin(), filelist.end(), LFUCompare);
	LOG_INFO_NOENDL("GetSpreadFile(lfu), fileid[hitnew]: ");
	for (iter = filelist.begin(); iter != filelist.end(); ++iter) {
		if (iter->status == NORMAL) {
			output.push_back(iter->fileid);
			LOG_NOTIME_NOENDL(iter->fileid << "[" << iter->hitnew << "] ");
		}
	}
	LOG_NOTIME_NOENDL(endl);
	return 0;
}
// 返回 没有客户端访问 且 不是正在扩散 的文件
// 按照 hitnew 从小到大排序
int LFUSpreadStrategy::GetDeleteFile(vector<FileNode> &filelist, vector<int> &output)
{
	// 文件按照最后 hitnew 排序，大的在前，后面需倒序遍历添加
	sort(filelist.begin(), filelist.end(), LFUCompare);
	LOG_INFO_NOENDL("GetDeleteFile(lfu), fileid[weight]: ");
	for(int i = filelist.size() - 1; i >= 0; --i)
	{
		if(filelist[i].load == 0 && filelist[i].status != SPREADING )
		{
			output.push_back(filelist[i].fileid);
			LOG_NOTIME_NOENDL(filelist[i].fileid << "[" << filelist[i].weight << "] ");
		}
	}
	LOG_NOTIME_NOENDL(endl);
	return 0;
}

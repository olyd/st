#include "diskmgr.h"

#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

#include <cassert>

#include "log.h"
#include "utils.h"

DiskInfo::DiskInfo(int _diskid) 
{
	diskid = _diskid;
	waitingThread = 0;
	readingThread = 0;
	Pthread_mutex_init(&mutex, NULL);
	Pthread_cond_init(&cond, NULL);
}
DiskInfo::~DiskInfo()
{
	Pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);
}


DiskMgr::DiskMgr(ConfigStrip *config)
{
	// 虚拟读取磁盘时的初始化比较简单
	if(!config->isUseRealDevice){
		m_blockSize = config->blockSize;
		m_diskBand = config->diskBand;
		m_diskNumber = config->diskNumber;
		m_curReadThreadNum = config->curReadThreadNum;
		for (int i = 0; i < m_diskNumber; ++i) {
			DiskInfo *diskInfo = new DiskInfo(i);
			m_diskInfo.push_back(*diskInfo);
			// m_diskInfo[i] = 0;
			LOG_INFO("diskid " << m_diskInfo[i].diskid << " wait " << m_diskInfo[i].waitingThread << " read " << m_diskInfo[i].readingThread);
		}
		Pthread_mutex_init(&m_mutex, NULL);
		m_totalRead = 0;
		LOG_INFO("Finish initializing DiskMgr.");
		return;
	}
	m_blockSize = config->blockSize * 1024 * 1024;
	m_fileNumber = config->sourceNums;
	m_diskNumber = config->diskNumber;
	m_fileLength = config->maxLength / config->blockSize;
	m_curReadThreadNum = config->curReadThreadNum;
	for (int i = 0; i < m_diskNumber; ++i) {
		DiskInfo *diskInfo = new DiskInfo(i);
		m_diskInfo.push_back(*diskInfo);
		// m_diskInfo[i] = 0;
	}
	Pthread_mutex_init(&m_mutex, NULL);
	m_totalRead = 0;
	LOG_INFO("Start to initialize DiskMgr.");
	vector<string> segList;
	vector<FileInfoDisk> fileInfoList;
	vector<vector<string> > fileList;
	// m_fileIndex.push_back(segList);
	string prefix = "/disk";
	int perDiskSegNum = m_fileLength * m_fileNumber / m_diskNumber + 1;
	for (int i = 0; i < m_diskNumber; ++i) {
		segList.clear();
		string dirName = prefix + (char)('0' + i);
		GetFileNamesInDir(dirName, segList);
		assert((unsigned int)perDiskSegNum < segList.size());
		fileList.push_back(segList);
	}
	int curIndex = 0;
	for (int i = 0; i < m_fileNumber; ++i) {
		fileInfoList.clear();
		for (int j = 0; j < m_fileLength; ++j) {
			FileInfoDisk temp;
			temp.diskid = curIndex % m_diskNumber;
			temp.filename = fileList[curIndex % m_diskNumber][curIndex / m_diskNumber];
			fileInfoList.push_back(temp);
			// cout << "file: " << i << " seg: " << j << "name: " << temp.filename << endl;
			++curIndex;
		}
		m_fileIndex.push_back(fileInfoList);
	}
	LOG_INFO("Finish initializing DiskMgr.");
	// for (int i = 0; i < m_fileNumber; ++i) {
	// 	for (int j = 0; j < m_fileLength; ++j) {
	// 		cout << m_fileIndex[i][j].filename << " ";
	// 	}
	// 	cout << endl;
	// }
}


DiskMgr::~DiskMgr()
{
	//TODO delete m_diskinfo
	Pthread_mutex_destroy(&m_mutex);
}

void DiskMgr::ReadSeg(int fileid, int segid)
{
	LOG_INFO_NOENDL("real waiting thread distribution: ");
	for (int i = 0; i < m_diskNumber; ++i) {
		LOG_NOTIME_NOENDL(m_diskInfo[i].waitingThread << "\t");
	}
	LOG_NOTIME_NOENDL(endl);
	assert(fileid < m_fileNumber);
	assert(segid < m_fileLength);
	char *buffer = (char *)Malloc(m_blockSize);
	int diskid = m_fileIndex[fileid][segid].diskid;
	Pthread_mutex_lock(&m_diskInfo[diskid].mutex);
	++m_diskInfo[diskid].waitingThread;
	while (m_diskInfo[diskid].readingThread >= m_curReadThreadNum) {
		pthread_cond_wait(&m_diskInfo[diskid].cond, &m_diskInfo[diskid].mutex);
	}
	++m_diskInfo[diskid].readingThread;
	--m_diskInfo[diskid].waitingThread;
	Pthread_mutex_unlock(&m_diskInfo[diskid].mutex);
    LOG_INFO("server read file: " << m_fileIndex[fileid][segid].filename);
	int fd = open(m_fileIndex[fileid][segid].filename.c_str(), O_RDONLY);
	if (fd < 0) { 
		cout << "Error: open file error!: " << m_fileIndex[fileid][segid].filename << endl;
		assert(0);
	}
	if (read(fd, buffer, m_blockSize) != m_blockSize) {
		cout << "Error: read file error!" << endl;
		assert(0);
	}
	close(fd);
    free(buffer);
    //usleep(200000);
	Pthread_mutex_lock(&m_diskInfo[diskid].mutex);
	--m_diskInfo[diskid].readingThread;
	pthread_cond_broadcast(&m_diskInfo[diskid].cond);
	Pthread_mutex_unlock(&m_diskInfo[diskid].mutex);
	Pthread_mutex_lock(&m_mutex);
	++m_totalRead;
	Pthread_mutex_unlock(&m_mutex);	
}

void DiskMgr::ReadSeg(int fileid, int segid, int diskid)
{
	LOG_INFO_NOENDL("fake waiting thread distribution: ");
	for (int i = 0; i < m_diskNumber; ++i) {
		LOG_NOTIME_NOENDL(m_diskInfo[i].waitingThread << "\t");
	}
	LOG_NOTIME_NOENDL(endl);
	Pthread_mutex_lock(&m_diskInfo[diskid].mutex);
	++m_diskInfo[diskid].waitingThread;
	while (m_diskInfo[diskid].readingThread >= m_curReadThreadNum) {
		pthread_cond_wait(&m_diskInfo[diskid].cond, &m_diskInfo[diskid].mutex);
	}
	++m_diskInfo[diskid].readingThread;
	--m_diskInfo[diskid].waitingThread;
	Pthread_mutex_unlock(&m_diskInfo[diskid].mutex);
	
	//usleep(200000);
	double temp = m_blockSize * 1.0 / m_diskBand;
	LOG_INFO("server fake read, sleep " << temp << "s");
	usleep((int)(temp * 1000000));
	
	Pthread_mutex_lock(&m_diskInfo[diskid].mutex);
	--m_diskInfo[diskid].readingThread;
	pthread_cond_broadcast(&m_diskInfo[diskid].cond);
	Pthread_mutex_unlock(&m_diskInfo[diskid].mutex);
	Pthread_mutex_lock(&m_mutex);
	++m_totalRead;
	Pthread_mutex_unlock(&m_mutex);
}


void 
DiskMgr::GetFileNamesInDir(const string strDir, vector<string> &vecFileName)
{
	DIR* dir = NULL;
	struct dirent entry;
	struct dirent* entryPtr = NULL;
	char realPath[1024];
	realpath(strDir.c_str(), realPath);
	string strRealPath = realPath;

	dir = opendir(realPath);
	if (NULL == dir) {
		cout << strerror(errno) << ", strDir : " << strDir << endl;
		return;
	}

	readdir_r(dir, &entry, &entryPtr);
	while (entryPtr != NULL) {
		if (entry.d_type == DT_REG) {
			string strFileName = entry.d_name;
			if ("." == strFileName || ".." == strFileName) {
			}
			else {
				if (GetFileLength(strRealPath + "/" + strFileName) 
					== m_blockSize) {
					vecFileName.push_back(strRealPath + "/" + strFileName);
				}
			}
		}
		else if(entry.d_type == DT_DIR) {
			string dir = entry.d_name;
			if (!("." == dir || ".." == dir)) {
				GetFileNamesInDir(strRealPath + "/" + dir, vecFileName);
			}
		}
		readdir_r(dir, &entry, &entryPtr);
	}

	return;
}

off_t DiskMgr::GetFileLength(const string filename)
{
	struct stat st;
	if (lstat(filename.c_str(), &st) != 0) {
		cout << "Error: lstat error" << endl;
		exit(1);
	}
	return st.st_size;
}


void DiskMgr::PrintDiskInfo()
{
	LOG_INFO_NOENDL("print diskinfo:: ");
	for (int i = 0; i < m_diskNumber; ++i) {
		LOG_NOTIME_NOENDL(m_diskInfo[i].waitingThread << "\t");
	}
	LOG_NOTIME_NOENDL(endl);
}

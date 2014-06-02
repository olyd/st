#include "utils.h"

#include <sys/mman.h>

#include <fstream>

using namespace std;

extern struct timeval startTime;

static void Trim(string &str){
	str.erase(0, str.find_first_not_of(" "));
	str.erase(str.find_last_not_of(" ") + 1);
}

void ParseConfigFile(string config_filename, map<string, string> &keymap) {
	ifstream infile;
	infile.open(config_filename.c_str());

	string line;
	while (getline(infile, line)) {
		if(line.empty())
			continue;
		Trim(line);
		if(line.at(0) == '#')
			continue;
		int equalPos = line.find_first_of('=');
		string key = line.substr(0, equalPos);
		string value = line.substr(equalPos + 1);
		Trim(key);
		Trim(value);
		keymap.insert(make_pair(key, value));
	}
	infile.close();
}

// 注意获取边界的概率也要相同
// 随机获取[a,b]之间的任意浮点数，包括a,b
double Randomf(int a,int b){
	// double temp = random()/(RAND_MAX*1.0);
	// return a + (b-a) * temp;
	return rand() * 1.0 / RAND_MAX * (b - a) + a;
}

// 注意获取边界的概率也要相同
// 随机获取[a,b]之间的任意整数，包括a,b
int Randomi(int a,int b){
	// double  temp = random()/(RAND_MAX * 1.0);
	// turn (int)(a + (b - a ) * temp) ;
	return (rand() % (b-a+1) + a);
}

string numToString(int n){
	char buf[256];
	sprintf(buf,"%d",n);
	string temp(buf);
	return temp;
}

int stringToInt(const string &value){
	int result;
	string value1;
	bool isNegative = false;
	if(value.size()>0 && value[0]=='-'){
		isNegative = true;
		value1 = value.substr(1);
	}
	if(isNegative == true){
		result = atoi(value1.c_str());
		result = 0 - result;
	}else
		result = atoi(value.c_str());
//	cout<<"value = "<<value<<endl;
//	cout<<"result = "<<result<<endl;
	return result;

}

int mysleep(unsigned int usec){
	usleep(usec);
	return 0;
}

// 获取当前的绝对时间
int getCurrentTime(struct timeval *tv){
	struct timeval temp;
	gettimeofday(&temp,NULL);
	tv->tv_sec = temp.tv_sec;
	tv->tv_usec = temp.tv_usec;
	return 0;
}
// 获取相对于程序开始运行的时间，startTime在main函数中初始化
double getRelativeTime(){
	struct timeval curTime;
	gettimeofday(&curTime,NULL);
	return getTimeSlips(&curTime,&startTime);
}
// 获取两个时间之间的差值
double getTimeSlips(struct timeval *a,struct timeval *b){
	double sec;
	sec = a->tv_sec - b->tv_sec;
	sec += (a->tv_usec - b->tv_usec)/1000000.0;
	return sec;
}
// int comp(const FileCount &a ,const FileCount &b){
// 	return a.count > b.count;
// }
// int compS(const ServerLoad &a ,const ServerLoad &b){
// 	return a.load < b.load;
// }
double  minDouble(double a,double b){
	return a > b ? b:a;
}


void LockMemory(int size)
{
	if (size > 0) {
		size_t totalSize = 1024 * 1024 * 1024 * size;
		void *startAddr = Malloc(totalSize);
		int ret = mlock((void *)startAddr, totalSize);
		memset(startAddr, 1, totalSize);

		if (ret) {
			switch(ret) {
			case ENOMEM : // not enough memory
				cout << "Error : not enough memory, " ;
				break;
			case EPERM : // priviledged not enough
				cout << "Error : priviledged not enough, " ;
				break;
			case EAGAIN: // not enough memory
				cout << "Error : not enough memory, " ;
				break;
			default :
				cout << "Error : unkown, " ;
				break;
			}
			cout << strerror(errno) << endl;
			exit(1);
		} 
	}
}
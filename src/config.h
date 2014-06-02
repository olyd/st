#ifndef CONFIG_H
#define CONFIG_H

#include <map>
#include <string>

using namespace std;

// #define MAX_LISTEN_NUM 1000

typedef struct configGlobal{
    string simulation;
} ConfigGlobal;

typedef struct configType {
    unsigned int resourceNumber;
    unsigned int subServerNum;
    unsigned long minPlayLen;
    unsigned long maxPlayLen;
    unsigned int maxCapacity;
    unsigned int minCapacity;
    unsigned int maxLoad;
    unsigned int minLoad;
    unsigned int fileLength;
    unsigned int maxDiskBand;
	unsigned int reservedTime;
    string serverIpAddress;
    string serverPort;
    string spreadAlgorithm;
    unsigned int period;
    double DRBeta;
    double lambda;	
    double loadThresh;
    double loadThreshHigh;
    string logFile;
    unsigned int poissonLambda;
    unsigned int clientNumber;
    unsigned int zipfParameter;
    string requestListFile;
    string resultFile;
    bool haveCost ;
    int maxCopyFlow;
    int maxInFlow;
    int runTime;
} ConfigType;


typedef struct configStrip {
    int serverBand;
    int clientBand;

    int blockSize;
    int perSendSize;

    bool isP2POpen;
    int sourceNums;

    int diskNumber;
    int curReadThreadNum;
    int lockMemory;
    string placeStrategy;
    int diskBand;

    int maxLength;
    int minLength;
    int maxBitRate;
    int minBitRate;

    int serverBlockNum;
    int clientBlockNum;

    int thelta;
    int lambda;
    int backZeta;
    int backSigma;
    int forZeta;
    int forSigma;

    int hotPlaces;

    int playToPlay;
    int playToPause;
    int playToForward;
    int playToBackward;
    int playToStop;

    int clientNums;

    int multiple;
    int sampleFrequency;

    int serverPort;
    int clientPort;

    int devNums;
    int clusterNums;
    string clusterAddress1;
    string serverAddress;

    int modify;
    bool isStartTogether;
    bool isUseRealDevice;

    string serverStrategy;
    string clientStrategy;
    int lrfuLambda;
    int period;
    bool isRepeat;

    int preFetch;
    bool isSpecial;
} ConfigStrip;



int parse_config_line(const string &line, string &key, string &value);
// 读取全局配置文件，即启动分条系统还是资源扩散系统
int read_vodglobal_config(const string filename, ConfigGlobal &configInfo);
// 读取资源扩散系统配置文件
int read_vodspread_config(const string filename, ConfigType &configInfo);
// 读取分条系统配置文件
int read_vodstrip_config(const string filename, ConfigStrip &configInfo);


int read_client_config(const string filename, ConfigType &);
int read_clientcircle_config(const string filename, double &zeta, double &sigma);
int read_config_file(const string fileName, map<string, string>& conf_map);

#endif

#include <sys/time.h>
#include <fstream>
#include <sstream>
#include <cassert>
#include <cstdlib>

#include "config.h"
#include "clientmgr.h"
#include "servermgr.h"
#include "timer.h"
#include "log.h"

#include "disktest.h"

struct timeval startTime;

pthread_mutex_t log_mutex;


int main(int argc, char *argv[])
{
	srand((unsigned)time(NULL));
	// just to initialize the timer.
	Timer::GetTimer();
	gettimeofday(&startTime, NULL);
	pthread_mutex_init(&log_mutex, NULL);

	bool isStrip = false;//是否为分条存储系统
	cout << "----------read <global> config----------" << endl;
	ConfigGlobal configGlobal;
	read_vodglobal_config("config/global.ini", configGlobal);
	if(configGlobal.simulation == "strip"){
		isStrip = true;
		cout << "...now start <strip> simulation..." << endl;
	}else if(configGlobal.simulation == "spread"){
		isStrip = false;
		cout << "...now start <spread> simulation..." << endl;
	}else{
		cout << "!!! int config/global.ini, simulation = strip/spread !!!" << endl;
		assert(0);
	}

	ConfigType configSpread;
	ConfigStrip configStrip;

	// 启动扩散系统
	if(!isStrip){
		cout << "----------read <spread> config----------" << endl;
		read_vodspread_config("config/spread.ini", configSpread);
		cout << "----------config over----------" << endl;

		ofstream currentSpreadConfig;
		currentSpreadConfig.open("current_spread_config.log", ios::out);
		stringstream ssSpreadConfig;
		ssSpreadConfig.str();
		// 如 dw_40_32_2_1_400_900
		ssSpreadConfig << configSpread.spreadAlgorithm << "_" << configSpread.period << "_" << configSpread.maxCapacity << "_" << configSpread.maxCopyFlow << "_" << configSpread.maxInFlow << "_" << configSpread.loadThresh * 1000 << "_" << configSpread.loadThreshHigh * 1000;
		currentSpreadConfig << ssSpreadConfig.str();
		currentSpreadConfig.close();		
	}
	// 启动分条系统
	else{
		cout << "----------read <strip> config----------" << endl;
		read_vodstrip_config("config/strip.ini", configStrip);
		cout << "----------config over----------" << endl;

		ofstream currentConfig;
		currentConfig.open("current_strip_config.log", ios::out);
		stringstream ssconfig;
		ssconfig.str();
		// 如 true_on_1000_fifos_20_fifo40_90_10
		if(configStrip.isSpecial){
			ssconfig << "true_";
		}else{
			ssconfig << "false_";
		}
		if(configStrip.isP2POpen){
			ssconfig << "on_";
		}else{
			ssconfig << "off_";
		}
		ssconfig << configStrip.serverBlockNum << "_" << configStrip.serverStrategy << "_" << configStrip.clientBlockNum << "_" << configStrip.clientStrategy << configStrip.period << "_" << configStrip.blockSize; 
		currentConfig << ssconfig.str();
		currentConfig.close();
	}

	// ClientManager *client_manager = new ClientManager(true, &configStrip);
	// client_manager->Run();

	// DiskTest *diskTest = new DiskTest(&configStrip);
	// diskTest->Run();


	int option;
	bool isserver = true;
	while ((option = getopt(argc, argv, "chsp:o:i:a:l:")) != -1) {
		switch (option) {
		case 'c':
			isserver = false;
			break;
		case 's':
			isserver = true;
			break;
		case 'p':
			if(!isStrip){
				configSpread.maxCapacity = atoi(optarg);
				LOG_INFO("main: set maxCapacity = " << configSpread.maxCapacity);
			}
			break;
		case 'o':
			if(!isStrip){
				configSpread.maxCopyFlow = atoi(optarg);
				LOG_INFO("main: set maxCopyFlow = " << configSpread.maxCopyFlow);
			}
			break;
		case 'i':
			if(!isStrip){
				configSpread.maxInFlow = atoi(optarg);
				LOG_INFO("main: set maxInFlow = " << configSpread.maxInFlow);
			}
			break;
		case 'a':
			if(!isStrip){
				configSpread.spreadAlgorithm = optarg;
				LOG_INFO("main: set spreadAlgorithm = " << configSpread.spreadAlgorithm);
			}
			break;
		case 'l':
			if(!isStrip){
				configSpread.loadThresh = atoi(optarg) / 1000.0;
				LOG_INFO("main: set loadThresh = " << configSpread.loadThresh);
			}
			break;
		case 'h':	
			cout << "Usage: st [option]" << endl;
			cout << "-c, run as client" << endl;
			cout << "-s, run as server(default)" << endl;
			cout << "-h, show this message" << endl;
			if(!isStrip){
				cout << "-p, set maxCapacity" << endl;
				cout << "-o, set maxCopyFlow" << endl;
				cout << "-i, set maxInFlow" << endl;
				cout << "-a, set spreadAlgorithm" << endl;
				cout << "-l, set loadThresh(0-1000)" << endl;
			}
			return 0;
		default:
			cout << "Usage: st [option]" << endl;
			cout << "-c, run as client" << endl;
			cout << "-s, run as server(default)" << endl;
			cout << "-h, show this message" << endl;
			if(!isStrip){
				cout << "-p, set maxCapacity" << endl;
				cout << "-o, set maxCopyFlow" << endl;
				cout << "-i, set maxInFlow" << endl;
				cout << "-a, set spreadAlgorithm" << endl;
				cout << "-l, set loadThresh(0-1000)" << endl;
			}
			return 0;
		}
	}


	if (isserver) {
		ServerManager *server_manager;
		if(isStrip){
			server_manager = new ServerManager(true, &configStrip);
		}else{
			server_manager = new ServerManager(false, &configSpread);
		}
		server_manager->Run();
	}
	else {
		ClientManager *client_manager;
		if(isStrip){
			client_manager = new ClientManager(true, &configStrip);
		}else{
			client_manager = new ClientManager(false, &configSpread);
		}
		client_manager->Run();
	}

	return 0;
}

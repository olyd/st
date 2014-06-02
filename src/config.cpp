#include "config.h"

#include <iostream>
#include <cstdlib>
#include <fstream>

#include <string>
#include <cctype>
#include <algorithm>

#include "error.h"

int parse_config_line(const string &line, string &key, string& value) {
    size_t pos = 0;
    string lineBuf = line;
    while ((pos = lineBuf.find(' ')) != string::npos) {
        lineBuf.erase(pos, 1);
    }
    while ((pos = lineBuf.find('\t')) != string::npos) {
        lineBuf.erase(pos, 1);
    }

    if (lineBuf[0] == '#')
        return -1;
    pos = lineBuf.find('#');
    if (pos != string::npos) {
        lineBuf = lineBuf.substr(0, pos);
    }
    pos = lineBuf.find('=');
    if (pos != string::npos) {
        key = lineBuf.substr(0, pos);
        value = lineBuf.substr(pos + 1);
    }
    return 0;

}

int read_vodglobal_config(const string fileName, ConfigGlobal &configInfo){
    ifstream confFile(fileName.c_str());
    if (!confFile.is_open()) {
        cout << "\tFatal Error : Cannot open configure file "
                << fileName << endl;
        exit(-1);
    }

    string line;
    cout << "read config file " << fileName << endl;
    while (confFile.good()) {
        getline(confFile, line);
        string key, value;
        parse_config_line(line, key, value);
        if (key.size() == 0 || value.size() == 0)
            continue;
        //cout << "set " << key << " = " << value << endl;
        if (key.compare("simulation") == 0) {
            configInfo.simulation = value;
            cout << "set " << key << " = " << configInfo.simulation << endl;
        }else {
            cout << "can't find  arg :" << key << endl;
        }

    }
    confFile.close();
    return 0;
}

int read_vodspread_config(const string fileName, ConfigType &configInfo) {
    ifstream confFile(fileName.c_str());
    if (!confFile.is_open()) {
        cout << "\tFatal Error : Cannot open configure file "
                << fileName << endl;
        exit(-1);
    }

    string line;
    cout << "read config file " << fileName << endl;
    while (confFile.good()) {
        getline(confFile, line);
        string key, value;
        parse_config_line(line, key, value);
        if (key.size() == 0 || value.size() == 0)
            continue;
        //cout << "set " << key << " = " << value << endl;
        if (key.compare("resourceNumber") == 0) {
            configInfo.resourceNumber = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.resourceNumber << endl;
        } else if (key.compare("subServerNum") == 0) {
            configInfo.subServerNum = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.subServerNum << endl;
        } else if (key.compare("maxPlayLen") == 0) {
            configInfo.maxPlayLen = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.maxPlayLen << endl;
        } else if (key.compare("minPlayLen") == 0) {
            configInfo.minPlayLen = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.minPlayLen << endl;
        } else if (key.compare("minLoad") == 0) {
            configInfo.minLoad = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.minLoad << endl;
        } else if (key.compare("period") == 0) {
            configInfo.period = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.period << endl;
        } else if (key.compare("DRBeta") == 0) {
            configInfo.DRBeta = atoi(value.c_str()) / 1000.0;
            cout << "set " << key << " = " << configInfo.DRBeta << endl;
        } else if (key.compare("lambda") == 0) {// set the lambda of LRFU
            configInfo.lambda = atoi(value.c_str()) / 1000.0;
            cout << "set " << key << " = " << configInfo.lambda << endl;
        } else if (key.compare("loadThresh") == 0) {
            configInfo.loadThresh = atoi(value.c_str()) / 1000.0;
            cout << "set " << key << " = " << configInfo.loadThresh << endl;
        } else if (key.compare("loadThreshHigh") == 0) {
            configInfo.loadThreshHigh = atoi(value.c_str()) / 1000.0;
            cout << "set " << key << " = " << configInfo.loadThreshHigh << endl;
        } else if (key.compare("minCapacity") == 0) {
            configInfo.minCapacity = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.minCapacity << endl;
        } else if (key.compare("maxDiskBand") == 0) {
            configInfo.maxDiskBand = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.maxDiskBand << endl;
        } else if (key.compare("fileLength") == 0) {
            configInfo.fileLength = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.fileLength << endl;
        } else if (key.compare("maxCapacity") == 0) {
            configInfo.maxCapacity = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.maxCapacity << endl;
        } else if (key.compare("minLoad") == 0) {
            configInfo.minLoad = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.minLoad << endl;
        } else if (key.compare("maxLoad") == 0) {
            configInfo.maxLoad = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.maxLoad << endl;
        } else if (key.compare("reservedTime") == 0) {
            configInfo.reservedTime = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.reservedTime << endl;
        } else if (key.compare("serverPort") == 0) {
            configInfo.serverPort = value;
            cout << "set " << key << " = " << configInfo.serverPort << endl;
        } else if (key.compare("spreadAlgorithm") == 0) {
            transform(value.begin(), value.end(), value.begin(), ::tolower);
            configInfo.spreadAlgorithm = value;
            cout << "set " << key << " = " << configInfo.spreadAlgorithm << endl;
        } else if (key.compare("logFile") == 0) {
            configInfo.logFile = value;
            cout << "set " << key << " = " << configInfo.logFile << endl;
        } else if (key.compare("resultFile") == 0) {
            configInfo.resultFile = value;
            cout << "set " << key << " = " << configInfo.resultFile << endl;
        }else if (key.compare("poissonLambda") == 0) {
            configInfo.poissonLambda = atoi(value.c_str()) ;
            cout << "set " << key << " = " << configInfo.poissonLambda << endl;
        } else if (key.compare("zipfParameter") == 0) {
            configInfo.zipfParameter = atoi(value.c_str()) ;
            cout << "set " << key << " = " << configInfo.zipfParameter << endl;
        } else if (key.compare("resourceNumber") == 0) {
            configInfo.resourceNumber = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.resourceNumber << endl;
        } else if (key.compare("clientNumber") == 0) {
            configInfo.clientNumber = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.clientNumber << endl;
        } else if (key.compare("haveCost") == 0) {
            configInfo.haveCost = value.compare("true")==0? true : false;
            cout << "set " << key << " = " << configInfo.haveCost << endl;
        } else if (key.compare("serverIpAddress") == 0) {
            configInfo.serverIpAddress = value;
            cout << "set " << key << " = " << configInfo.serverIpAddress << endl;
        } else if (key.compare("requestListFile") == 0) {
            configInfo.requestListFile = value;
            cout << "set " << key << " = " << configInfo.requestListFile << endl;
        } else if (key.compare("maxCopyFlow") == 0) {
            configInfo.maxCopyFlow = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.maxCopyFlow << endl;
        } else if (key.compare("maxInFlow") == 0) {
            configInfo.maxInFlow = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.maxInFlow << endl;
        } else if (key.compare("runTime") == 0) {
            configInfo.runTime = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.runTime << endl;
        } else {
            cout << "can't find  arg :" << key << endl;
        }

    }
    confFile.close();
    return 0;
}

int read_vodstrip_config(const string fileName, ConfigStrip &configInfo) {
    ifstream confFile(fileName.c_str());
    if (!confFile.is_open()) {
        cout << "\tFatal Error : Cannot open configure file "
                << fileName << endl;
        exit(-1);
    }

    string line;
    cout << "read config file " << fileName << endl;
    while (confFile.good()) {
        getline(confFile, line);
        string key, value;
        parse_config_line(line, key, value);
        if (key.size() == 0 || value.size() == 0)
            continue;
        // cout << "set " << key << " = " << value << endl;
        if (key.compare("serverBand") == 0) {
            configInfo.serverBand = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.serverBand << endl;
        } else if (key.compare("clientBand") == 0) {
            configInfo.clientBand = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.clientBand << endl;
        } else if (key.compare("blockSize") == 0) {
            configInfo.blockSize = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.blockSize << endl;
        } else if (key.compare("perSendSize") == 0) {
            configInfo.perSendSize = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.perSendSize << endl;
        } else if (key.compare("isP2POpen") == 0) {
            configInfo.isP2POpen = value.compare("true")==0? true : false;
            cout << "set " << key << " = " << configInfo.isP2POpen << endl;
        } else if (key.compare("sourceNums") == 0) {
            configInfo.sourceNums = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.sourceNums << endl;
        } else if (key.compare("maxLength") == 0) {
            configInfo.maxLength = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.maxLength << endl;
        } else if (key.compare("minLength") == 0) {
            configInfo.minLength = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.minLength << endl;
        } else if (key.compare("maxBitRate") == 0) {
            configInfo.maxBitRate = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.maxBitRate << endl;
        } else if (key.compare("minBitRate") == 0) {
            configInfo.minBitRate = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.minBitRate << endl;
        } else if (key.compare("serverBlockNum") == 0) {
            configInfo.serverBlockNum = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.serverBlockNum << endl;
        } else if (key.compare("clientBlockNum") == 0) {
            configInfo.clientBlockNum = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.clientBlockNum << endl;
        } else if (key.compare("thelta") == 0) {
            configInfo.thelta = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.thelta << endl;
        } else if (key.compare("lambda") == 0) {
            configInfo.lambda = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.lambda << endl;
        } else if (key.compare("backZeta") == 0) {
            configInfo.backZeta = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.backZeta << endl;
        } else if (key.compare("backSigma") == 0) {
            configInfo.backSigma = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.backSigma << endl;
        } else if (key.compare("forZeta") == 0) {
            configInfo.forZeta = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.forZeta << endl;
        } else if (key.compare("forSigma") == 0) {
            configInfo.forSigma = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.forSigma << endl;
        } else if (key.compare("hotPlaces") == 0) {
            configInfo.hotPlaces = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.hotPlaces << endl;
        } else if (key.compare("playToPlay") == 0) {
            configInfo.playToPlay = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.playToPlay << endl;
        } else if (key.compare("playToPause") == 0) {
            configInfo.playToPause = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.playToPause << endl;
        } else if (key.compare("playToForward") == 0) {
            configInfo.playToForward = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.playToForward << endl;
        } else if (key.compare("playToBackward") == 0) {
            configInfo.playToBackward = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.playToBackward << endl;
        } else if (key.compare("playToStop") == 0) {
            configInfo.playToStop = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.playToStop << endl;
        } else if (key.compare("clientNums") == 0) {
            configInfo.clientNums = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.clientNums << endl;
        } else if (key.compare("multiple") == 0) {
            configInfo.multiple = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.multiple << endl;
        } else if (key.compare("sampleFrequency") == 0) {
            configInfo.sampleFrequency = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.sampleFrequency << endl;
        } else if (key.compare("serverPort") == 0) {
            configInfo.serverPort = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.serverPort << endl;
        } else if (key.compare("clientPort") == 0) {
            configInfo.clientPort = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.clientPort << endl;
        } else if (key.compare("devNums") == 0) {
            configInfo.devNums = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.devNums << endl;
        } else if (key.compare("clusterNums") == 0) {
            configInfo.clusterNums = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.clusterNums << endl;
        } else if (key.compare("clusterAddress1") == 0) {
            configInfo.clusterAddress1 = value;
            cout << "set " << key << " = " << configInfo.clusterAddress1 << endl;
        } else if (key.compare("serverAddress") == 0) {
            configInfo.serverAddress = value;
            cout << "set " << key << " = " << configInfo.serverAddress << endl;
        } else if (key.compare("modify") == 0) {
            configInfo.modify = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.modify << endl;
        } else if (key.compare("isStartTogether") == 0) {
            configInfo.isStartTogether = value.compare("true")==0?true:false;
            cout << "set " << key << " = " << configInfo.isStartTogether << endl;
        } else if (key.compare("isUseRealDevice") == 0) {
            configInfo.isUseRealDevice = value.compare("true")==0?true:false;
            cout << "set " << key << " = " << configInfo.isUseRealDevice << endl;
        } else if (key.compare("serverStrategy") == 0) {
            transform(value.begin(), value.end(), value.begin(), ::tolower);
            configInfo.serverStrategy = value;
            cout << "set " << key << " = " << configInfo.serverStrategy << endl;
        } else if (key.compare("clientStrategy") == 0) {
            transform(value.begin(), value.end(), value.begin(), ::tolower);
            configInfo.clientStrategy = value;
            cout << "set " << key << " = " << configInfo.clientStrategy << endl;
        } else if (key.compare("lrfuLambda") == 0) {
            configInfo.lrfuLambda = atoi(value.c_str());
        } else if (key.compare("period") == 0) {
            configInfo.period = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.period << endl;
        } else if (key.compare("isRepeat") == 0) {
            configInfo.isRepeat = value.compare("true")==0?true:false;
            cout << "set " << key << " = " << configInfo.isRepeat << endl;
        } else if (key.compare("preFetch") == 0) {
            configInfo.preFetch = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.preFetch << endl;
        } else if (key.compare("isSpecial") == 0) {
            configInfo.isSpecial = value.compare("true")==0?true:false;
            cout << "set " << key << " = " << configInfo.isSpecial << endl;
        } else if (key.compare("diskNumber") == 0) {
            configInfo.diskNumber = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.diskNumber << endl;
        } else if (key.compare("placeStrategy") == 0) {
            configInfo.placeStrategy = value;
            cout << "set " << key << " = " << configInfo.placeStrategy << endl;
        }else if (key.compare("lockMemory") == 0) {
            configInfo.lockMemory = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.lockMemory << endl;
        } else if (key.compare("curReadThreadNum") == 0) {
            configInfo.curReadThreadNum = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.curReadThreadNum << endl;
        } else if (key.compare("diskBand") == 0) {
            configInfo.diskBand = atoi(value.c_str());
            cout << "set " << key << " = " << configInfo.diskBand << endl;
        }else {
            cout << "can't find  arg :" << key << endl;
        }
    }
    confFile.close();
    return 0;
}

int read_client_config(const string fileName, ConfigType &clientConfig) {
    ifstream confFile(fileName.c_str());

    if (!confFile.is_open()) {
        cout << "\tFatal Error : Cannot open configure file "
                << fileName << endl;
        exit(-1);
    }

    string line;
    cout << "read configure file " << fileName << endl;
    while (confFile.good()) {
        getline(confFile, line);
        string key, value;
        parse_config_line(line, key, value);
        if (key.size() == 0 || value.size() == 0)
            continue;
        cout << "set " << key << " = " << value << endl;
        if (key.compare("poissonLambda") == 0) {
            clientConfig.poissonLambda = atoi(value.c_str()) ;
        } else if (key.compare("zipfParameter") == 0) {
            clientConfig.zipfParameter = atoi(value.c_str()) ;
        } else if (key.compare("resourceNumber") == 0) {
            clientConfig.resourceNumber = atoi(value.c_str());
        } else if (key.compare("clientNumber") == 0) {
            clientConfig.clientNumber = atoi(value.c_str());
        } else if (key.compare("serverIpAddress") == 0) {
            clientConfig.serverIpAddress = value;
        } else if (key.compare("serverPort") == 0) {
            clientConfig.serverPort = value;
        } else if (key.compare("logFile") == 0) {
            clientConfig.logFile = value;
        } else if (key.compare("requestListFile") == 0) {
            clientConfig.requestListFile = value;
        } else {
            cout << "can't find this arg :" << key << endl;
        }

    }
    confFile.close();
    return 0;
}

int read_clientcircle_config(const string filename, double &zeta, double &sigma) {
    return 0;
}

static string& trim(string& s) {
    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}

//read a  file

int read_config_file(const string filename, map<string, string>& conf_map) {
    ifstream infile;
    infile.open(filename.c_str());
    if (!infile) {
        err_msg("Can not open file %s", filename.c_str());
        return -1;
    }

    conf_map.clear();

    for (string line, key, value; getline(infile, line);) {
        int pos; //position of "="
        trim(line);
        if (line.empty()) continue;
        if (line[0] == '#') continue;
        pos = line.find_first_of('=');
        key = line.substr(0, pos);
        value = line.substr(pos + 1, line.length() - pos - 1);
        trim(key);
        trim(value);
        conf_map[key] = value;
    }
    return 0;
}


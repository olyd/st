#include<iostream>
#include<sstream>
#include<stdlib.h>
#include<assert.h>
#include<fstream>
#include<vector>
int main(int argc,char* argv[]){
	int clientNum = atoi(argv[1]);
	std::fstream ifs;
	ifs.open("timeCount.log");
	assert(ifs.is_open());
	std::vector<int> count(101);
	for(int i=0;i<101;i++){
		count[i]=0;	
	}	
	while(!ifs.eof()){
		int num;
		for(int i=0;i<100;i++){
			ifs >> num;
			if(ifs.fail())
				break;
			count[i] += num;
		}
	}
	for(int i=0;i<100;i++){
		std::cout<<count[i]<<std::endl;
	}
	return 0;
}

#include<iostream>
#include<sstream>
#include<stdlib.h>
#include<assert.h>
#include<fstream>
#include<vector>
int main(int argc,char* argv[]){
	int clientNum = atoi(argv[1]);
	std::fstream ifs;
	ifs.open("hitCount.log");
	assert(ifs.is_open());
	std::vector<int> count(100);
	for(int i=0;i<100;i++){
		count[i]=0;	
	}	
	while(!ifs.eof()){
		int num;
		for(int i=1;i<100;i++){
			ifs >> num;
			if(ifs.fail())
				break;
			count[i] += num;
			if(num ==0)
				break;
		}
	}
	for(int i=1;i<100;i++){
		if(count[i]==0)
			break;
		else{
			std::cout<<count[i]<<std::endl;
		}
	}
	return 0;
}

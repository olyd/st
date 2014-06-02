#include<iostream>
#include<fstream>
#include<sstream>
using namespace std;
int main(){
    ostringstream fileStr;
    for(int i=1;i<=150;i++){
        fileStr.str("");
        fileStr<<"requestFile"<<i<<".log";
        ifstream ifs(fileStr.str().c_str());
        if(!ifs){
            cout<<"can't open file "<<fileStr.str()<<endl;
            return -1;
        }
        int pre,now;
        pre = 0;
        int line = 0;
        while(!ifs.eof()){
            ifs >>now;
            if(ifs.fail())
                break;
            line ++;
            if(line == 4 && now !=1)
                cout<<fileStr.str()<<":  error in line "<<line<<endl;
            if(now != pre+1 ){
                if(line >4){
                     cout<<fileStr.str()<<":  error in line "<<line<<endl;
                }
            }else{
                    pre = now;
            }
        }
    }
    return 0;
}

#ifndef MARKV_H_
#define MARKV_H_

#include <vector>

using namespace std;

struct MMBlock{
	int status;
	int prob;
};

class Markv {
public:
	Markv(int pToPlay, int pToPause, int pToForward, int pToBackward, int pToStop);
	~Markv();
	int GetNextStatus(int curStatus);
private:
	vector<MMBlock> mMMVect;
};

#endif
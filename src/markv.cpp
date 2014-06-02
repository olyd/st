#include "markv.h"
#include "message.h"
#include "utils.h"
#include <cassert>

Markv::Markv(int pToPlay, int pToPause, int pToForward, int pToBackward, int pToStop)
{
	MMBlock tmpBlock;
	tmpBlock.status = PLAY;
	tmpBlock.prob = pToPlay;
	mMMVect.push_back(tmpBlock);

	tmpBlock.status = PAUSE;
	tmpBlock.prob = pToPause;
	mMMVect.push_back(tmpBlock);

	tmpBlock.status = FORWARD;
	tmpBlock.prob = pToForward;
	mMMVect.push_back(tmpBlock);

	tmpBlock.status = BACKWARD;
	tmpBlock.prob = pToBackward;
	mMMVect.push_back(tmpBlock);

	tmpBlock.status = STOP;
	tmpBlock.prob = pToStop;
	mMMVect.push_back(tmpBlock);
}


Markv::~Markv()
{
	mMMVect.clear();
}


int Markv::GetNextStatus(int curStatus)
{
	// 为啥弄个 | 或？直接 || 不就好了
	if((curStatus == PAUSE) | (curStatus == FORWARD) | (curStatus == BACKWARD))
		return PLAY;
	else{
		int temp = Randomi(1,1000);
		int sum = 0;
		for(unsigned int i = 0;i < mMMVect.size();i++){
			sum += mMMVect[i].prob;
			if(sum >= temp){
				return mMMVect[i].status;
			}
		}
	}
	cout << " markv play status parameter error..." << endl;
	assert(0);
	return PLAY;
}

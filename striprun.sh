#!/bin/bash

#you have to be root
if [ $UID != 0 ];then
	echo "Permisstion denied: You have to run this script as root..."
	exit
fi
#modify max open file number
ulimit -n  65535

serverlog="server_strip.log"
clientlog="client_strip.log"

STARTNUM=1
ENDNUM=3
#echo $0 $1 $2
if [ $# -eq 1 ];then
	STARTNUM=$1
	ENDNUM=$1
elif [ $# -eq 2 ];then
	if [ $1 -gt $2 ];then
		echo "Usage: $0 [startnum] [endnum], [startnum] <= [endnum]."
		exit
	fi
	STARTNUM=$1
	ENDNUM=$2
elif [ $# -gt 2 ];then
	echo "Usage: $0 [startnum] [endnum], [startnum] <= [endnum]."
	exit
else
	STARTNUM=1
	ENDNUM=3
fi
#echo $STARTNUM
#echo $ENDNUM
#exit

for((i=$STARTNUM;i<=$ENDNUM;i++))
do
	#prepare data
	rm -rf data/requestFile*.log
	rm -rf data/client*.log
	cp -rf test_$i/requestFile*.log data/

	#start to run
	./st -s > $serverlog & 
	sleep 10s
	./st -c > $clientlog &

	#end and kill
	sleep 10000s
#	while true
#	do
#		sed -n '$,$p' $serverlog > temp
#		nowtimes=`sed 's/At:\([0-9.]*\).*/\1/g' temp`
#		echo $nowtimes
#		if [ $(echo "${nowtimes} > 5001" | bc)x = 1x ];then
#			break
#		fi
#		sleep 1s
#	done
	#echo "kill st..."
	killall st
	exit
	#collect data
	mv -f $serverlog test_$i/
	mv -f $clientlog test_$i/
	mv -f data/buffer.log test_$i/
	mv -f data/balance_degree.log test_$i/
	mv -f data/client*.log test_$i/
done

#clear data rubbish
rm -rf data/requestFile*.log
rm -rf temp

#auto commit data to svn
./collect_strip.sh $STARTNUM $ENDNUM svn
echo "test is over,from test_$STARTNUM to test_$ENDNUM..."

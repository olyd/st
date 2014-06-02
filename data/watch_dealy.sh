#!/bin/bash



while true
do
	delaynum=`./delay_tongji.sh $1 |wc -l`
	echo $delaynum" client delay..."
	#echo `sed -n '$,$p' ../client_strip.log`
	#sleep 1s
	if [ $delaynum -lt 1 ];then
		#echo `sed -n '$,$p' ../client_strip.log`
		sleep 1s
	else
		echo `sed -n '$,$p' ../client_strip.log`
		sleep 1s
		#exit
	fi
done

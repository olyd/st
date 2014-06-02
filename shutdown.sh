#!/bin/bash
if [ $# -eq 1 ];then
	if [ $1 == "off" ];then
		echo "killall shutdown..."
		killall shutdown
		exit
	elif [ $1 -lt 0 ];then
		echo "Usage:$0 time. set shutdown,time >= 0"
		echo "Usage:$0 off.  cancel shutdown..."
		exit
	elif [ $1 -eq 0 ];then
		shutdown -h now
	else
		shutdown -h +$1 
	fi
else
	echo "Usage:$0 time. set shutdown,time >= 0"
	echo "Usage:$0 off.  cancel shutdown..."
	exit
fi


#!/bin/bash

#you have to be root
if [ $UID != 0 ];then
	echo "Permisstion denied: You have to run this script as root..."
	exit
fi
#modify max open file number
ulimit -n  65535

serverlog="server_spread.log"
clientlog="client_spread.log"
rstlog=data/rst

./st -s > $serverlog &
sleep 5s
./st -c > $clientlog &

sleep 5410s

# auto commit data to svn
./collect_spread.sh svn
echo "spread test is over..."

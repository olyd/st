#!/bin/bash

algo=DW
maxout=2
maxin=1
thresh=400
capacity=16

suffix=${algo}_${capacity}_${maxout}_${maxin}_${thresh}
serverlog=serverlog_$suffix
echo $serverlog
clientlog=clientlog_$suffix
echo $clientlog


./st -s -a $algo -p $capacity -o $maxout -i $maxin -l $thresh > $serverlog &
sleep 5s
./st -c > $clientlog &


sleep 5410s

rstlog=data/rst
if [ -f "$rstlog" ];then
	svn up data
	echo -n $serverlog >> $rstlog
	echo -ne "\t" >> $rstlog
	tail -1 $serverlog >> $rstlog
	sort $rstlog -o $rstlog 
	svn commit $rstlog -m "add $rstlog"
fi



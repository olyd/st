#!/bin/bash

rm hitCount.log

for i in {1..150};
do
	if [ -f hitCount$i.log ];then
		tail -1 hitCount$i.log >> hitCount.log
	else
		echo "can't read hitCount$i.log"
	fi
done
g++ -g getCountDetail.cpp -o getCountDetail
./getCountDetail 150 

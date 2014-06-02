#!/bin/bash

rm timeCount.log

for i in {1..150};
do
	if [ -f timeCount$i.log ];then
		tail -1 timeCount$i.log >> timeCount.log
	else
		echo "can't read hitCount$i.log"
	fi
done
g++ -g getCountDetail.cpp -o getCountDetail
./getCountDetail 150 

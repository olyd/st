#!/bin/bash

rm before10.log

times=0
for i in {1..150};
do
	if [ -f file$i.log ];then
		times=`expr $times + 1`
		if [ $times == 11 ];then
			exit
		fi
		cat file$i.log >> before10.log
#		echo "file$i.log exist"
	fi	
done

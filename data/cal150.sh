#!/bin/bash

rm before150.log


total=0
hits=0

times=0
for((i=1;i<=$1;i++))
do
	if [ -f file$i.log ];then
		times=`expr $times + 1`
		if [ $times -gt $1 ];then
			break
		fi
		cat file$i.log >> before150.log
	fi
done

while read line
do
	echo $line > temp
	tmpVar=`sed 's/\(.*\) \(.*\)/\1/g' temp`
	total=`expr $total + $tmpVar`
	tmpVar=`sed 's/\(.*\) \(.*\)/\2/g' temp`
	hits=`expr $hits + $tmpVar`	
done < before150.log

#echo -ne $total "\t" $hits "\t"
printf "%-8s %-8s " $total $hits
result=`echo "scale=6;$hits/$total" | bc -l`
#echo -n $result
printf "%-8s " $result

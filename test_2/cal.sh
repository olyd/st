#!/bin/bash

rm before10.log

total=0
hits=0

times=0
for i in {1..150};
do
	if [ -f file$i.log ];then
		times=`expr $times + 1`
		if [ $times == 11 ];then
			break
		fi
		cat file$i.log >> before10.log
	fi
done

while read line
do
	echo $line > temp
	tmpVar=`sed 's/\(.*\) \(.*\)/\1/g' temp`
	total=`expr $total + $tmpVar`
	tmpVar=`sed 's/\(.*\) \(.*\)/\2/g' temp`
	hits=`expr $hits + $tmpVar`	
done < before10.log

echo -ne $total "\t" $hits "\t"
echo "scale=6;`expr $hits/$total`" | bc -l

#!/bin/bash

rm file*.log
CLIENTNUMS=$1

while read line;
do
    echo $line > temp;
    first=`sed 's/\(.*\) \(.*\)/\1/g' temp`
    second=`sed 's/\(.*\) \(.*\)/\2/g' temp`
    sed -n '$,$p' clientHit$second.log >> file$first.log
done < requestList_$CLIENTNUMS.log

for filename in `ls file*.log`;
do
    wc -l $filename > temp
    lines=`sed 's/\(.*\) \(.*\)/\1/g' temp`
    if [ $lines != 1 ];then
        cat $filename >> total.log
    fi
done

#sed -n '$,$p'

#!/bin/bash

CLIENTNUMS=$1
rm requestList_$CLIENTNUMS.log

for((i=1;i<=$CLIENTNUMS;i++))
do
    fileseg=`sed -n '1,1p' requestFile$i.log`;
    echo $fileseg $i >> requestList_$CLIENTNUMS.log
done

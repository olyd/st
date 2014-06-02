#!/bin/bash


echo "in requestFile$1..."
sed -i '5,$d' requestFile$1.log
for((j=5;j<604;j++))
do
    result=`expr $RANDOM % 600`
#    sed -i "${j}c\
#       ${result}" requestFile$1.log
    echo $result >> requestFile$1.log
done

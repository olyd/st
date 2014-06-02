#!/bin/bash

if [ $# != 1 ];then
    echo "Usage: $0 [clientnum]"
    exit
fi


for((i=1;i<=20;i++))
do
    for((j=1;j<=8;j++))
    do
        num=` echo "( $i - 1 ) * 8 + $j" |bc `
        echo $num
        ./gen_one.sh $num &
    done
    wait
done

echo "over"

#!/bin/bash

file="startTimeList"

> $file

for((i=1;i<151;i++))
do
    echo -n $i" " >> $file
    sed -n '3p' requestFile$i.log >> $file
done

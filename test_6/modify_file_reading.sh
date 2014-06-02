#!/bin/bash

for((i=1;i<=160;i++))
do
    sed -i "1c\
		${i}" requestFile$i.log
#    rm requestFile$i.log
done

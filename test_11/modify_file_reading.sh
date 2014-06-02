#!/bin/bash

for((i=1;i<=150;i++))
do
	sed -i "1c\
		${i}" requestFile$i.log
done

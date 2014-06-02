#!/bin/bash

for((i=1;i<=300;i++))
do
	sed -i "1c\
		${i}" requestFile$i.log
done

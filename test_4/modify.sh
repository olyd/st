#!/bin/bash


for((i=1;i<=150;++i))
do
	sed "1c$i" requestFile$i.log > temp
	cat temp > requestFile$i.log
done

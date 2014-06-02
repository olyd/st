#!/bin/bash

for((i=1;i<=150;i++))
do
	awk '{if($1>600) print $1,FILENAME}' requestFile$i.log
done

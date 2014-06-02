#!/bin/bash


for((i=2;i<=300;i++))
do
	cp requestFile1.log requestFile$i.log
done


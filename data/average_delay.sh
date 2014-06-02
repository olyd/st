#!/bin/bash

file=$1

awk 'BEGIN{sum=0;count=0}{print $1;sum =sum+$1;count+=1 }END{print sum/count}' $file

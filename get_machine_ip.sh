#!/bin/bash


ifconfig |grep "inet " >temp1
awk '{print $2}' temp1 > temp2
awk -F: '{print $2}' temp2 > temp1
sed -i '/127.0.0.1/d' temp1
sed -i '/10./d' temp1
cat temp1

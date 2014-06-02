#!/bin/bash

filename="rst_spread"
tempfile="temp_spread"
currentConfig="current_spread_config.log"
serverlog="server_spread.log"
clientlog="client_spread.log"

config=`sed -n '1,1p' $currentConfig`
#echo $config

echo "" > $tempfile

echo "collect data started..."
echo -n `./get_machine_ip.sh`" " >> $tempfile
echo -n `date +"%Y%m%d_%H%M%S"`" " >> $tempfile
cat $currentConfig >> $tempfile
#awk -F_ '{printf("%s_%d_%d_%d_%d_%d_%d\t",$1,$2,$3,$4,$5,$6,$7)}' $currentConfig >> $tempfile


sed -n '$,$p' $serverlog > temp
result=`awk '{print $2,$3,$4,$5,$6}' temp`
echo -ne "\t"$result >> $tempfile


svn up $filename
cat $tempfile
cat $tempfile >> $filename
if [ $# -eq 1 ] && [ $1 == "svn" ];then
    svn commit $filename -m "add rst_spread: ${config}" --username mjq --password mjq
fi
echo "collect data finished..."

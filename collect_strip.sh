#!/bin/bash

filename="rst_strip"
tempfile="temp_strip"
currentConfig="current_strip_config.log"

config=`sed -n '1,1p' $currentConfig`
#echo $config

svncommit="nosvn"
if [ $# -eq 3 ];then
	STARTNUM=$1
	ENDNUM=$2
	svncommit=$3
elif [ $# -eq 2 ];then
	STARTNUM=$1
	ENDNUM=$2
elif [ $# -eq 0 ];then
	STARTNUM=1
	ENDNUM=3
else
	echo Usage:$0
	echo Usage:$0 startnum endnum
	echo Usage:$0 startnum endnum svn
	exit
fi

echo "collect data started..."
echo "" > $tempfile
for((i=$STARTNUM;i<=$ENDNUM;i++))
do
	echo -e "\tcollect data in test_$i"
	cd test_$i
	echo -n `../get_machine_ip.sh`" " >> ../$tempfile
	echo -n `date +"%Y%m%d_%H%M%S"`" " >> ../$tempfile
	awk -F_ -v num=$i '{printf("%s_%s_%d_%d_%s_%d_%s_%d\t",$1,$2,num,$3,$4,$5,$6,$7)}' ../$currentConfig >> ../$tempfile
	
    chmod u+x *.sh
    ./clientHit.sh >> ../$tempfile
	#echo -ne "\t"  >> ../$tempfile
	./serverHit.sh >> ../$tempfile
	cd ..
done
svn up $filename
cat $tempfile
cat $tempfile >> $filename
if [ $svncommit == "svn" ];then
    svn commit $filename -m "add rst_strip: ${config}" --username mjq --password mjq
fi
echo "collect data finished..."

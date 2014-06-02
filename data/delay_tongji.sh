#!/bin/bash

if [ $# != 5 ];then
	echo Usage:$0 [number] [delay_time]  [starttime] [ignore_clientnum_start] [ignore_clientnum_end]
	exit
fi

delay_time=$2
start_time=$3
ignore_clientnum_start=$4
ignore_clientnum_end=$5

file=temp_delay
echo "count average max"
while true
do
    > $file
    playtimes=0
    for((i=1;i<=$1;++i))
    do
        if [ $i -ge $ignore_clientnum_start ] && [ $i -le $ignore_clientnum_end ];then
            continue;
        fi
    	awk -v dtime=$delay_time -v stime=$start_time '{if($1 > dtime&& $2>stime && $2<5000) print $1,$2,FILENAME}' clientdelay$i.log >> $file
        each_times=`awk 'BEGIN{count=0;}{count++;}END{print count}' clientdelay$i.log`
#        echo $each_times
        let playtimes+=each_times
    done
    sort -nk 2 $file | tail -7
    
    
    delaytimes=`awk 'BEGIN{count=0;}{count+=1 }END{print count;}' $file`
    awk 'BEGIN{sum=0;count=0;max=0}{if($1>max) max=$1;sum =sum+$1;count+=1 }END{if(count>0) print "delaytimes:",count,"\taverage:",sum/count,"\tmax:",max;else print count,0,max}' $file

    sed -n '$p' buffer.log > temp
    asktimes=`awk '{print $NF}' temp`
    echo -ne "asktimes:"$asktimes"\t"
#    echo -ne "playtimes:"$playtimes"\t"
    echo -ne "delaytimes:"$delaytimes"\t"
#   if [ -n $totaltimes ] && [ $totaltimes -gt 0 ];then
#       echo "...."
        rate=`echo "scale=5;$delaytimes / $asktimes * 2" | bc`
#   fi
    echo -e "blockrate:"$rate"\t"
    sleep 2
done



#!/bin/bash

if [ $# -ne 5 ];then
    echo "Usage: $0 <start_client_num> <end_client_num> <block_size> <bitrate> <stabletime>" 
    exit
fi

startnum=$1
endnum=$2
blocksize=$3
bitrate=$4
stabletime=$5

playtime=`echo "scale=6;${blocksize}*8*1024*1024/${bitrate}/1000/1000" |bc -l`
echo "###########################################"
echo "blocksize:"$blocksize
echo "bitrate:"$bitrate
echo "playtime: "$playtime
echo "###########################################"

sum=0
count=0
max=0
delay=0
totalPlayCount=0
for((i=$1;i<=$2;i++))
do
    echo "now in client $i......"
    sed '/MSG_DELETE_SEG/d' client$i.log > client${i}_temp.log
    sed -i '/MSG_ADD_SEG/d' client${i}_temp.log
    sed -i '/MSG_SEG_ACK/d' client${i}_temp.log
    sed -i '/play fileId:/d' client${i}_temp.log
    sed -i '/read to end/d' client${i}_temp.log
    sed -i 's/At://g' client${i}_temp.log
    awk -v stabletime=$stabletime '{if($1>stabletime) print $0}' client${i}_temp.log > temp;cat temp > client${i}_temp.log
    #exit
    head -1 client${i}_temp.log > temp
    rst=`sed -n '/MSG_SEG_FIN/p' temp `
    #cat "$rst"
    if [ -n "$rst" ];then
        #echo "not empty"
        sed -i '1d' client${i}_temp.log
    fi
    
    tail -1 client${i}_temp.log > temp
    rst=`sed -n '/MSG_SEG_ASK/p' temp `
    if [ -n "$rst" ];then
        #echo "not empty"
        sed -i '$d' client${i}_temp.log
    fi
    
    awk '{print $1}' client${i}_temp.log > temp
    sed 'N; s/\n/ /' temp > client${i}_temp.log
    
    awk '{print $2-$1}' client${i}_temp.log > temp;cat temp > client${i}_temp.log
    average=`awk 'BEGIN{sum=0;count=0;}{sum +=$1;count++;}END{print sum/count}' client${i}_temp.log`
    max_downtime=`awk 'BEGIN{max=0;}{if($1>max) max=$1;}END{print max}' client${i}_temp.log`
    playcount=`awk 'BEGIN{count=0;}{count++;}END{print count}' client${i}_temp.log`
    echo -e "\taverage download time: "$average 
    echo -e "\tmax download time: "$max_downtime 
    echo -e "\tplay count: "$playcount
    tmp=`echo "$max_downtime > $max"|bc`
    if [ $tmp -eq 1 ];then
        max=$max_downtime
    fi
    echo -e "\tglobal max_download_time:"$max
    let totalPlayCount+=$playcount

    delaytimes=`awk -v playtime=$playtime 'BEGIN{delaytime=playtime;count=0}{if($1>delaytime) count++;}END{print count}' client${i}_temp.log`
    echo -e "\tdelay times:"$delaytimes
    
    sum=`echo " $sum + $average "|bc -l`
    let count+=1
    let delay+=$delaytimes
    #echo $delay
done

echo "###########################################"
echo "blocksize:"$blocksize
echo "bitrate:"$bitrate
echo "playtime: "$playtime
echo "total play count:"${totalPlayCount}
#echo "sum:"$sum
#echo "count:"$count
result=`echo "scale=6;$sum/$count"|bc -l`
echo "average download time:"$result
echo "global max_download_time:"$max
echo "total delay times:"$delay
echo "###########################################"

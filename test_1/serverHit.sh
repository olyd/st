#!/bin/bash

# get last line
sed -n '$,$p' buffer.log > temp1
awk -F: '{print $2}' temp1 > temp2
# get index 6 (totalTimes) index 7 (hitTimes)
eval $(awk '{printf("totalTimes=%d\nhitTimes=%d", $6,$7)};' temp2 )

#echo "totalTimes:"$totalTimes
#echo "hitTimes:"$hitTimes

# get server buffer hit %
result=`echo "scale=6;$hitTimes/$totalTimes" | bc -l`
#echo -e "\t"$result
printf "%-8s\n" $result



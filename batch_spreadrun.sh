#!/bin/bash

if [ $# -eq 2 ] ||[ $# -eq 3 ];then
    ipprefix=root@$1
    oper=$2
	shutdowntime=$3
    if [ $oper != "get" ] && [ $oper != "run" ] && [ $oper != "stop" ] && [ $oper != "shutdown" ];then
        echo "Usage:$0 [192.168.1] [get/run/stop]"
		echo "Usage:$0 [192.168.1] [shutdown] [90]"
        exit
    fi
else
    echo "Usage:$0 [192.168.1] [get/run/stop]"
    echo "Usage:$0 [192.168.1] [shutdown] [90]"
    exit
fi

file="spread_machine_config"
sed -n '/^on /p' $file > temp
iplist=`awk '{print $2}' temp`
#iplist=(11 12 13 14 15 16 21 22 23 24 25 26 17 18 19 27 28 29 37 38 39 47 48 49 57 58 59 20 30 40)
#iplist=(11 12 13 14 15 16)

for ip in ${iplist[@]}
do
	echo  $ipprefix.$ip
	ipEnable=off
	eval $(awk -v tmp=$ip '/on '"$ip"'/{ printf("	\
									ipEnable=%s\n	\
									ip=%s\n \
									strategy=%s\n \
									period=%s\n \
									capacity=%s\n \
									maxout=%s\n \
									maxin=%s\n \
									thresh=%s\n \
									threshHigh=%s\n",$1,$2,$3,$4,$5,$6,$7,$8,$9);}' $file)
	#echo -ne "\t$ipEnable"
	echo -ne "config is:  "
	if [ $ipEnable == "on" ];then
		printf "\t%-6s%-6s%-6s%-6s%-6s%-6s%-6s\n" $strategy $period $capacity $maxout $maxin $thresh $threshHigh
		
		sed "s/\(spreadAlgorithm=\).*/\1$strategy/g;
				s/\(period=\).*/\1$period/g
				s/\(maxCapacity=\).*/\1$capacity/g
				s/\(maxCopyFlow=\).*/\1$maxout/g
				s/\(maxInFlow=\).*/\1$maxin/g
				s/\(loadThresh=\).*/\1$thresh/g
				s/\(loadThreshHigh=\).*/\1$threshHigh/g" config/spread.ini > temp;cat temp > config/spread.ini;
		#cat config/spread.ini
		chmod u+x expect_spread.sh
		./expect_spread.sh $ipprefix.$ip $oper $shutdowntime
	else
		echo "$ipprefix.$ip is off..."
	fi
done

echo "batch spread over over over"

#!/bin/bash

if [ $# -eq 2 ] || [ $# -eq 3 ];then
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

file="strip_machine_config"
sed -n '/^on /p' $file > temp
iplist=`awk '{print $2}' temp`
#iplist=(11 12 13 14 15 16 21 22 23 24 25 26 17 18 19 27 28 29 37 38 39 47 48 49 57 58 59 20 30 40)
#iplist=(11 12 13 14 15 16)

for ip in ${iplist[@]}
do
	echo $ipprefix.$ip
	ipEnable=off
	eval $(awk -v tmp=$ip '/on '"$ip"'/{ printf("	\
									ipEnable=%s\n	\
									ip=%s\n \
									isP2POpen=%s\n \
									num=%s\n \
									serverStrategy=%s\n \
									serverBlockNum=%s\n \
									clientStrategy=%s\n \
									clientBlockNum=%s\n \
									period=%s\n \
									blockSize=%s\n",$1,$2,$3,$4,$5,$6,$7,$8,$9,$10);}' $file)
	#echo -ne "\t$ipEnable"
	echo -ne "config is:  "
	if [ $ipEnable == "on" ];then
		printf "\t%-6s%-6s%-6s%-6s%-6s%-6s%-6s%-6s\n" $isP2POpen $num $serverStrategy $serverBlockNum $clientStrategy $clientBlockNum $period $blockSize
		
		sed "s/\(isP2POpen=\).*/\1$isP2POpen/g;
				s/\(serverStrategy=\).*/\1$serverStrategy/g
				s/\(serverBlockNum=\).*/\1$serverBlockNum/g
				s/\(clientStrategy=\).*/\1$clientStrategy/g
				s/\(clientBlockNum=\).*/\1$clientBlockNum/g
				s/\(period=\).*/\1$period/g
				s/\(blockSize=\).*/\1$blockSize/g" config/strip.ini > temp;cat temp > config/strip.ini;
		#cat config/strip.ini
        chmod u+x expect_strip.sh
        if [ $num -eq 0 ];then
            ./expect_strip.sh $ipprefix.$ip 1 3 $oper $shutdowntime
        else
            ./expect_strip.sh $ipprefix.$ip $num $num $oper $shutdowntime
        fi
	else
		echo "$ipprefix.$ip is off..."
	fi
	if [ $oper == "run" ];then
		sleep 2s
	fi
done

echo "batch strip over over over"


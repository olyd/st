#!/usr/bin/expect

#puts $argc
if { ($argc != 2) && ($argc != 3) } {
	puts "Usage:$argv0 user@ip get/run/stop" 
	puts "Usage:$argv0 user@ip shutdown time(a number)" 
	exit
}
set timeout 10
set ip [lindex $argv 0]
set oper [lindex $argv 1]
set shutdowntime [lindex $argv 2]
set passwd "server-302"
#puts $ip,$startnum,$endnum,$oper

if {$oper == "run"} {
    spawn scp ./config/spread.ini "$ip:~/st/config/"
    expect {
    	"*yes/no" {
    		send "yes\r"
    		expect "*assword" {
    			send "$passwd\r"
    			send "exit\r"
    		}
    	}
    	"*assword:" {
    		send "$passwd\r"
    		send "exit\r"
    	}
    	expect eof
    }
}

spawn ssh -o StrictHostKeyChecking=no "$ip"
expect {
	"*assword:" {
		send "$passwd\r"
		expect "*#"
		
        if {$oper == "get"} {
		    #svn checkout
    		send "cd /root && rm st -rf && svn co svn://59.77.16.16/st\r"
	    	expect {
                "*#" {
                }
                "*peer*"{
    		        send "svn co svn://59.77.16.16/st\r"
                    expect "*#"
                }
            }
        }
        if {$oper == "run"} {
			send "cd /root/st && ./kill.sh && svn up\r"
			expect "*#"
			set timeout 2
		    send "nohup ./spreadrun.sh &\r"
            expect "*]#"
        }
        if {$oper == "stop"} {
			send "cd /root/st && ./kill.sh \r"
			set timeout 2
			expect "*#"
        }
		if {$oper == "shutdown"} {
			send "cd /root/st && ./shutdown.sh $shutdowntime \r"
			set timeout 2
			expect "*#"
		}
        #send "exit\r"
	}
	expect eof
}

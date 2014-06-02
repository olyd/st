#!/bin/bash

killall st
killall striprun.sh
killall spreadrun.sh
killall sleep

#kill -9 `ps aux | grep "sleep 5000s"   | sed -n '1,1p' | awk '{print $2}'`
#kill -9 `ps aux | grep "./striprun.sh" | sed -n '1,1p' | awk '{print $2}'`

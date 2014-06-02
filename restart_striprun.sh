#!/bin/bash

./kill.sh
svn up
make
./striprun.sh &

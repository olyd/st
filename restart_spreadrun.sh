#!/bin/bash

./kill.sh
svn up
make
./spreadrun.sh
ls

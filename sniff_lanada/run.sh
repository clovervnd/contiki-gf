#!/bin/bash
APP=$1
ID=$2

make clean
make $1 NODEID=$ID
msp430-objcopy $1.exp5438 -O ihex $1.ihex
sudo make $1.upload

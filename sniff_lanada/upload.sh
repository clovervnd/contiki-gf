#!/bin/bash
APP=$1

msp430-objcopy $1.exp5438 -O ihex $1.ihex
sudo make $1.upload

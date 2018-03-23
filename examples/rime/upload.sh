#!/bin/bash 
make clean
make example-broadcast > result.txt
msp430-objcopy example-broadcast.exp5438 -O ihex example-broadcast.ihex
sudo make example-broadcast.upload

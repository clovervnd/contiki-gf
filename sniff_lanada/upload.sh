#!/bin/bash
#NODE=$1

msp430-objcopy sniff_test.exp5438 -O ihex sniff_test.ihex
sudo make sniff_test.upload

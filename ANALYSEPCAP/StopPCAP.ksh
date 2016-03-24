#! /bin/bash

killall PCAP_ANALY tail grep 
sleep 2 
killall -9 PCAP_ANALY tail grep 

exit 0

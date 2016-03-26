#! /bin/bash

killall PCAP_ANALY tail grep StartPCAP.ksh 
sleep 2 
killall -9 PCAP_ANALY tail grep StartPCAP.ksh

exit 0

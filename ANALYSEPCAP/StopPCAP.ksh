#! /bin/bash
APPNAME=PCAP_ANALY_TCPT
killall $APPNAME tail grep StartPCAP.ksh 
sleep 2 
killall -9 $APPNAME tail grep StartPCAP.ksh

exit 0

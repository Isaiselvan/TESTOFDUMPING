#! /bin/bash


./PCAP_ANALY > result.log &
tail -f result.log | grep --line-buffered "Splunk" > splunk.log &
tail -f splunk.log





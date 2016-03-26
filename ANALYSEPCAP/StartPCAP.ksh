#! /bin/bash


startanal() {
./PCAP_ANALY > result.log &
tail -f result.log | grep --line-buffered "Splunk" > splunk.log &
#tail -f splunk.log
}


stopanal(){
killall PCAP_ANALY tail grep
sleep 2
killall -9 PCAP_ANALY tail grep
}

while true
do
mon=`pgrep PCAP_ANALY`
echo "$mon START..."
  if [ -z $mon ]
  then
  echo "Starting Analyser"
  stopanal
  startanal
  fi
echo "$mon START..."
sleep 5
done




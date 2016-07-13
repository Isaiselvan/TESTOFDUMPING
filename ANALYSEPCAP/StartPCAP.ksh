#! /bin/ksh

APPNAME=PCAP_ANALY_TCPT

startanal() {
#./$APPNAME > result.log &
./PCAP_ANALY_TCPT -l pcapanal1.log -i 1 -c 7 > metrics1.log 2>&1 &
./PCAP_ANALY_TCPT -l pcapanal2.log -i 2 -c 6 > metrics2.log 2>&1 &
./PCAP_ANALY_TCPT -l pcapanal3.log -i 3 -c 5 > metrics3.log 2>&1 &
./PCAP_ANALY_TCPT -l pcapanal4.log -i 4 -c 4 > metrics4.log 2>&1 &
./PCAP_ANALY_TCPT -l pcapanal5.log -i 5 -c 3 > metrics5.log 2>&1 &
./PCAP_ANALY_TCPT -l pcapanal1.log -i 1 -c 7 > MetricsDump.log 2>&1 &
tail -f result.log | grep --line-buffered "Splunk" > splunk.log &
#tail -f splunk.log
}


stopanal(){
killall $APPNAME tail grep
sleep 2
killall -9 $APPNAME tail grep
}
restart=`cat /dev/null`
mon=`cat /dev/null`
while true
do
mon=`pgrep $APPNAME `
restart=`cat /dev/null`
smov=`ls splunk.log`
if [ -z $smov ]
then
echo "File does not exists"
restart="true"
fi

smov=`find . -name "splunk.log" -mmin +1`

if [ ! -z $smov ]
then
restart="true"
fi

#echo "$smov smov" 
#echo "$mon START..."
#echo "$restart reSta.."

if [ -z $mon ] || [ ! -z $restart ] 
  then
  echo "Starting Analyser"
  stopanal
  startanal
  fi
#echo "$mon START..."
sleep 5
done




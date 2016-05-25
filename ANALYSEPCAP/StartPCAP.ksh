#! /bin/ksh

APPNAME=PCAP_ANALY_TCPT

startanal() {
./$APPNAME > result.log &
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




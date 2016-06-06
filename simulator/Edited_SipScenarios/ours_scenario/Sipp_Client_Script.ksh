#! /bin/ksh


start() {
nohup sudo /usr/local/bin/sipp -sn uac $1:55123  > /dev/null 2>&1 & 
nohup sudo /usr/local/bin/sipp -sn uac $1:55124  > /dev/null 2>&1 & 
nohup sudo /usr/local/bin/sipp -sn uac $1:55125  > /dev/null 2>&1 & 
nohup sudo /usr/local/bin/sipp -sn uac $1:55126  > /dev/null 2>&1 & 
nohup sudo /usr/local/bin/sipp -sn uac $1:55127  > /dev/null 2>&1 & 
nohup sudo /usr/local/bin/sipp -sn uac $1:55128  > /dev/null 2>&1 & 
nohup sudo /usr/local/bin/sipp -sn uac $1:55129  > /dev/null 2>&1 & 
nohup sudo /usr/local/bin/sipp -sn uac $1:55130  > /dev/null 2>&1 & 
nohup sudo /usr/local/bin/sipp -sn uac $1:55131  > /dev/null 2>&1 &

nohup sudo tcpreplay --loop=0 --intf1=eth0 /home/training/FBMPOC/tcpreplay/ngtp_complete_activation.pcap1.pcap > /dev/null 2>&1 & 
nohup sudo tcpreplay --loop=0 --intf1=eth0 /home/training/FBMPOC/tcpreplay/new1.pcap > /dev/null 2>&1 & 

nohup iperf -c $1 -b 1G -t 172800 -u > /dev/null 2>&1 &
nohup iperf -c $1 -t 172800 > /dev/null 2>&1 & 
}


if [ -z $1 ]
    then
      start 127.0.0.1
else
      start $1 
fi


#! /bin/bash

PGDEV=/proc/net/pktgen/kpktgend_0
function pgset() 
{
	local result
	echo  $1 > $PGDEV
}

COUNT="count 1000000000"
#RATEP="ratep 1440000"

pgset "rem_device_all"
pgset "add_device eth0"
echo "Added  $PGDEV"
#PGDEV=/proc/net/pktgen/kpktgend_1
#pgset "rem_device_all"
#pgset "add_device eth0"
#echo "Added  $PGDEV"
#PGDEV=/proc/net/pktgen/kpktgend_2
#pgset "rem_device_all"
#pgset "add_device eth0"
#echo "Added  $PGDEV"


PGDEV=/proc/net/pktgen/eth0
echo "Configuring $PGDEV"
pgset "clone_skb 1000000"
pgset "pkt_size 60"
#pgset "max_pkt_size 60"
#pgset "dst 10.76.190.86"
#pgset "dst_mac 34:17:EB:AF:06:A7"
#DevStack Details
pgset "dst 10.76.190.80"
pgset "dst_mac 34:17:eb:af:08:f3"

pgset "delay 0"
pgset $COUNT
pgset $RATEP
#pgset "udp_src_min 6000"
#pgset "udp_src_max 6000"
#pgset "udp_dst_min 123"
#pgset "udp_dst_max 123"

echo "Running... CTRL-C to stop"
PGDEV=/proc/net/pktgen/pgctrl
pgset "start"
echo "Started Sending"

# Di s pl a y the r e s u l t s
grep -h pps /proc/net/pktgen/eth0

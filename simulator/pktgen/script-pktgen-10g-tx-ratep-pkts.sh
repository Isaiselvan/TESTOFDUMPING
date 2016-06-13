#! /bin/sh
# $1 Rate in packets per s
# $2 Number of CPUs to use
function pgset() {
    local result
    echo $1 > $2
}

# Config Start Here -----------------------------------------------------------

# thread config
CPUS=$2
PKTS=`echo "scale=0; $3/$CPUS" | bc`
CLONE_SKB="clone_skb 10"
PKT_SIZE="pkt_size 60"
COUNT="count $PKTS"
DELAY="delay 0"
MAC="34:17:EB:AF:06:A7"
ETH="eth0"

RATEP=`echo "scale=0; $1/$CPUS" | bc`

for processor in {0..7}
do
PGDEV=/proc/net/pktgen/kpktgend_$processor
#  echo "Removing all devices"
 pgset "rem_device_all" "$PGDEV"
done


for ((processor=0;processor<$CPUS;processor++))
do
PGDEV=/proc/net/pktgen/kpktgend_$processor
#  echo "Adding $ETH"
 pgset "add_device $ETH@$processor" "$PGDEV"
 
PGDEV=/proc/net/pktgen/$ETH@$processor
#  echo "Configuring $PGDEV"
 pgset "$COUNT" "$PGDEV"
 pgset "flag QUEUE_MAP_CPU" "$PGDEV" 
 pgset "$CLONE_SKB" "$PGDEV"
 pgset "$PKT_SIZE" "$PGDEV"
 pgset "$DELAY" "$PGDEV"
 pgset "ratep $RATEP" "$PGDEV"
 pgset "dst 113.128.163.198" "$PGDEV" 
 pgset "dst_mac $MAC" "$PGDEV"
 # Random address with in the min-max range
 pgset "flag IPDST_RND" "$PGDEV"
 pgset "dst_min 10.0.0.0" "$PGDEV"
 pgset "dst_max 10.255.255.255" "$PGDEV"
# enable configuration packet
 pgset "config 1" "$PGDEV"
 pgset "flows 1024" "$PGDEV"
 pgset "flowlen 8" "$PGDEV"
done

# Time to run
PGDEV=/proc/net/pktgen/pgctrl

 echo "Running... ctrl^C to stop"
 pgset "start" "$PGDEV"
 echo "Done"

#grep -h pps /proc/net/pktgen/eth*

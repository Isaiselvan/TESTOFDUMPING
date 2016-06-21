#! /bin/ksh 

#sudo rmmod pf_ring
#sudo insmod /lib/modules/2.6.32-573.22.1.el6.x86_64/kernel/net/pf_ring/pf_ring.ko min_num_slots=16384 enable_tx_capture=0 quick_mode=1
nohup sudo ./build/icis-dump -l 5,6,7 -n 4 -- -w /apps/opt/LIBTRACE/test/ICISPCAP -G 10 -B 1048576 -i eth0 > result.log 2>&1  & 
#nohup sudo ./build/icis-dump -l 5,6,7 -n 4 --vdev "eth_pcap0,iface=eth0" -- -w /apps/opt/LIBTRACE/test/ICISPCAP -G 30 -B 1048576 -q 8 -p 0x01 
#nohup sudo ./build/icis-dump -l 0,6,7 -n 4 -- -w /apps/opt/LIBTRACE/test/ICISPCAP -G 10 -B 2097152 -p 0x01 > /dev/null 2>&1 
sleep 5 

Healthcheck=`find . -name "DumperStat.log" -mmin +1`

result="bad"

if [  -z $Healthcheck ] 
then
result=`cat /dev/null`
fi

if [ -z $result ]
 then
   echo "FBM Dumper started .... "
   #tail -f DumperStat.log & 
else 
   echo "Dumper failed to start .... Start again"
   echo " "
fi

exit


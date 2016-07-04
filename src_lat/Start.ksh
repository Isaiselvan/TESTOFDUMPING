#! /bin/ksh 

#sudo rmmod pf_ring
#sudo insmod /lib/modules/2.6.32-573.22.1.el6.x86_64/kernel/net/pf_ring/pf_ring.ko min_num_slots=16384 enable_tx_capture=0 quick_mode=1
#nohup sudo ./build/icis-dump -l 5,6,7 -n 4 -- -w /apps/opt/LIBTRACE/test/ICISPCAP -G 10 -B 1048576 -i eth0 > result.log 2>&1  & 
#nohup sudo ./build/icis-dump -l 5,6,7 -n 4 --vdev "eth_pcap0,iface=eth0" -- -w /apps/opt/LIBTRACE/test/ICISPCAP -G 30 -B 1048576 -q 8 -p 0x01 
#nohup sudo ./build/icis-dump -l 0,6,7 -n 4 -- -w /apps/opt/LIBTRACE/test/ICISPCAP -G 10 -B 2097152 -p 0x01 > /dev/null 2>&1 
#sudo ./build/Vzprobe-dump -l 0,2,1 -n 4 --vdev 'eth_pcap0,iface=eth0' -- -w /apps/opt/LIBTRACE/test/FBMPOC -p 0x0f -G 5 -B 1048576
#sudo ./build/Vzprobe-dump -l 0,2,1 -n 4 --vdev 'eth_pcap0,iface=eth0'  --vdev 'eth_pcap1,iface=eth0' -- -w /apps/opt/LIBTRACE/test/FBMPOC -p 0x0f --config='(0,0,0),(1,0,0)' -G 5 -B 1048576

#sudo ./build/Vzprobe-dump -l 0,1,2,3,4,5 -n 4 --vdev 'eth_pcap0,iface=lo'  --vdev 'eth_pcap1,iface=lo' --vdev 'eth_pcap2,iface=lo'  --vdev 'eth_pcap3,iface=lo' -- -w /apps/opt/LIBTRACE/test/FBMPOC -p 0x0f --config='(0,0,0),(1,0,1),(2,0,2),(3,0,3)' -G 5 -B 1048576 --wcore 4

##MULTI (PORT, QUEUE, CORE)
#sudo ./build/Vzprobe-dump -l 0,1,2,3,4,5 -n 4 --vdev 'eth_pcap0,iface=lo'  --vdev 'eth_pcap1,iface=lo' --vdev 'eth_pcap2,iface=eno16777736'  --vdev 'eth_pcap3,iface=eno33554984' -- -w /apps/opt/LIBTRACE/test/FBMPOC -p 0x0f --config='(0,0,0),(1,0,1),(2,0,2),(3,0,3)' -G 5 -B 1048576 --wcore 4

nohup sudo ./build/Vzprobe-dump -l 0,1,2,3,4,5 -n 4  -- -w /apps/opt/LIBTRACE/test/FBMPOC -p 0x0f --config='(0,0,0),(1,0,1),(2,0,2),(3,0,3)' -G 5 -B 1048576 --wcore 4 > result.log 2>&1  &
nohup sudo ./build/Vzprobe-dump -l 0,1,2,3,4,5,6,7,8,9,10 -n 32 -- -w /apps/opt/LIBTRACE/test/FBMPCAP -G 10 -p 0x01 -B 33554432 --wcore 7 --config "(0,0,0),(0,1,1),(0,2,2),(0,3,4),(0,4,6)"

#10Mpps without loss 
sudo ./build/Vzprobe-dump -l 0,1,2,3,4,5,6 -n 32 -- -w /apps/opt/LIBTRACE/test/FBMPCAP -G 10 -p 0x01 -B 33554432 --wcore 3 --config "(0,0,0),(0,1,1),(0,2,2),(0,3,3),(0,4,4)" > result.log 2>&1 &
Dpdk 21 
RAM node0: 15360

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


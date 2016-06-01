#! /bin/ksh 

sudo rmmod pf_ring
sudo insmod /lib/modules/2.6.32-573.22.1.el6.x86_64/kernel/net/pf_ring/pf_ring.ko min_num_slots=16384 enable_tx_capture=0 quick_mode=1
nohup sudo ./build/icis-dump -l 5,6,7 -n 8 -- -w /apps/opt/LIBTRACE/test/ICISPCAP -G 5 -B 1048576 -i em1 > /dev/null 2>&1  & 

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


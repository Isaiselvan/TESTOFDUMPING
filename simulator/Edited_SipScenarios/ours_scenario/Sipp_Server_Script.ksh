#! /bin/ksh


#nohup /usr/local/bin/sipp -sf sipp_uas_300.xml -p 55123 > /dev/null 2>&1 &
#nohup /usr/local/bin/sipp -sf sipp_uas_305.xml -p 55124 > /dev/null 2>&1 &
#nohup /usr/local/bin/sipp -sf sipp_uas_420.xml -p 55125 > /dev/null 2>&1 &
#nohup /usr/local/bin/sipp -sf sipp_uas_486.xml -p 55126 > /dev/null 2>&1 &
#nohup /usr/local/bin/sipp -sf sipp_uas_491.xml -p 55127 > /dev/null 2>&1 &
#nohup /usr/local/bin/sipp -sf sipp_uas_500.xml -p 55128 > /dev/null 2>&1 &
#nohup /usr/local/bin/sipp -sf sipp_uas_505.xml -p 55129 > /dev/null 2>&1 &
#nohup /usr/local/bin/sipp -sf sipp_uas_603.xml -p 55130 > /dev/null 2>&1 &
#nohup /usr/local/bin/sipp -sf sipp_uas_606.xml -p 55131 > /dev/null 2>&1 &


nohup iperf -s > /dev/null 2>&1 &
nohup iperf -s -u > /dev/null  2>&1 & 


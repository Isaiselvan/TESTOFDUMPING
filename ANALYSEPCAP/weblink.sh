#!/bin/bash 


cd /apps/opt/LIBTRACE/test/

StartWeblink(){
        COUNTER=0
         while [  $COUNTER -lt 15 ]; do
            # echo The counter is $COUNTER
            /usr/bin/wget www.google.com > /dev/null 2>&1 &
            /usr/bin/wget www.youtube.com > /dev/null 2>&1 &
            /usr/bin/wget www.netflix.com > /dev/null  2>&1 &
            /usr/bin/wget www.yahoo.com > /dev/null 2>&1 &
            let COUNTER=COUNTER+1 
         done
}

Times=0
while [ $Times -lt 1200 ]; do
StartWeblink
sleep 3
let Times=Times+1
done

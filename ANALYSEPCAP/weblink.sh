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

#https://verizon.webex.com/mw0401lsp13/mywebex/default.do?service=1&siteurl=verizon&nomenu=true&main_url=%2Fmc0901lsp13%2Fmeetingcenter%2Fmeetinginfo%2Fmeetinginfo.do%3Fsiteurl%3Dverizon%26confID%3D3422228124%26Action%3DWMI%26MTID%3Dma743d1ccaa2755945b238ef0495e652a%26FrameSet%3D2%26Host%3DQUhTSwAAAALCCetHNA0UhecgA3RuPkH1SDTXY3y0jop_nsz3qy1E2INU5R_Mdg25JaDEyCB3QnENcTaPOxOy8nzhbRJjjAtL0%26UID%3D0%26trackingNumber%3D271580266

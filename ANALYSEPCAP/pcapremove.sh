#!/bin/bash 


cd /apps/opt/LIBTRACE/test/

StartWeblink(){
              #find /apps/opt/LIBTRACE/test/*.taken -type f -mmin +1 | xargs rm -f > /dev/null 
              #find /apps/opt/LIBTRACE/test/*ready.pcap -type f | xargs rm -f > /dev/null 
              find /apps/opt/LIBTRACE/test/*.completed -type f | xargs rm -f > /dev/null
}

Times=1
while [ $Times ]; do
StartWeblink
sleep 1
done

#https://verizon.webex.com/mw0401lsp13/mywebex/default.do?service=1&siteurl=verizon&nomenu=true&main_url=%2Fmc0901lsp13%2Fmeetingcenter%2Fmeetinginfo%2Fmeetinginfo.do%3Fsiteurl%3Dverizon%26confID%3D3422228124%26Action%3DWMI%26MTID%3Dma743d1ccaa2755945b238ef0495e652a%26FrameSet%3D2%26Host%3DQUhTSwAAAALCCetHNA0UhecgA3RuPkH1SDTXY3y0jop_nsz3qy1E2INU5R_Mdg25JaDEyCB3QnENcTaPOxOy8nzhbRJjjAtL0%26UID%3D0%26trackingNumber%3D271580266

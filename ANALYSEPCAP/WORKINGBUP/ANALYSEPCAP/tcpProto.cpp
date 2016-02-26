#include "tcpProto.h"


int protocolTCP::addPkt(libtrace_packet_t *pkt, m_tcpPacket tcppkt)
{
 //DownLink flag can be set here to tcppkt
     m_totalpkts++;
     m_totaldata+=tcppkt.getDataLen();  
     if( tcppkt.ethernetlayer.ether_type == TRACE_ETHERTYPE_IP)
     m_totalipv4++;
     else if ( tcppkt.ethernetlayer.ether_type == TRACE_ETHERTYPE_IPV6 )
     m_totalip6++;
   
     if(!trace_get_direction (pkt))
     {
       m_totaldownlink+=tcppkt.getDataLen();
       tcppkt.Downlink = true; 
     }else 
     {
      m_totaluplink+=tcppkt.getDataLen();
      tcppkt.Downlink = false;
     }
     

     m_pkt.push_back(tcppkt);
     
 // Add session logic
}

int protocolTCP::addSession(libtrace_packet_t *pkt,m_tcpPacket tcppkt)
{
  m_tcpSession frameSession; 
  char addrstr_src[INET_ADDRSTRLEN];  
  char addrstr_dst[INET_ADDRSTRLEN];
             if(tcppkt.ipv == 4)
             {
             inet_ntop(AF_INET, &(tcppkt.ip4.ip_src), addrstr_src, INET_ADDRSTRLEN);
             inet_ntop(AF_INET, &(tcppkt.ip4.ip_dst), addrstr_dst, INET_ADDRSTRLEN);
             }
             else 
             {
             inet_ntop(AF_INET, &(tcppkt.ipv6.ip_src), addrstr_src, INET_ADDRSTRLEN);
             inet_ntop(AF_INET, &(tcppkt.ipv6.ip_dst), addrstr_dst, INET_ADDRSTRLEN);
             }

             frameSession.scrIpPort = (std::string)addrstr_src ;
             frameSession.src = tcppkt.tcp.source;   
             frameSession.destIpPort = (std::string)addrstr_dst ;
             frameSession.dst = tcppkt.tcp.dest;
             
          
             if(m_session.size() == 0 ) 
             {
              //Add time   
               m_session.push_back(frameSession);
             } 
             else 
             {

             }  
// Iterator for session

 return 0;
}

unsigned long int protocolTCP::bandWidthCalc () {
         m_bandwidth = m_totaldata * 8/ (m_endtime - m_starttime) ;
         return m_bandwidth;
}

void protocolTCP::calculatemetrics() {

}

void protocolTCP::displaymetrics() {
    std::cout << "\nTcp layer Metrics" << std::endl;
    std::cout << "Tcp packet total count = " << m_totalpkts << std::endl;
    std::cout << "Total Ipv4 packets = " << m_totalipv4 << std::endl;
    std::cout << "Total Ipv6 packets = " << m_totalip6 << std::endl;
    std::cout << "Tcp data usage = " << m_totaldata/1024/1024 << "MB" << std::endl;    
    std::cout << "Number of distincted sessions " << m_totalSession << std::endl;
    std::cout << "BandWidth usage " << (bandWidthCalc () / 1024 /1024 ) << "Mbps" << std::endl;
    std::cout << "Total Uplink data = " << (m_totaluplink / 1024/1024) << "MB" << std::endl  ;
    std::cout << "Total Downlink data = " << (m_totaldownlink/1024/1024) << "MB\n" << std::flush  ;
}

int getPercUplink(){}

int getPercDownLink(){}


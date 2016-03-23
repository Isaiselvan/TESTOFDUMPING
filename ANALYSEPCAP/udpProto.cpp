#include "udpProto.h"


int protocolUDP::addPkt(libtrace_packet_t *pkt, m_Packet &udppkt)
{
 //DownLink flag can be set here to tcppkt
     m_totalpkts++;
     m_totaldata+=udppkt.getDataLen();  
     if( udppkt.ethernetlayer.ether_type == TRACE_ETHERTYPE_IP)
     m_totalipv4++;
     else if ( udppkt.ethernetlayer.ether_type == TRACE_ETHERTYPE_IPV6 )
     m_totalip6++;
   
     if(!trace_get_direction (pkt))
     {
       m_totaldownlink+=udppkt.getDataLen();
       udppkt.Downlink = true; 
     }else 
     {
      m_totaluplink+=udppkt.getDataLen();
      udppkt.Downlink = false;
     }
     

    //m_pkt.push_back(udppkt); //Comment
         
 // Add AppLayer
    layerSeven.processPkt(pkt, udppkt);
     
 // Adding to lte , Will be added only if it has a Gtp
    getLteDash()->parseGtp(pkt, (char *)udppkt.pay_load);

  return 0; 
}

int protocolUDP::addSession(libtrace_packet_t *pkt,m_Packet udppkt)
{
  m_udpSession frameSession; 
  char addrstr_src[INET_ADDRSTRLEN];  
  char addrstr_dst[INET_ADDRSTRLEN];
             if(udppkt.ipv == 4)
             {
             inet_ntop(AF_INET, &(udppkt.ip4.ip_src), addrstr_src, INET_ADDRSTRLEN);
             inet_ntop(AF_INET, &(udppkt.ip4.ip_dst), addrstr_dst, INET_ADDRSTRLEN);
             }
             else 
             {
             inet_ntop(AF_INET, &(udppkt.ipv6.ip_src), addrstr_src, INET_ADDRSTRLEN);
             inet_ntop(AF_INET, &(udppkt.ipv6.ip_dst), addrstr_dst, INET_ADDRSTRLEN);
             }

             frameSession.scrIpPort = (std::string)addrstr_src ;
             frameSession.src = udppkt.udp.source;   
             frameSession.destIpPort = (std::string)addrstr_dst ;
             frameSession.dst = udppkt.udp.dest;
             
          
             if(m_session.size() == 0 ) 
             {
              //Add time   
               //m_session.push_back(frameSession);
             } 
             else 
             {

             }  
// Iterator for session

 return 0;
}

unsigned long int protocolUDP::bandWidthCalc () {
         m_bandwidth = m_totaldata * 8/ (m_endtime - m_starttime) ;
         return m_bandwidth;
}

void protocolUDP::calculatemetrics() {

}

void protocolUDP::displaymetrics(std::string splunkkey) {
    std::cout << "\nUdp layer Metrics" << std::endl;
    std::cout << "\nSTARTTIME: " << m_starttime << "\tENDTIME: " << m_endtime << std::endl;
    std::cout << "Udp packet total count = " << m_totalpkts << std::endl;
    std::cout << "Total Ipv4 packets = " << m_totalipv4 << std::endl;
    std::cout << "Total Ipv6 packets = " << m_totalip6 << std::endl;
    std::cout << "Udp data usage = " << m_totaldata/1024/1024 << "MB" << std::endl;    
    std::cout << "Number of distincted sessions " << m_totalSession << std::endl;
    std::cout << "BandWidth usage " << (bandWidthCalc () / 1024 /1024 ) << "Mbps" << std::endl;
    std::cout << "Total Uplink data = " << (m_totaluplink / 1024/1024) << "MB" << std::endl  ;
    std::cout << "Total Downlink data = " << (m_totaldownlink/1024/1024) << "MB\n" << std::flush  ;
    
    std::cout << splunkkey << " Total_pkt=" << m_totalpkts << " Total_Datalen=" << m_totaldata
               << " BandWidth=" << (bandWidthCalc () ) << " Total_Uplink="
                << (m_totaluplink ) << " Total_DoLink=" << (m_totaldownlink) <<
                 " Ipv4=" << m_totalipv4 << " Ipv6=" << m_totalip6 << std::endl;
   // Print lte stats
     getLteDash()->printstats(splunkkey);

   // Applayer info
      layerSeven.printStat(splunkkey);    
}


//int protocolUDP::topLayer(libtrace_packet_t *)
//{
 // Check if the UPD payload is a GTPU
 //trace_get_payload_from_udp (libtrace_udp_t *udp, uint32_t *remaining);  
//}


int protocolUDP::getPercUplink(){}

int protocolUDP::getPercDownLink(){}


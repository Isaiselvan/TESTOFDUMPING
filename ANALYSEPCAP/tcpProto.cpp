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
 

 return 0;
}

int protocolTCP::bandWidthCalc () {
         m_bandwidth = (m_totaldata / (m_endtime - m_starttime)) * 8;
         return m_bandwidth;
}

void protocolTCP::calculatemetrics() {

}

void protocolTCP::displaymetrics() {
    std::cout << "\nTcp layer Metrics" << std::endl;
    std::cout << "Tcp packet total count = " << m_totalpkts << std::endl;
    std::cout << "Tcp data usage = " << m_totaldata << std::endl;    
    std::cout << "Number of distincted sessions " << m_session.size() << std::endl;    std::cout << "BandWidth usage " << (bandWidthCalc () / 1024 /1024 ) << "Mbps" << std::endl;
    std::cout << "Total Uplink data = " << (m_totaluplink / 1024/1024) << "MB" << std::endl  ;
    std::cout << "Total Downlink data = " << (m_totaldownlink/1024/1024) << "MB" << std::endl  ;
}

int getPercUplink(){}

int getPercDownLink(){}


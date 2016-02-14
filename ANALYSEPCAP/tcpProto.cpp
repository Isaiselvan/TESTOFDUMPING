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

     m_pkt.push_back(tcppkt);
     
 // Add session logic
         
}


int protocolTCP::bandWidthCalc () {

}

void protocolTCP::calculatemetrics() {

}

void protocolTCP::displaymetrics() {
    std::cout << "\nTcp layer Metrics" << std::endl;
    std::cout << "Tcp total count = " << m_totalpkts << std::endl;
    std::cout << "Tcp data uage = " << m_totaldata << std::endl;    std::cout << "Number of distincted sessions " << m_session.size() << std::endl; 
    
}

int getPercUplink(){}

int getPercDownLink(){}

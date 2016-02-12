#include <iostream>
#include <string>
#include <string.h>
#include <list>
#include "protocol.h"
#include "libtrace.h"

typedef struct TcpPacket{
  libtrace_ether_t ethernetlayer;
  libtrace_ip_t  ip4;
  libtrace_ip6_t ipv6;
  libtrace_tcp_t tcp; 
}m_tcpPacket;

typedef struct tcpSession{

  std::string scrIp;
  std::string destIp;
  time_t starttime;
  time_t endtime;
//port
//port
}m_tcpSession; 


class protocolTCP : public protocolBase{

private:
   int m_totalpkts;   
   int m_totaldata;
   int m_totaluplink;
   int m_totaldownlink; 
   int m_percentageUplink;
   int m_percentagedownlink;
   int m_totalipv4;
   int m_totalip6;
   int m_bandwidth; 
   std::list<m_tcpSession> m_session;
   std::list<m_tcpPacket>  m_pkt;
public:
   protocolTCP(int start,int end):protocolBase(TRACE_IPPROTO_TCP, start, end)
   {
     m_totalpkts = 0;
     m_totaldata = 0; 
     m_totaluplink = 0;
     m_totaldownlink = 0; 
     m_percentageUplink = 0;
     m_percentagedownlink = 0;
     m_totalipv4 = 0;
     m_totalip6 = 0;
   }

   ~protocolTCP(){
    m_session.clear();
    m_pkt.clear();
    }; 
   
   int addPkt(libtrace_packet_t *pkt);  
   int bandWidthCalc();
   void calculatemetrics();
   void displaymetrics();
   int getPercUplink();
   int getPercDownLink();
};


//<sourceIP/Node, >
//distincted source ip / dest ip duration of session 
//

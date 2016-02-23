#include <iostream>
#include <string>
#include <string.h>
#include <list>
#include "protocol.h"
//#include "libtrace.h"
#include "libtrace_parallel.h"

typedef struct TcpPacket{ // Used as template refer other members to change name
  libtrace_ether_t ethernetlayer;
  libtrace_ip_t  ip4;
  libtrace_ip6_t ipv6;
  libtrace_tcp_t tcp;
  int timeStamp; 
  int ipv; 
  int dataLen;
  bool Downlink;// Set this flag in
  int getDataLen(){ 
  if(dataLen < 0) 
  std::cout << "Datalen lessthan 0" << dataLen << std::endl;    
  return dataLen; }
    
}m_tcpPacket;

typedef struct tcpSession{

  std::string scrIpPort;
  std::string destIpPort;
  time_t starttime;
  time_t endtime;
  int src;
  int dst;
}m_tcpSession; 


class protocolTCP : public protocolBase{

private:
   unsigned long  int  m_totalpkts;   
   unsigned long  int  m_totaldata;
   unsigned long  int  m_totaluplink;
   unsigned long  int  m_totaldownlink; 
   int m_percentageUplink;
   int m_percentagedownlink;
   int m_totalipv4;
   int m_totalip6;
   unsigned long int m_bandwidth; 
   unsigned long  int m_totalSession;
   std::list<m_tcpSession> m_session;
   std::list<m_tcpPacket>  m_pkt;
public:
   protocolTCP(int start,int end):protocolBase(TRACE_IPPROTO_TCP, start, end)
   {
     m_totalpkts = 0;
     m_totaldata = 0; 
     m_totaluplink = 0;
     m_totalSession = 0;
     m_totaldownlink = 0; 
     m_percentageUplink = 0;
     m_percentagedownlink = 0;
     m_totalipv4 = 0;
     m_totalip6 = 0;
   }

   virtual ~protocolTCP(){
    m_session.clear();
    m_pkt.clear();
    }; 
     
   int addPkt(libtrace_packet_t *, m_tcpPacket);///Used in template use the same name in other classes  
   unsigned long int bandWidthCalc();
   void calculatemetrics();
   void displaymetrics();
   int addSession(libtrace_packet_t *pkt,m_tcpPacket tcppkt);
   int getPercUplink();
   int getPercDownLink();
};


//<sourceIP/Node, >
//distincted source ip / dest ip duration of session 
//

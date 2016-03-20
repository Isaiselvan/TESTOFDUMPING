#ifndef TCP_PROTO_H
#define TCP_PROTO_H
#include <iostream>
#include <string>
#include <string.h>
#include <list>
#include "protocol.h"
//#include "libtrace.h"
#include "libtrace_parallel.h"
#include "packetCmm.h"

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
   std::list<m_Packet>  m_pkt;
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
     
   int addPkt(libtrace_packet_t *, m_Packet&);///Used in template use the same name in other classes  
   unsigned long int bandWidthCalc();
   void calculatemetrics();
   void displaymetrics(std::string splunkkey);
   int addSession(libtrace_packet_t *pkt,m_Packet tcppkt);
   int getPercUplink();
   int getPercDownLink();
};

#endif //TCP_PROTO_H
//<sourceIP/Node, >
//distincted source ip / dest ip duration of session 
//

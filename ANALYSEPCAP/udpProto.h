#ifndef UDP_PROTO_H
#define UDP_PROTO_H
#include <iostream>
#include <string>
#include <string.h>
#include <list>
#include <map>
#include "protocol.h"
//#include "libtrace.h"
#include "libtrace_parallel.h"
#include "packetCmm.h"
#include "lteGtp.h"

typedef struct udpSession{

  std::string scrIpPort;
  std::string destIpPort;
  time_t starttime;
  time_t endtime;
  int src;
  int dst;
}m_udpSession; 

//class LteProtoBase;

class protocolUDP : public protocolBase{

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
   std::list<m_udpSession> m_session;
   std::list<m_Packet>  m_pkt;
   LteProtoBase *LteDash; 
   //appLayer layerSeven;
public:
   protocolUDP(int start,int end):protocolBase(TRACE_IPPROTO_UDP, start, end)
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
     LteDash = NULL;
   }

   virtual ~protocolUDP(){
    m_session.clear();
    m_pkt.clear();
    if(LteDash){
       delete LteDash;
     }
    }; 
     
   int addPkt(libtrace_packet_t *, m_Packet *);  
   unsigned long int bandWidthCalc();
   void calculatemetrics();
   void displaymetrics(std::string splunkkey);
   int addSession(libtrace_packet_t *pkt,m_Packet udppkt);
   int getPercUplink();
   int getPercDownLink();
   LteProtoBase * getLteDash(){
    if(!LteDash)
        LteDash = new LteProtoBase(m_starttime, m_endtime);     

     return LteDash;
   }
  //int topLayer(libtrace_packet_t *);
};

#endif //UDP_PROTO_H
//<sourceIP/Node, >
//distincted source ip / dest ip duration of session 
//

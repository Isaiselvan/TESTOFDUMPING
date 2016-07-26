#ifndef PACKET_COMMON_H
#define PACKET_COMMON_H

#include <iostream>
#include <string>
#include <string.h>
#include <list>
//#include "protocol.h"
//#include "libtrace.h"
#include "libtrace_parallel.h"
//
#define TIMEINT 60 //Sec 

typedef struct Packet{ // Used as template refer other members to change name
  libtrace_ether_t ethernetlayer;
  libtrace_ip_t  ip4;
  uint8_t * ip_ptr;//For internal purpose. Should be null after flow
  int ipSize; // Added for DPI
  libtrace_ip6_t ipv6; 
  libtrace_udp_t udp;
  libtrace_tcp_t tcp;
  libtrace_ipproto_t type;
  double timeStamp;
  int ipv;
  int dataLen;
  bool Downlink;// Set this flag in
  int getDataLen(){
  if(dataLen < 0)
  std::cout << "Datalen lessthan 0" << dataLen << std::endl;
  return dataLen; }
  char *pay_load;
  uint16_t srcPort;
  uint16_t dstPort;
}m_Packet;


enum Displaylayer { CORE_LAYER =0 , 
       USER_LAYER_LTE,
       
       MAX_LAYER
      };

enum pktDistr { 
       p0to60 =0,
       p61to127,
       p128to255,
       p256to511,
       p512to1023,
       p1024to1513,
       p1514to1523,
       pgt1523,
       maxPktdistri  
      };

#endif // END PACKET_COMMON_H

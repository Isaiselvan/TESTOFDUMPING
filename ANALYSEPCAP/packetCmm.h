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

typedef struct Packet{ // Used as template refer other members to change name
  libtrace_ether_t ethernetlayer;
  libtrace_ip_t  ip4;
  uint8_t * ip_ptr;//For internal purpose. Should be null after flow
  int ipSize; // Added for DPI
  libtrace_ip6_t ipv6; 
  libtrace_udp_t udp;
  libtrace_tcp_t tcp;
  libtrace_ipproto_t type;
  int timeStamp;
  int ipv;
  int dataLen;
  bool Downlink;// Set this flag in
  int getDataLen(){
  if(dataLen < 0)
  std::cout << "Datalen lessthan 0" << dataLen << std::endl;
  return dataLen; }
  char pay_load[128]; // Max snaplen for analysis
}m_Packet;


enum Displaylayer { CORE_LAYER =0 , 
       USER_LAYER_LTE,
       
       MAX_LAYER
      };


#endif // END PACKET_COMMON_H

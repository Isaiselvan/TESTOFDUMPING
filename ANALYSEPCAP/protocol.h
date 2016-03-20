#ifndef PROTOCOL_BASE_H
#define PROTOCOL_BASE_H

#include <iostream>
#include <string>
#include <string.h>
//#include "libtrace.h"
#include <arpa/inet.h>
#include "libtrace_parallel.h"
#include "packetCmm.h"
#include <ctime>



class protocolBase { 

protected: 
  int m_starttime;
  int m_endtime; 
  libtrace_ipproto_t m_protocoltyp;  
public :
   protocolBase(libtrace_ipproto_t type, int start, int end): m_starttime(start),m_endtime(end),m_protocoltyp(type){ };
   virtual  ~protocolBase(){};
   virtual unsigned long int bandWidthCalc() {} ; // TO be pure , But can't use in Map 
   virtual void calculatemetrics() {} ; // "
   virtual void displaymetrics(std::string splunkKey) {} ;  // "
   virtual int addPkt(libtrace_packet_t *, m_Packet&){
     std::cout << "protocol not initialised properly \n" << std::endl;
     }
};

#endif

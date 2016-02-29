#ifndef LTE_GTP_BASE
#define LTE_GTP_BASE
#include <iostream>
#include <string>
#include <string.h>
#include <arpa/inet.h>
#include "libtrace_parallel.h"
#include "packetCmm.h"
#include "protocol.h"

#define MAX_MESSAGE_TYPE 255
//Interested message types 16 - 23 for Control, Data plane 26 and 255


typedef struct gtphrd {
  uint8_t m_version;   // really a 3 uint3_t   82    87   
  bool m_protocolType;   
  bool m_extensionHeaderFlag;  
  bool m_sequenceNumberFlag;
  bool m_nPduNumberFlag; 
  uint8_t m_messageType;  
  uint16_t m_length; 
  uint32_t m_teid; 
  uint16_t m_sequenceNumber; 
  uint8_t m_nPduNumber; 
  uint8_t m_nextExtensionType;   

}GTPhrd;


//Single instance maintanined in Udp protocol 
class LteProtoBase {

   int startTime; 
   int endtime; 
   unsigned long  int  m_totalpkts;
   unsigned long  int  m_totaldata;
   unsigned long  int  m_totaluplink;
   unsigned long  int  m_totaldownlink; 
   unsigned long int m_bandwidth;
   //LTE specific
   int totMess[MAX_MESSAGE_TYPE];
   std::string messType[MAX_MESSAGE_TYPE];
   unsigned long int totalGTP_C; 
   unsigned long int totalGTP_U; 
   unsigned long int totalData_C;
   unsigned long int totalData_U;
   unsigned long int totalError_U; // Message type 26 

//Real user content over lte 
 std::map<std::string, std::map<libtrace_ipproto_t, protocolBase* > > applayerdasB;
 //std::list<GTPhrd> m_pkt; Hold when needed 
 
public :
  int parseGtp(libtrace_packet_t *, GTPhrd * );
  int addPkt(libtrace_packet_t *, GTPhrd, libtrace_ipproto_t);
  protocolBase*  getProtoBase(std::string node, libtrace_ipproto_t);
  int cleardashB(int newtsrtime, int newendtime);
  void printstats(); // Future will be sending to some other module or UI  

};


#endif //LTE_GTP_BASE

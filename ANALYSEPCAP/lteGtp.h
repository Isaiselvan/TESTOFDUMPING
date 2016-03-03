#ifndef LTE_GTP_BASE
#define LTE_GTP_BASE
#include <iostream>
#include <string>
#include <string.h>
#include <arpa/inet.h>
#include "libtrace_parallel.h"
#include "packetCmm.h"
#include "protocol.h"
#include "displayStats.h" 
//#include <map>

#define MAX_MESSAGE_TYPE 256
#define packetCount 0
#define datalen 1

#define	GtpVersion1		 0x01
#define	GTPv1            	 0x01 // Version 1
#define GTPv0                    0x00 // Version 0

//Interested message types 16 - 23 for Control, Data plane 26 and 255
const short validMess[8] = {16,17,18,19,20,21,26,255};

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
  char payload[65000];
  int payLoadLen; 

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
   unsigned long int totalGTP_C[2]; 
   unsigned long int totalGTP_U[2];
   unsigned long int totalError[2]; // Message type 26 

//Real user content over lte 
// std::map<std::string, std::map<libtrace_ipproto_t, protocolBase* > > applayerdasB;
 //std::list<GTPhrd> m_pkt; Hold when needed 
 
public :
  LteProtoBase(int start, int end){
   startTime = start;
   endtime = end; 
   m_totalpkts = 0 ;
   m_totaldata = 0 ; 
   m_totaluplink = 0 ; 
   m_totaldownlink = 0; 
   m_bandwidth = 0 ; 
   bzero(totMess, sizeof(int) * MAX_MESSAGE_TYPE);
   //Initialse interested messages alone
  messType[0x10] ="Create PDP Context Request";// 11
  messType[0x11] ="Create PDP Context Response";
  messType[0x12]= "Update PDP Context Request";
  messType[0x13] = "Update PDP Context Response"; 
  messType[0x14] = "Delete PDP Context Request";
  messType[0x15] = "Delete PDP Context Response";//15
  messType[0x1A] = "Error Indication";//26 
  messType[0xFF] = "G-PDU";//255 
  bzero(totalGTP_C, sizeof(unsigned long int) * 2);
  bzero(totalGTP_U, sizeof(unsigned long int) * 2);
  bzero(totalError, sizeof(unsigned long int) * 2);
  displayStats::getdashB(USER_LAYER_LTE)->cleardashB(start, end);
  }

  ~LteProtoBase(){
   displayStats::getdashB(USER_LAYER_LTE)->cleardashB(0, 0);
   }
  int parseGtp(libtrace_packet_t *, char *);
  int addPkt(libtrace_packet_t *, GTPhrd);
  protocolBase*  getProtoBase(std::string node, libtrace_ipproto_t);
  int cleardashB(int newtsrtime, int newendtime);
  void printstats(); // Future will be sending to some other module or UI  
  unsigned long int bandWidthCalc () ;

};


#endif //LTE_GTP_BASE

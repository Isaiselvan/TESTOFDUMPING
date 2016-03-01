#include "lteGtp.h"


int LteProtoBase::parseGtp(libtrace_packet_t * pkt , libtrace_udp_t * udpPkt)
{
 //Check if the UPD payload is a GTPU
 uint32_t rem =0;
 char *Gtp_ptr= NULL;  
 char gtpHrd[8]; //64 bits
 GTPhrd gtpHeader;

 Gtp_ptr = (char *)trace_get_payload_from_udp (udpPkt,&rem);
 
 
 if(rem < 8 && !Gtp_ptr)
     return -1; 

 memcpy(gtpHrd, Gtp_ptr, 8) ;

  // Check version
     if ((gtpHrd[0] >> 5) != GtpVersion1) // 0-2 version total 8 >> 5  is 1st 3 bits
     return (-1);

 // Check Protocol Type
     if ( ((gtpHrd[0] & 0x10) >> 4) == GTPv1) // 00010000
     {                                     // interested in 4 bit
        gtpHeader.m_version = 1;
     }else if (((gtpHrd[0] & 0x10) >> 4) == GTPv0)
     {
        gtpHeader.m_version = 0;  
     }else 
      return -1; 

    gtpHeader.m_messageType = gtpHrd[1];
   if(gtpHeader.m_messageType >= 16 && gtpHeader.m_messageType <= 21)
   {
     totalGTP_C[0]++;
     totalGTP_C[1] += m_totaldata;
   }else if (gtpHeader.m_messageType == 26)
   {
     totalError[0]++;
     totalError[1] += m_totaldata;
   }else if ( gtpHeader.m_messageType == 255)
   {
    totalGTP_U[0]++;
    totalGTP_U[1] += m_totaldata;
   }else 
   {
    return -1;
   }

 // length 
    gtpHeader.m_length = (gtpHrd[2] << 8) | gtpHrd[3];
    
    // Store the TEID
    gtpHeader.m_teid  = (gtpHrd[4] << 24) | (gtpHrd[5] << 16) |
      				(gtpHrd[6] << 8)  | (gtpHrd[7]);
    
  //Protocol statistics 
   m_totalpkts++;
   m_totaldata += gtpHeader.m_length;
   totMess[gtpHeader.m_messageType]++; //Individual message split up
   //gtpHeader.payload = (void *)Gtp_ptr; 
   memcpy (gtpHeader.payload, (void *)Gtp_ptr,  gtpHeader.m_length);
  if(gtpHeader.m_messageType == 255)// only G-PDU contains some user data 
    addPkt(pkt,gtpHeader); 

 return 1;
}


int LteProtoBase::addPkt(libtrace_packet_t * pkt, GTPhrd gtpHrd)
{
 
//  TRACE_TYPE_ETH  
  libtrace_packet_t nextLprt;

  trace_construct_packet(&nextLprt, TRACE_TYPE_ETH, (void *)gtpHrd.payload, gtpHrd.m_length);

#ifdef USER_TCP_UDP
    {
       displayStats::getdashB(USER_LAYER_LTE)->ParsePkt(&nextLprt); // If it fails or not

    } 
#endif 
 
return 1; 
}

protocolBase* LteProtoBase::getProtoBase(std::string node, libtrace_ipproto_t){

}

int LteProtoBase::cleardashB(int newtsrtime, int newendtime)
{

}

void LteProtoBase::printstats(){

}

#include "lteGtp.h"


int LteProtoBase::parseGtp(libtrace_packet_t * pkt , char * udpPkt)
{
 //Check if the UPD payload is a GTPU
 uint32_t rem =0;
 char *Gtp_ptr= udpPkt;  
 char gtpHrd[8]; //64 bits
 GTPhrd gtpHeader;

 //Gtp_ptr = (char *)trace_get_payload_from_udp (udpPkt,&rem);
 
 
 if(!Gtp_ptr)
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


 // length 
    gtpHeader.m_length = (gtpHrd[2] << 8) | gtpHrd[3];
    
   gtpHeader.m_messageType = gtpHrd[1];
   if(gtpHeader.m_messageType >= 16 && gtpHeader.m_messageType <= 21)
   {
     totalGTP_C[0]++;
     totalGTP_C[1] += gtpHeader.m_length;
   }else if (gtpHeader.m_messageType == 26)
   {
     totalError[0]++;
     totalError[1] += gtpHeader.m_length;
   }else if ( gtpHeader.m_messageType == 255)
   {
    totalGTP_U[0]++;
    totalGTP_U[1] += gtpHeader.m_length;
   }else 
   {
    return -1;
   }
    
    // Store the TEID
    gtpHeader.m_teid  = (gtpHrd[4] << 24) | (gtpHrd[5] << 16) |
      				(gtpHrd[6] << 8)  | (gtpHrd[7]);
    
  //Protocol statistics 
   m_totalpkts++;
   m_totaldata += gtpHeader.m_length;
   totMess[gtpHeader.m_messageType]++; //Individual message split up
   //gtpHeader.payload = (void *)Gtp_ptr;
/*if (Buffer[0] & 0x07)
 * {
 *   // if any one of the flag isset, then min 4 bytes extra header will be present
 *      if (Buffer[0] & 0x02)
 *          NumPresent_ = true;
 *
 *              if (Buffer[0] & 0x01)
 *                     NPduNum
 *
 *                         if (Buffer[0] & 0x04)
 *                                 ExtHdr
 *
 *                                 }
 *
 *                                 if(any)
 *                                 8 + 4
 *
 *                                 if(exthdr)
 *                                 + 4 + 4 */
   gtpHeader.m_extensionHeaderFlag = false; 
   gtpHeader.m_sequenceNumberFlag  = false;
   gtpHeader.m_nPduNumberFlag = false; 
 
   int gtpHr_len = 8;
     
   if(gtpHrd[0] & 0x07)
   {
      gtpHr_len+=4;

     if (gtpHrd[0] & 0x02)
        gtpHeader.m_sequenceNumberFlag  = true;
   
     if (gtpHrd[0] & 0x01)
        gtpHeader.m_nPduNumberFlag = true; 

     if (gtpHrd[0] & 0x04)
        {
        gtpHeader.m_extensionHeaderFlag = true;
        gtpHr_len+=4;
        } 
   
   }   
    

   char *gtp_payload = Gtp_ptr + gtpHr_len; 
   gtpHeader.payLoadLen = gtpHeader.m_length - gtpHr_len;
   memcpy (gtpHeader.payload, (void *)gtp_payload,  gtpHeader.payLoadLen);

  if(gtpHeader.m_messageType == 255)// only G-PDU contains some user data 
    addPkt(pkt,gtpHeader); 

 return 1;
}


int LteProtoBase::addPkt(libtrace_packet_t * pkt, GTPhrd gtpHrd)
{
 
//  TRACE_TYPE_ETH  
  libtrace_packet_t *nextLprt = trace_create_packet();
  if(nextLprt == NULL)
     return -1;
// Commented lines test for dummy packet , user level traffic succeeded 
  //libtrace_linktype_t lnk;
  //uint32_t rem =0 ;
  //char * chekPlod = (char *) trace_get_packet_buffer(pkt, &lnk, &rem); 
  char * GtpPayload = gtpHrd.payload;
  while( gtpHrd.payLoadLen > 40)
  {
  trace_construct_packet(nextLprt, trace_get_link_type(pkt), (void *)GtpPayload, gtpHrd.m_length);  
    //trace_construct_packet(nextLprt, lnk, chekPlod, rem); 

#ifdef USER_TCP_UDP
    
       //libtrace_packet_t * constPktPtr = trace_strip_packet(nextLprt);
      // if(constPktPtr)
      // {
           libtrace_ip_t * IPcheck =   trace_get_ip (nextLprt) ;
  
        if(IPcheck)
        {
           //  static int b = 0; 
           // if ( b != 0 )
            // {
              // b = 0;
            // return 1; 
            //  }    
          // b++;
          std::cout << "\nExtracted the payload successfully\n" <<std::endl;
          //std::cout << "\n PacketPayload " << chekPlod << std::endl;
          //std::cout << "\n\n\n Actual Gtp payload " << GtpPayload  << std::endl;
          displayStats::getdashB(USER_LAYER_LTE)->ParsePkt(nextLprt); // If it fails or not
         break;
        }
      // }

     if(!gtpHrd.m_extensionHeaderFlag)
      break; 

      GtpPayload += 4;
      gtpHrd.payLoadLen -= 4;
#endif 
  }
   trace_destroy_packet(nextLprt); 
return 1; 
}

protocolBase* LteProtoBase::getProtoBase(std::string node, libtrace_ipproto_t){

}

int LteProtoBase::cleardashB(int newtsrtime, int newendtime)
{

}

unsigned long int LteProtoBase::bandWidthCalc () {

         if ((endtime - startTime) < 1)
              return 0;

         m_bandwidth = m_totaldata * 8/ (endtime - startTime) ;
         return (m_bandwidth / 1024 /1024 ); // return in Mbps
}



void LteProtoBase::printstats(){
    if(!m_totalpkts)
     return;
    std::cout << "\n\tLte Metrics \n" << std::endl;
    std::cout << "\nSTARTTIME: " << startTime << "\tENDTIME: " << endtime << std::endl;
    std::cout << "\t total GTP packets :" << m_totalpkts << std::endl;
    std::cout << "\t total length of gtp packet : " << m_totaldata << std::endl;
    std::cout << "\t Avergae Packet size = "<< (m_totalpkts == 0 ? 0: (m_totaldata/m_totalpkts/8)) << "KB"<< std::endl;
    std::cout << "\t Lte Bandwidth usage " << bandWidthCalc() << " Mbps"<< std::endl;
    std::cout << "\t Total control plane packets: " << totalGTP_C[0] << std::endl;
    std::cout << "\t Total G-PDU packets: "  << totalGTP_U[0] << std::endl;
    std::cout << "\t Total control plane datalen: " << totalGTP_C[1] << std::endl;
    std::cout << "\t Total G-PDU datalen: "<< totalGTP_U[1] << std::endl;

    std::cout << "\t Total gtp-u with error code: "<< totalError[1] << std::endl;
       
    //Packet spitup based on message type
    std::cout << "\t Gtpu message type slipt up " << std::endl;
    for(int i = 0; i < 8; i++)
     std::cout << "\t"<< messType[validMess[i]] << " : " << totMess[validMess[i]] << std::endl;
    

   std::cout << "User plane traffic \n" << std::endl;
   displayStats::getdashB(USER_LAYER_LTE)->printstats();
}

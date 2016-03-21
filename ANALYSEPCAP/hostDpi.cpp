#include "hostDpi.h"


int appLayer::processPkt(libtrace_packet_t * pkt, m_Packet& cmPkt)//ndpi_detection_process_packet
{
  
     //struct ndpi_id_struct *src, *dst;
     //struct ndpi_flow_struct *ndpi_flow = NULL;
     ndpi_flow_t dummyFlow;
     //ndpi_protocol Detectedprotocol ;  
     u_int64_t time;
     struct timeval pktTime = trace_get_timeval(pkt);

     time = ((uint64_t) pktTime.tv_sec )* detection_tick_resolution + pktTime.tv_usec / (1000000 / detection_tick_resolution);

     //Allocate mem for ndpi_id_struct
     if((dummyFlow.ndpi_flow = (ndpi_flow_struct *)calloc(1, size_flow_struct)) == NULL) {
        printf("[NDPI] %s(2): not enough memory\n", __FUNCTION__);
        return(-1);
      }
      memset(dummyFlow.ndpi_flow, 0, size_flow_struct);

      if((dummyFlow.src = (ndpi_id_struct *)calloc(1, size_id_struct)) == NULL) {
        printf("[NDPI] %s(3): not enough memory\n", __FUNCTION__);
        return(-1);
      }
      memset(dummyFlow.src, 0, size_id_struct);

      if((dummyFlow.dst = (ndpi_id_struct *) calloc(1, size_id_struct)) == NULL) {
        printf("[NDPI] %s(4): not enough memory\n", __FUNCTION__);
        return(-1);
      }
      memset(dummyFlow.dst,  0, size_id_struct);

      // Fill Flow 
      if(cmPkt.ipv == 4)
       {
        ndpi_iphdr *  iph = (ndpi_iphdr *)cmPkt.ip_ptr;
        if(iph->saddr < iph->daddr ){
              dummyFlow.lower_ip = iph->saddr;
              dummyFlow.upper_ip = iph->daddr;
          }  else
          {
              dummyFlow.lower_ip = iph->daddr;//ndpi_ipv6hdr *
              dummyFlow.upper_ip = iph->saddr;
          }
       }else if(cmPkt.ipv == 6){
         ndpi_ipv6hdr *iph6 = ( ndpi_ipv6hdr *) cmPkt.ip_ptr;
         /*if( iph6->saddr < iph6->daddr){
            dummyFlow.lower_ip = iph6->saddr;
            dummyFlow.upper_ip = iph6->daddr;
         }else {
            dummyFlow.lower_ip = iph6->daddr;
            dummyFlow.upper_ip = iph6->saddr;  
         }*///Support later

       }
 
     dummyFlow.protocol =  cmPkt.type;
     if( cmPkt.type == TRACE_IPPROTO_TCP) 
     {
        if(dummyFlow.lower_ip < dummyFlow.upper_ip)
          {
              dummyFlow.lower_port = cmPkt.tcp.source ;
              dummyFlow.upper_port = cmPkt.tcp.dest;
          }else 
          {
              dummyFlow.lower_port = cmPkt.tcp.dest;
              dummyFlow.upper_port = cmPkt.tcp.source;
          }   

     }else if( cmPkt.type == TRACE_IPPROTO_UDP){
        if(dummyFlow.lower_ip < dummyFlow.upper_ip){
             dummyFlow.lower_port = cmPkt.udp.source;
             dummyFlow.upper_port = cmPkt.udp.dest;
        }else {
             dummyFlow.lower_port = cmPkt.udp.dest;
             dummyFlow.upper_port = cmPkt.udp.source;
        }
     }else {
           dummyFlow.lower_port = 0;
           dummyFlow.upper_port = 0;
     }
  
   

      dummyFlow.detected_protocol = ndpi_detection_process_packet(ndpi_struct, dummyFlow.ndpi_flow, cmPkt.ip_ptr, cmPkt.ipSize, time, dummyFlow.src, dummyFlow.dst);
      //if(dummyFlow.detected_protocol.protocol == NDPI_PROTOCOL_UNKNOWN)
      //dummyFlow.detected_protocol = ndpi_detection_giveup(ndpi_struct, dummyFlow.ndpi_flow);
      
      if(dummyFlow.detected_protocol.protocol == NDPI_PROTOCOL_UNKNOWN)
         dummyFlow.detected_protocol = ndpi_guess_undetected_protocol(ndpi_struct,dummyFlow.protocol, ntohl(dummyFlow.lower_ip), ntohs(dummyFlow.lower_port),                                    ntohl(dummyFlow.upper_ip), ntohs(dummyFlow.upper_port));

      protocol_counter[dummyFlow.detected_protocol.protocol]++;  
      protocol_counter_bytes[dummyFlow.detected_protocol.protocol] += cmPkt.getDataLen(); 
 
     //std::cout << "Check proto " << ndpi_get_proto_name(ndpi_struct, dummyFlow.detected_protocol.protocol) << "cmPkt.ipSize" << cmPkt.ipSize;
     //free_ndpi_flow(dummyFlow); 
     ndpi_free(dummyFlow.ndpi_flow); dummyFlow.ndpi_flow =NULL;
     ndpi_free(dummyFlow.src); dummyFlow.src = NULL;
     ndpi_free(dummyFlow.dst); dummyFlow.dst = NULL;          

}


void appLayer::printStat(std::string &splunk)
{
     for(int i = 0; i <= ndpi_get_num_supported_protocols(ndpi_struct); i++)
        {
           if ( protocol_counter[i] > 0 ){
             std::cout << splunk << " AppLayer= " << ndpi_get_proto_name(ndpi_struct, i) << " Total_pkt="  << protocol_counter[i] << " Total_Datalen=" << protocol_counter_bytes[i]/1024/1024 << std::endl; 
           }   
        }
}



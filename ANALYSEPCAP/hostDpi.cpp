#include "hostDpi.h"


int appLayer::processPkt(libtrace_packet_t * pkt, m_Packet& cmPkt)//ndpi_detection_process_packet
{
  
     struct ndpi_id_struct *src, *dst;
     struct ndpi_flow_struct *ndpi_flow = NULL;
     ndpi_protocol Detectedprotocol ;  
     u_int64_t time;
     struct timeval pktTime = trace_get_timeval(pkt);

     time = ((uint64_t) pktTime.tv_sec )* detection_tick_resolution + pktTime.tv_usec / (1000000 / detection_tick_resolution);

     //Allocate mem for ndpi_id_struct
     if((ndpi_flow = (ndpi_flow_struct *)calloc(1, size_flow_struct)) == NULL) {
        printf("[NDPI] %s(2): not enough memory\n", __FUNCTION__);
        return(-1);
      }

      if((src = (ndpi_id_struct *)calloc(1, size_id_struct)) == NULL) {
        printf("[NDPI] %s(3): not enough memory\n", __FUNCTION__);
        return(-1);
      }

      if((dst = (ndpi_id_struct *) calloc(1, size_id_struct)) == NULL) {
        printf("[NDPI] %s(4): not enough memory\n", __FUNCTION__);
        return(-1);
      }

      Detectedprotocol = ndpi_detection_process_packet(ndpi_struct, ndpi_flow, cmPkt.ip_ptr, cmPkt.ipSize, time, src, dst);

     std::cout << "Check proto " << ndpi_get_proto_name(ndpi_struct, Detectedprotocol.protocol) << "cmPkt.ipSize" << cmPkt.ipSize;

     ndpi_free(ndpi_flow);
     ndpi_free(src);
     ndpi_free(dst);          

}

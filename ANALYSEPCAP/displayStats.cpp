#include "displayStats.h"

  int displayStats::ParsePkt(libtrace_packet_t *pkt) {
      	uint8_t proto;
	uint16_t ethertype;
	uint32_t rem;
	void *ltheader = NULL;
        uint8_t ether_dhost [6] ;
        memcpy(ether_dhost, trace_get_source_mac(pkt),6);
        //ether_dhost = *(trace_get_source_mac(pkt));
 	uint8_t ether_shost [6] ;  
        memcpy(ether_shost,trace_get_destination_mac(pkt), 6);

       	libtrace_ip_t *ip = NULL;
	libtrace_ip6_t *ip6 = NULL;

        ltheader = trace_get_layer3(pkt, &ethertype, &rem);
        if (ltheader == NULL)
		return -1;

	/* If there is no packet remaining, there is no point in going any
         * 	 * further */
	if (rem == 0)
		return -1;

          if (ethertype == TRACE_ETHERTYPE_IP) {
		ip = (libtrace_ip_t *)ltheader;

		ltheader = trace_get_payload_from_ip(ip, &proto, &rem);

	} else if (ethertype == TRACE_ETHERTYPE_IPV6) {
		ip6 = (libtrace_ip6_t *)ltheader;

		ltheader = trace_get_payload_from_ip6(ip6, &proto, &rem);

	} else {
		return -1;
	}
     
       	if (ltheader == NULL)
	return -1;
        
        if (proto == TRACE_IPPROTO_UDP)
	{	
             m_tcpPacket tcppkt; 
        }else
         return -1; // Support for other protocols can be extended..
    
     return 0;    
  }

  template <typename T> 
  int addPkt(T) {
   
  
   return 0; 
  }


  int cleardashB(int newtsrtime, int newendtime){}
  void printstats(){} 

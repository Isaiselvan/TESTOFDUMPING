#include "displayStats.h"

  int displayStats::ParsePkt(libtrace_packet_t *pkt)//Going forward change this function to template and move the code to TcpPacket

    {
      	uint8_t proto;
	uint16_t ethertype;
	uint32_t rem;
	void *ltheader = NULL;
        uint8_t ether_shost [6] ;
        int starttime = 0;
        int pktlen = trace_get_wire_length(pkt); 
        memcpy(ether_shost, trace_get_source_mac(pkt),6);
        //ether_dhost = *(trace_get_source_mac(pkt));
 	uint8_t ether_dhost [6] ;  
        memcpy(ether_dhost,trace_get_destination_mac(pkt), 6);

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
        //Time of packet trace_get_seconds
        starttime = trace_get_seconds(pkt);
        
        if (proto == TRACE_IPPROTO_TCP)
	{	
             m_tcpPacket tcppkt;
             
             //Fill Ethernet details
             memcpy(tcppkt.ethernetlayer.ether_dhost,ether_dhost,6);
             memcpy(tcppkt.ethernetlayer.ether_shost , ether_shost,6);
             tcppkt.ethernetlayer.ether_type = ethertype; 
             
             //Fill Ip Layer 
             tcppkt.ip4 = *ip;
             tcppkt.ipv6 = *ip6;
             
             //Fill Layer4/5 details
             tcppkt.tcp = *((libtrace_tcp_t *)ltheader);

             //Fill the wirelen of packet
             tcppkt.dataLen = pktlen;
        
         //Add pkt to protocolbase 
         if( addPkt(tcppkt, pkt,(libtrace_ipproto_t)proto, starttime) == -1)
          return -1;
  
        }else
         return -1; // Support for other protocols can be extended..
        
         
     return 0;    
  }

  template <typename T> 
  int displayStats::addPkt(T pkt,libtrace_packet_t * ptrpkt, libtrace_ipproto_t prototype, int pktTime) {
        


    char buff[10];
    if(prototype == TRACE_IPPROTO_TCP)
      trace_ether_ntoa (pkt.ethernetlayer.ether_shost, (char *)buff); //else if 
     
    
    std::string node(buff);
 
   // Access to data and Map So we lock
    pthread_mutex_lock(&disLock);
    if ( curIntEndtime!=0 && (curIntEndtime < pktTime))
       {
        printstats();
        cleardashB(pktTime,pktTime+60);
        StatsAvailable = false;      
       }
     
     protocolBase * protoBase = getProtoBase(node, prototype);
    
      if(protoBase == NULL)
         return -1;
       
      if(prototype == TRACE_IPPROTO_TCP)
        { 
         if(((protocolTCP *)protoBase)->addPkt(ptrpkt, pkt) == -1)
         return -1; 
        }
        totaldatalen+=getDataLen(pkt);
        totalpkts+=1; 
  
    pthread_mutex_unlock(&disLock);
   return 0; 
  }

  protocolBase *  displayStats::getProtoBase(std::string node, libtrace_ipproto_t type)
 {
   protocolBase* ret = NULL ;
   std::map<std::string, std::map<libtrace_ipproto_t, protocolBase* > >::iterator it;
   std::map<libtrace_ipproto_t, protocolBase* >::iterator protIt; 
   //std::map<libtrace_ipproto_t, protocolBase* > protocol;
   if(dashboard.size() > 0)
   {
     
        it = dashboard.find(node);
          if(it != dashboard.end())
            {
                  
                  if (it->second.size() > 0)
                       protIt = it->second.find(type); 

                    if(protIt != it->second.end())
                       ret = protIt->second;
            }
      

   }

  if (!ret )
  {
      printf("\n Creating protocolbase st=%d end=%d\n", curIntStarttime,
          curIntEndtime);
      std::map<libtrace_ipproto_t, protocolBase* > protocoltmp; 

      if(type == TRACE_IPPROTO_TCP) 
      protocoltmp[type] = new protocolTCP(curIntStarttime,curIntEndtime);
     // protocolTCP a(curIntStarttime,curIntEndtime);
     // protocoltmp[type] = &a;
      dashboard[node] = protocoltmp;
      ret = getProtoBase(node, type);
  }

  return ret; 
  }
  
  int displayStats::cleardashB(int newtsrtime, int newendtime){
   //clear map and protocol base 
  }
  void displayStats::printstats(){
    //Total display
    std::cout << "Total packts = "<< totalpkts << std::endl; 
    std::cout  << "Total packetlen ="<< totaldatalen << std::endl;
   //Total number of nodes
    std::cout << "Total number of nodes =" << dashboard.size() << std::endl;  
   //Total number of protocols in that node
   std::map<std::string, std::map<libtrace_ipproto_t, protocolBase* > > ::const_iterator it;    
   for (it = dashboard.begin(); it != dashboard.end(); it++)
   {
    std::cout << "Total number of protocols = " << it->second.size();
     std::map<libtrace_ipproto_t, protocolBase* > ::const_iterator itr;
     for (itr = it->second.begin(); itr != it->second.end(); itr++)
          itr->second->displaymetrics();
   }
  } 

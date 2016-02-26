#include "displayStats.h"

const char clr[] = { 27, '[', '2', 'J', '\0' };
const char topLeft[] = { 27, '[', '1', ';', '1', 'H','\0' ,'\n', '\n'};
std::string protcolname[20];

displayStats * displayStats::displayBoard = NULL;


 
  int displayStats::ParsePkt(libtrace_packet_t *pkt)//Going forward change this function to template and move the code to TcpPacket

    {
      	uint8_t proto;
	uint16_t ethertype;
	uint32_t rem;
	void *ltheader = NULL;
        uint8_t ether_shost [6] ;
        int starttime = 0;
        int pktlen =  trace_get_wire_length(pkt); 
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
             if(ip)
             {
             tcppkt.ip4 = *ip;
             tcppkt.ipv = 4;
             } else if(ip6)
             {
             tcppkt.ipv6 = *ip6;
             tcppkt.ipv = 6;
             } 
             
             //Fill Layer4/5 details
             tcppkt.tcp = *((libtrace_tcp_t *)ltheader);

             //Fill the wirelen of packet
             if(pktlen > 0)
             tcppkt.dataLen = pktlen;
             else 
             tcppkt.dataLen = 0;
             
             tcppkt.timeStamp = starttime;
         //Add pkt to protocolbase 
         if( addPkt(tcppkt, pkt,(libtrace_ipproto_t)proto, starttime) == -1)
          return -1;
  
        }else
         return -1; // Support for other protocols can be extended..
        
         
     return 0;    
  }

  template <typename T> 
  int displayStats::addPkt(T pkt,libtrace_packet_t * ptrpkt, libtrace_ipproto_t prototype, int pktTime) {
        
     if ( curIntStarttime > pktTime)
     {
            return -1; 
     }
     
 
   // Access to data and Map So we lock
    pthread_mutex_lock(&disLock);
    if ( curIntEndtime == 0 )
      {
        curIntStarttime = pktTime;
        curIntEndtime = pktTime + TIMEINT; 
      }
    else if (curIntEndtime < pktTime)
      {
        printstats();
        cleardashB(pktTime,pktTime + TIMEINT);
        clearStats();              
    }
    char buff[10];
    
    if(prototype == TRACE_IPPROTO_TCP)
      trace_ether_ntoa (pkt.ethernetlayer.ether_shost, (char *)buff); //else if 
     
    std::string node(buff);
    protocolBase * protoBase = getProtoBase(node, prototype);
    
      if(protoBase == NULL)
         return -1;
       
      if(prototype == TRACE_IPPROTO_TCP)
        { 
         if(((protocolTCP *)protoBase)->addPkt(ptrpkt, pkt) == -1)// addPkt forced function implementation
         return -1; 
        }
        totaldatalen+=getDataLen(pkt); // getDataLen forced function imple
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
     std::map<std::string, std::map<libtrace_ipproto_t, protocolBase* > > ::iterator it;
     std::map<libtrace_ipproto_t, protocolBase* > *innerlayerTmp; 
      for (it = dashboard.begin(); it != dashboard.end(); it++)
   {
     std::map<libtrace_ipproto_t, protocolBase* > ::iterator itr;
     
     for (itr = it->second.begin(); itr != it->second.end(); itr++)
       {
           if(itr->second)
             delete (itr->second);
       }
          
         it->second.clear();//Protocol map
         //innerlayerTmp = &it->second;
         //innerlayerTmp->clear();
   }
     
        dashboard.clear();//Node clear
        curIntStarttime = newtsrtime;
        curIntEndtime = newendtime;
        totalpkts = 0;
        totaldatalen = 0; 

  }
  void displayStats::printstats(){
    int node  = 0;

    std::cout << clr <<  topLeft ; 
    std::cout << "\n\n\nPcap file stats" << std::endl;
    std::cout << "\t| received | accepted | filtered | dropped | captured | error |" << std::endl;

    std::cout << "\t| " << pcapStats.received<< "\t| "<< pcapStats.accepted << "\t| " << pcapStats.filtered << "\t| " << pcapStats.dropped << "\t| " << pcapStats.captured << "\t| " << pcapStats.errors << "\t|" << std::endl; 
    //Total display
    std::cout << "Total packts = "<< totalpkts << std::endl; 
    std::cout  << "Total packetlen = "<< totaldatalen << std::endl;
    std::cout << "Avergae Packet size = "<< (totalpkts == 0 ? 0: (totaldatalen/totalpkts/8)) << "KB"<< std::endl;
   
   //Total number of nodes
    std::cout << "Total number of nodes =" << dashboard.size() << std::endl;  
   
   //Total number of protocols in that node
   std::map<std::string, std::map<libtrace_ipproto_t, protocolBase* > > ::const_iterator it;    
   for (it = dashboard.begin(); it != dashboard.end(); it++)
   {
    std::cout << "\nNode " << ++node << " :" << it->first << std::endl; 
    std::cout << "Total number of protocols = " << it->second.size();
    std::cout << "\n\n" ; 
     std::map<libtrace_ipproto_t, protocolBase* > ::const_iterator itr;
     for (itr = it->second.begin(); itr != it->second.end(); itr++)
         {
          std::cout << "\nProtocol : " << protcolname[itr->first] <<  std::endl;
          itr->second->displaymetrics();
         }
   }
} 

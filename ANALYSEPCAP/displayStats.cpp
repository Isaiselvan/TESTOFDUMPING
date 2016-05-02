//#include <algorithm>

#include "displayStats.h"
#include "tcpProto.h"
#include "udpProto.h"

const char clr[] = { 27, '[', '2', 'J', '\0' };
const char topLeft[] = { 27, '[', '1', ';', '1', 'H','\0' ,'\n', '\n'};
pthread_mutex_t displayStats::disLock[MAX_LAYER];
//displayStats * diBoard[] = {NULL,NULL};
//displayStats * displayStats::displayBoard = diBoard;
displayStats * displayStats::displayBoard[MAX_LAYER] = {NULL, NULL};

//std::fill_n(displayStats * displayStats::displayBoard, NULL, MAX_LAYER);


 
  int displayStats::ParsePkt(libtrace_packet_t *pkt)//Going forward change this function to template and move the code to TcpPacket

    {
      	uint8_t proto;
	uint16_t ethertype;
	uint32_t rem;
	void *ltheader = NULL;
        uint8_t ether_shost [6] ;
        int starttime = 0;
        int pktlen =  trace_get_wire_length(pkt);
        int ipSize =0; 
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
        ipSize = rem;  
        
         if (ethertype == TRACE_ETHERTYPE_IP) {
		ip = (libtrace_ip_t *)ltheader;

		//ltheader = trace_get_payload_from_ip(ip, &proto, &rem);
                //  ltheader = trace_get_transport(pkt, &proto, &rem);
	} else if (ethertype == TRACE_ETHERTYPE_IPV6) {
		ip6 = (libtrace_ip6_t *)ltheader;

		//ltheader = trace_get_payload_from_ip6(ip6, &proto, &rem);
                //  ltheader = trace_get_transport(pkt, &proto, &rem);
	} else {
		return -1;
	}
        
        rem = 0;
        ltheader = trace_get_transport(pkt, &proto, &rem);
       	if (ltheader == NULL || rem == 0)
	return -1;
        //Time of packet trace_get_seconds
        starttime = trace_get_seconds(pkt);
        
		
             m_Packet ppkt;
             
             //Fill Ethernet details
             memcpy(ppkt.ethernetlayer.ether_dhost,ether_dhost,6);
             memcpy(ppkt.ethernetlayer.ether_shost , ether_shost,6);
             ppkt.ethernetlayer.ether_type = ethertype; 
             
             //Fill Ip Layer 
             if(ip)
             {
             ppkt.ip4 = *ip;
             ppkt.ip_ptr = (uint8_t *)ip;
             ppkt.ipv = 4;
             } else if(ip6)
             {
             ppkt.ipv6 = *ip6;
             ppkt.ip_ptr = (uint8_t *)ip6;
             ppkt.ipv = 6;
             } 
             //
             ppkt.ipSize = ipSize;
             //Fill the wirelen of packet
             if(pktlen > 0)
             ppkt.dataLen = pktlen;
             else 
             ppkt.dataLen = 0;
             
             ppkt.timeStamp = starttime;
             
             ppkt.type = (libtrace_ipproto_t)proto;
        
        if (proto == TRACE_IPPROTO_TCP)
        {
             //Fill Layer4/5 details
             ppkt.tcp = *((libtrace_tcp_t *)ltheader);
             char *tcp_payload;
             void *transport  ;
             //rem = 0;
             libtrace_udp_t *tcp;
             tcp = (libtrace_udp_t *)ltheader;
             //transport = trace_get_transport(

             tcp_payload = (char *)trace_get_payload_from_udp(tcp,&rem);

             if(tcp_payload && rem != 0)
             memcpy (ppkt.pay_load, tcp_payload, sizeof(ppkt.pay_load));

  
        }else if (proto == TRACE_IPPROTO_UDP)
        {
             
             //Fill Layer4/5 details
             ppkt.udp = *((libtrace_udp_t *)ltheader);
             char *udp_payload;
             void *transport  ;
             //rem = 0; 
             libtrace_udp_t *udp;
             udp = (libtrace_udp_t *)ltheader;
             //transport = trace_get_transport(

             udp_payload = (char *)trace_get_payload_from_udp(udp,&rem);

             if(udp_payload && rem != 0) 
             memcpy (ppkt.pay_load, udp_payload, sizeof(ppkt.pay_load));

        }else 
         return -1; // Support for other protocols can be extended..
        
         //Add pkt to protocolbase 
         if( addPkt(ppkt, pkt,(libtrace_ipproto_t)proto, starttime) == -1)
          return -1;
         
     return 0;    
  }

   
  int displayStats::addPkt(m_Packet pkt,libtrace_packet_t * ptrpkt, libtrace_ipproto_t prototype, int pktTime) {
        
     if ( curIntStarttime > pktTime)
     {
            return -1; 
     }
     
 
   // Access to data and Map So we lock
    pthread_mutex_lock(&disLock[layer]);
    {
    if ( curIntEndtime == 0 )
      {
        curIntStarttime = pktTime;
        curIntEndtime = pktTime + TIMEINT; 
      }
    else if (curIntEndtime < pktTime)
      {
        printstats();
        displayStats::getdashB(USER_LAYER_LTE)->cleardashB(0, 0); // Find a correct location later
        cleardashB(pktTime,pktTime + TIMEINT);
        clearStats();              
      }
    char buff[10];
    char addrstr_src[INET_ADDRSTRLEN];
    std::string ip , dstip;
    if(pkt.ipv == 4)
    {
      //inet_ntop(AF_INET, &(pkt.ip4.ip_src), (char *)&addrstr_src, INET_ADDRSTRLEN);
       ip = inet_ntoa((pkt.ip4.ip_src));
       dstip = inet_ntoa((pkt.ip4.ip_dst));
       //std::cout << "DEV: IP=" << ip << std::endl;
       if(pkt.ip4.ip_src.s_addr  > pkt.ip4.ip_dst.s_addr )
           dstip = ip +"-"+ dstip;
       else
           dstip = dstip +"-"+ip;
    }
    else
    {
      inet_ntop(AF_INET, &(pkt.ipv6.ip_src), (char *)&addrstr_src, INET_ADDRSTRLEN);
       ip = addrstr_src;
      inet_ntop(AF_INET, &(pkt.ipv6.ip_dst), (char *)&addrstr_src, INET_ADDRSTRLEN);
       //ip = inet_ntoa((pkt.ipv6.ip_src));
       dstip = addrstr_src;
       if(pkt.ip4.ip_src.s_addr  > pkt.ip4.ip_dst.s_addr )
           dstip = ip +"-"+ dstip;
       else
           dstip = dstip +"-"+ip;
    }


      trace_ether_ntoa (pkt.ethernetlayer.ether_shost, (char *)buff); //else if 
     
     std::string node(buff);

     node = node + " IP=" + ip + " DSTIP=" + dstip;

    if(layer != CORE_LAYER)
    {
       node = "";  
       node = node + "UserTraffic NODEIP=" + ip + " DSTIP=" + dstip;
    }

    protocolBase * protoBase = getProtoBase(node, prototype);
    
      if(protoBase == NULL)
      {
       pthread_mutex_unlock(&disLock[layer]);
       return -1;
      }

      if(protoBase->addPkt(ptrpkt, pkt) == -1)// addPkt forced function implementation
      {
       pthread_mutex_unlock(&disLock[layer]);
       return -1; 
      }

        totaldatalen+=pkt.getDataLen(); // getDataLen forced function imple
        totalpkts+=1;
        fillPktDist(pkt.getDataLen());
                  
    }
    pthread_mutex_unlock(&disLock[layer]);

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
                       ret = protIt->second; // Only return this 
            }
      

   }

  if (!ret )
  {
      //printf("\n Creating protocolbase st=%d end=%d\n", curIntStarttime,
       //   curIntEndtime);
      std::map<libtrace_ipproto_t, protocolBase* > protocoltmp; 

      if(type == TRACE_IPPROTO_TCP) 
      protocoltmp[type] = new protocolTCP(curIntStarttime,curIntEndtime);
      else if (type == TRACE_IPPROTO_UDP)
      protocoltmp[type] = new protocolUDP(curIntStarttime,curIntEndtime);  
     // protocolTCP a(curIntStarttime,curIntEndtime);
     // protocoltmp[type] = &a;
      if(it != dashboard.end())
      {
//       std::map<libtrace_ipproto_t, protocolBase* > *prtProto = &dashboard[node];
         dashboard[node].insert(std::pair<libtrace_ipproto_t, protocolBase*>(type,protocoltmp[type])); 
//       prtProto = protocoltmp[type]; 
      }else
      {
       std::cout << "\nCreating new Node \n"  << std::endl;
       dashboard[node] = protocoltmp;
      }
 
        
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
             delete (itr->second); //Protocol delete
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
        for (int count = 0; count <= maxPktdistri; count++)
         pktdist[count] = 0;

  }
  void displayStats::printstats(){
    int node  = 0;

    if(layer == CORE_LAYER)
    { 
    std::cout << clr <<  topLeft ; 
    std::cout << "\n\n\nPcap file stats" << std::endl;
    std::cout << "\t| received | accepted | filtered | dropped | captured | error |" << std::endl;

    std::cout << "\t| " << pcapStats.received<< "\t| "<< pcapStats.accepted << "\t| " << pcapStats.filtered << "\t| " << pcapStats.dropped << "\t| " << pcapStats.captured << "\t| " << pcapStats.errors << "\t|" << std::endl; 
    }
    if(!totalpkts)
       return;
    //Total display
    std::cout << "Total packts = "<< totalpkts << std::endl; 
    std::cout << "Total packetlen = "<< totaldatalen << std::endl;
    std::cout << "Avergae Packet size = "<< (totalpkts == 0 ? 0: (totaldatalen/totalpkts/8)) << "KB"<< std::endl;
    time_t curT = curIntStarttime; 
    struct tm * curTimeInfo;
    char TimeBuf[300];
    curTimeInfo = localtime(&curT);
    strftime(TimeBuf, 100, "%F  %T", curTimeInfo);
    std::string curTime(TimeBuf);
    //std::cout << "<Splunk>;" << "<" << curTime << ">;Total_packets=" << totalpkts << std::endl;
    //std::cout << "<Splunk>;" << "<" << curTime << ">;Total_len=" << totaldatalen << std::endl;
    //std::cout << "<Splunk>;" << "<" << curTime << ">;Toatl_Node=" << dashboard.size() << std::endl;
      
   //Total number of nodes
    std::cout << "Total number of nodes =" << dashboard.size() << std::endl;  
   
   //Total number of protocols in that node
   std::map<std::string, std::map<libtrace_ipproto_t, protocolBase* > > ::const_iterator it;    
   for (it = dashboard.begin(); it != dashboard.end(); it++)
   {
    std::cout << "\nNode " << ++node << " :" << it->first << std::endl; 
    //std::cout << "<Splunk>;" << "<" << curTime << ">;<Node"<<it->first<<">;";
    std::cout << "Total_protocols = " << it->second.size();
    std::cout << "\n\n" ; 
     std::map<libtrace_ipproto_t, protocolBase* > ::const_iterator itr;
     for (itr = it->second.begin(); itr != it->second.end(); itr++)
         {
          sprintf(TimeBuf, "Splunk %s node=%d Mac=%s Layer=%d TransPotocol=%s"
           , curTime.c_str(),node - 1, it->first.c_str(), layer, protcolname[itr->first].c_str());
          std::cout << "\nProtocol : " << protcolname[itr->first] <<  std::endl;
          itr->second->displaymetrics((std::string)TimeBuf);
         }
   }
   if(layer == CORE_LAYER)
   {
   std::cout << "User plane traffic \n" << std::endl;
   displayStats::getdashB(USER_LAYER_LTE)->printstats();
   }
  sprintf(TimeBuf, "Splunk %s",curTime.c_str());
  std::cout << TimeBuf << " DistPkt=0to60 DistCount=" <<pktdist[p0to60] << std::endl;
  std::cout << TimeBuf << " DistPkt=61to127 DistCount=" <<pktdist[p61to127]<< std::endl;
  std::cout << TimeBuf << " DistPkt=128to255 DistCount=" <<pktdist[p128to255]<< std::endl;
  std::cout << TimeBuf << " DistPkt=256to511 DistCount=" <<pktdist[p256to511]<< std::endl; 
  std::cout << TimeBuf << " DistPkt=512to1023 DistCount=" <<pktdist[p512to1023]<< std::endl; 
  std::cout << TimeBuf << " DistPkt=1024to1513 DistCount=" <<pktdist[p1024to1513]<< std::endl; 
  std::cout << TimeBuf << " DistPkt=1514to1523 DistCount=" <<pktdist[p1514to1523]<< std::endl; 
  std::cout << TimeBuf << " DistPkt=gt1523 DistCount=" <<pktdist[pgt1523]<< std::endl; 
} 

int displayStats::fillPktDist(unsigned long int size)
{
   if(size < 61)
   {pktdist[p0to60]++; return p0to60;} 
   else if(size < 128)
   {pktdist[p61to127]++; return p61to127;}
   else if(size < 256)
   {pktdist[p128to255]++; return p128to255;}
   else if(size < 512)
   {pktdist[p256to511]++; return p256to511;}
   else if(size < 1024)
   {pktdist[p512to1023]++; return p512to1023;}
   else if(size < 1514)
   {pktdist[p1024to1513]++; return p1024to1513;}
   else if(size < 1523)
   {pktdist[p1514to1523]++; return p1514to1523;}
   else 
   {pktdist[pgt1523]; return pgt1523;}
}

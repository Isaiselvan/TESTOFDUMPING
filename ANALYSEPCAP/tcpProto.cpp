#include "tcpProto.h"
#include "Interface.h"
#include "GxInterface.h"
#include "GyInterface.h"
#include "S6bInterface.h"
#include <time.h>
#include <unordered_map>
#include <sstream>

std::unordered_map <std::string, GxInterface  * > GxMap;
std::unordered_map <std::string, GyInterface  * > GyMap;
std::unordered_map <std::string, S6BInterface * > S6bMap;

int protocolTCP::addPkt(libtrace_packet_t *pkt, m_Packet *tcppkt)
{
 //DownLink flag can be set here to tcppkt
     m_totalpkts++;
     m_totaldata+=tcppkt->getDataLen();

     switch(tcppkt->ethernetlayer.ether_type)
     {
         case TRACE_ETHERTYPE_IP:
             m_totalipv4++;
         break;
         case TRACE_ETHERTYPE_IPV6:
             m_totalip6++;
         break;
         default:
         break;
     }

     /*
     if( tcppkt->ethernetlayer.ether_type == TRACE_ETHERTYPE_IP)
     m_totalipv4++;
     else if ( tcppkt->ethernetlayer.ether_type == TRACE_ETHERTYPE_IPV6 )
     m_totalip6++;
     */

     if(!trace_get_direction (pkt))
     {
       m_totaldownlink+=tcppkt->getDataLen();
       tcppkt->Downlink = true; 
     }else 
     {
      m_totaluplink+=tcppkt->getDataLen();
      tcppkt->Downlink = false;
     }
     
      
     //layerSeven.processPkt(pkt, *tcppkt);  //ABHINAY
     //m_pkt.push_back(tcppkt);
    
     // Add session logic
     if(tcppkt->srcPort == 3868 || tcppkt->dstPort == 3868)
     {
         Diameter dPkt(tcppkt->pay_load);
         dPkt.timeStamp = tcppkt->timeStamp;
         //dPkt.printPkt();
         //
    std::string ip , dstip, node;
    switch (tcppkt->ipv) 
    {
        case 4:
          
         ip = inet_ntoa((tcppkt->ip4.ip_src));
         dstip = inet_ntoa((tcppkt->ip4.ip_dst));
         if(dPkt.request == 1) 
          node = ip +"-"+ dstip;      
         else
          node = dstip +"-"+ip;
        break;

        case 6:

         char addrstr[INET6_ADDRSTRLEN];
         inet_ntop(AF_INET6, &(tcppkt->ipv6.ip_src), addrstr, INET6_ADDRSTRLEN);
         ip = addrstr;
         inet_ntop(AF_INET6, &(tcppkt->ipv6.ip_dst), addrstr, INET6_ADDRSTRLEN);
         dstip = addrstr; 
         if(dPkt.request == 1)
          node = ip +"-"+ dstip;
         else
          node = dstip +"-"+ip;
          break;

         default :

           return -1; 
    }                                                                                                                                                
         Interface *interface = getInterface(dPkt, node);
         if(interface == NULL)
             return -1;

         switch(interface->checkTime(dPkt.timeStamp))
         {
             case 0:
                 break;
             case 1:
                 interface->addPkt(dPkt);
                 break;
             case 2:
                 std::string src, dst;
                 std::stringstream split(node);
                 getline(split, src, '-');
                 getline(split, dst, '-');
                 node = "sip=" + src + " dip=" + dst;
                 interface->printStats(node);
                 interface->clearStats();
                 interface->addPkt(dPkt);
                 break;
         }
     }
     
}

Interface* protocolTCP::getInterface(Diameter dPkt, std::string &nodeip)
{
  GxInterface  *gxInterface  = NULL;
  GyInterface  *gyInterface  = NULL;
  S6BInterface *s6bInterface = NULL;
    switch(dPkt.appId)
    {
        case GX:
           //std::unordered_map<std::string,double>::const_iterator got = GxMap.find (nodeip);

            //if ( got == GxMap.end() )
              gxInterface = GxMap[nodeip];// =  new GxInterface;
            //else
              if(!gxInterface)
                {
                gxInterface = new GxInterface(nodeip); //gxInterface = my_map[nodeip];
                //std::cout << "Creating new node " << nodeip << std::endl; 
                GxMap[nodeip] = gxInterface;
                }
//Distinct HOH TEST starts here
	      static uint64_t reqHopid= 0;
	      static uint64_t resHopid= 100000;

	      if(reqHopid == 100000)
		      reqHopid = 0;

	      if(resHopid == 0)
		      resHopid = 100000;

	      if(dPkt.request)
	      {
			      dPkt.hopIdentifier = ++reqHopid;
	      }
	      else
	      {
			      dPkt.hopIdentifier = resHopid--;
	      }

//Distinct HOH TEST ends here

            return gxInterface;
            break; 

        case GY:
            gyInterface = GyMap[nodeip];
            if(!gyInterface)
                {
                gyInterface = new GyInterface; //gxInterface = my_map[nodeip];
                GyMap[nodeip] = gyInterface;
                }
          
//Distinct HOH TEST starts here
	      static uint64_t yreqHopid= 0;
	      static uint64_t yresHopid= 10000;

	      if(yreqHopid == 10000)
		      yreqHopid = 0;

	      if(yresHopid == 0)
		      yresHopid = 10000;

	      if(dPkt.request)
	      {
			      dPkt.hopIdentifier = ++yreqHopid;
	      }
	      else
	      {
			      dPkt.hopIdentifier = yresHopid--;
	      }

//Distinct HOH TEST ends here

            return gyInterface;
            break; 
        case S6B:
            s6bInterface = S6bMap[nodeip]; 
            if(!s6bInterface)
                {
                 s6bInterface = new S6BInterface; //gxInterface = my_map[nodeip];
                 S6bMap[nodeip] = s6bInterface;
                }
//Distinct HOH TEST starts here
	      static uint64_t sreqHopid= 0;
	      static uint64_t sresHopid= 10000;

	      if(sreqHopid == 10000)
		      sreqHopid = 0;

	      if(sresHopid == 0)
		      sresHopid = 10000;

	      if(dPkt.request)
	      {
			      dPkt.hopIdentifier = ++sreqHopid;
	      }
	      else
	      {
			      dPkt.hopIdentifier = sresHopid--;
	      }

//Distinct HOH TEST ends here
 
            return s6bInterface;
            break;

        default:
            return NULL; 
    }
}

int protocolTCP::addSession(libtrace_packet_t *pkt,m_Packet tcppkt)
{
  m_tcpSession frameSession; 
  char addrstr_src[INET_ADDRSTRLEN];  
  char addrstr_dst[INET_ADDRSTRLEN];
             if(tcppkt.ipv == 4)
             {
             inet_ntop(AF_INET, &(tcppkt.ip4.ip_src), addrstr_src, INET_ADDRSTRLEN);
             inet_ntop(AF_INET, &(tcppkt.ip4.ip_dst), addrstr_dst, INET_ADDRSTRLEN);
             }
             else 
             {
             inet_ntop(AF_INET, &(tcppkt.ipv6.ip_src), addrstr_src, INET_ADDRSTRLEN);
             inet_ntop(AF_INET, &(tcppkt.ipv6.ip_dst), addrstr_dst, INET_ADDRSTRLEN);
             }

             frameSession.scrIpPort = (std::string)addrstr_src ;
             frameSession.src = tcppkt.tcp.source;   
             frameSession.destIpPort = (std::string)addrstr_dst ;
             frameSession.dst = tcppkt.tcp.dest;
             
          
             if(m_session.size() == 0 ) 
             {
              //Add time   
               m_session.push_back(frameSession);
             } 
             else 
             {

             }  
// Iterator for session
// Deep Packet inspection
             
 return 0;
}

unsigned long int protocolTCP::bandWidthCalc () {
         m_bandwidth = m_totaldata * 8/ (m_endtime - m_starttime) ;
         return m_bandwidth;
}

void protocolTCP::calculatemetrics() {

}

void protocolTCP::displaymetrics(std::string splunkkey) {
    std::cout << "\nTcp layer Metrics" << std::endl;
    std::cout << "\nSTARTTIME: " << m_starttime << "\tENDTIME: " << m_endtime << std::endl;
    std::cout << "Tcp packet total count = " << m_totalpkts << std::endl;
    std::cout << "Total Ipv4 packets = " << m_totalipv4 << std::endl;
    std::cout << "Total Ipv6 packets = " << m_totalip6 << std::endl;
    std::cout << "Tcp data usage = " << m_totaldata/1024/1024 << "MB" << std::endl;    
    std::cout << "Number of distincted sessions " << m_totalSession << std::endl;
    std::cout << "BandWidth usage " << (bandWidthCalc () / 1024 /1024 ) << "Mbps" << std::endl;
    std::cout << "Total Uplink data = " << (m_totaluplink / 1024/1024) << "MB" << std::endl  ;
    std::cout << "Total Downlink data = " << (m_totaldownlink/1024/1024) << "MB\n" << std::flush  ;

   std::cout << splunkkey << " Total_pkt=" << m_totalpkts << " Total_Datalen=" << m_totaldata 
               << " BandWidth=" << (bandWidthCalc ()) << " Total_Uplink=" 
                << (m_totaluplink ) << " Total_DoLink=" << (m_totaldownlink) << 
                 " Ipv4=" << m_totalipv4 << " Ipv6=" << m_totalip6 << std::endl;

  //layerSeven.printStat(splunkkey);
}

int protocolTCP::getPercUplink(){}

int protocolTCP::getPercDownLink(){}


#include "tcpProto.h"
#include "Interface.h"
#include "GxInterface.h"
#include "GyInterface.h"
#include "S6bInterface.h"
#include <time.h>
#include <fstream>


static  GxInterface  *gxInterface  = NULL;
static  GyInterface  *gyInterface  = NULL;
static  S6BInterface *s6bInterface = NULL;
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
         
      /*static FILE *f = NULL;
        if (f == NULL && (f = fopen("MetricsDump.log", "a+") == NULL);
        {
                printf("Error opening file!\n");
                exit(1);
        }*/

      //if(dPkt.request > 1)
          //return -1;
    char addrstr_src[INET6_ADDRSTRLEN];  
    char addrstr_dst[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &(tcppkt->ipv6.ip_src), addrstr_src, INET6_ADDRSTRLEN);
    inet_ntop(AF_INET6, &(tcppkt->ipv6.ip_dst), addrstr_dst, INET6_ADDRSTRLEN);
    static std::ofstream logger1("MetricsDump.log");
    time_t curT = dPkt.timeStamp; 
    struct tm * curTimeInfo;
    char TimeBuf[300];
    curTimeInfo = localtime(&curT);
    strftime(TimeBuf, 100, "%F  %T", curTimeInfo);
    std::string curTime(TimeBuf);
    std::string apid("UNK");
     
     switch(dPkt.appId)
    {
        case GX:
            apid = "GX";
            break;
        case GY:
            apid = "GY";
            break;
        case S6B:
            apid = "S6B";
            break;
        default:
            return -1;
    }

         logger1 << curTime  << " src_ip=" << addrstr_src << " dst_ip=" << addrstr_dst << " CC=" << dPkt.cc   << " Req_ans=" << dPkt.request <<   " appId=" << apid << " HOH=" << dPkt.hopIdentifier << " resCode=" << dPkt.resCode << " reqtype=" << dPkt.reqType << std::endl;    
        // fprintf(f, "Splunk time=%d CC=%d REPT=%d appId=%d HOH=%d resCode=%d reqtype=%d \n",  dPkt.timeStamp, dPkt.cc, dPkt.commandFlag, dPkt.appId, dPkt.hopIdentifier, dPkt.resCode, dPkt.reqType );

         return 1;
         //dPkt.printPkt();
         //Interface *interface = getInterface(dPkt);
         /*if(interface == NULL)
             return -1;

         switch(interface->checkTime(dPkt.timeStamp))
         {
             case 0:
                 break;
             case 1:
                 //interface->addPkt(dPkt);
                 break;
             case 2:
                 //interface->printStats();
                 //interface->clearStats();
                 //interface->addPkt(dPkt);
                 break;
         }*/
     }
     
}

Interface* protocolTCP::getInterface(Diameter dPkt)
{
    switch(dPkt.appId)
    {
        case GX:
            if(gxInterface == NULL)
            {
                gxInterface = new GxInterface;
                std::cout << "ABHINAY:: GxInterface is created." << std::endl;
            }
            return gxInterface;
            break; 

        case GY:
            if(gyInterface == NULL)
            {
                std::cout << "ABHINAY:: GyInterface is created." << std::endl;
                gyInterface = new GyInterface;
            }
            return gyInterface;
            break; 
        case S6B:
            if(s6bInterface == NULL)
            {
                std::cout << "ABHINAY:: S6bInterface is created." << std::endl;
                s6bInterface = new S6BInterface;
            }
            return s6bInterface;
            break;

        default:
            return NULL; 
    }
}

int protocolTCP::addSession(libtrace_packet_t *pkt,m_Packet tcppkt)
{
  m_tcpSession frameSession; 
  char addrstr_src[INET6_ADDRSTRLEN];  
  char addrstr_dst[INET6_ADDRSTRLEN];
             if(tcppkt.ipv == 4)
             {
             inet_ntop(AF_INET, &(tcppkt.ip4.ip_src), addrstr_src, INET_ADDRSTRLEN);
             inet_ntop(AF_INET, &(tcppkt.ip4.ip_dst), addrstr_dst, INET_ADDRSTRLEN);
             }
             else 
             {
             inet_ntop(AF_INET6, &(tcppkt.ipv6.ip_src), addrstr_src, INET6_ADDRSTRLEN);
             inet_ntop(AF_INET6, &(tcppkt.ipv6.ip_dst), addrstr_dst, INET6_ADDRSTRLEN);
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


#include "GyInterface.h"
#include <unordered_map>
#include <string>
#include <time.h>

GyInterface::GyInterface()
{
    memset(&GyStats,0,sizeof(CCGyStats));
    startTime = 0;
    endTime   = 0;
    reqtype   = 0;
    TS        = 0;
    RTT       = 0; 
}

int GyInterface::addPkt(Diameter &pkt)
{
    if(pkt.cc != CCRorA)
    {
        return 1;
    }

    reqtype = pkt.reqType;
    switch(reqtype)
    {
       case INITIAL:
           break;
       case UPDATE:
           break;
       case TERMINATE:
           break;
       case EVENT:
           break;
       default :
           return 1;
    }

    switch(pkt.request)
    {
        case 1:
            /* Handle Request */
            req[reqtype][pkt.hopIdentifier] = pkt.timeStamp;
            break;

        case 0:
           // Handle response
            TS = req[reqtype][pkt.hopIdentifier];
            if(TS == 0)
            {
                 GyStats.unKnwRes[reqtype-1]++;
                 return 0;
            }

            // Increment attempts if a req is found for res 
            GyStats.attempts[reqtype-1]++;

            // Calculate latency
            RTT = pkt.timeStamp - TS;
            if(RTT < 0 || RTT > 40)
            {
                 GyStats.timeoutCount[reqtype-1]++;
                 return 0;
            }
            GyStats.latency[reqtype-1] += RTT;
            GyStats.latencySize[reqtype-1]++;

           if(pkt.resCode < 3000 || pkt.resCode == 70001)
           {
               GyStats.succCount[reqtype-1]++;
           }
           else
           {
                GyStats.failCount[reqtype-1]++;
           }

           req[reqtype].erase(pkt.hopIdentifier);
           break;

        default:
            return 1;
    }
    return 0;
}

void GyInterface::printStats(std::string &node)
{
    curT = startTime;

    /* Calculate Time out requests */
    reqIt = req.begin();
    while(reqIt != req.end())
    {
        reqTmp =&(reqIt->second);
        reqIt1 = reqTmp->begin();
        while(reqIt1 != reqTmp->end())
        {
            if(reqIt1->second + DIAMETER_TIMEOUT < endTime)
            {
                GyStats.timeoutCount[(reqIt->first)-1]++;
                GyStats.attempts[(reqIt->first)-1]++;
                reqTmp->erase(reqIt1++);
            }
            else
            {
                reqIt1++;
            }
        }
        reqIt++;
    }

    /* Print Stats */
    curTimeInfo = localtime(&curT);
    strftime(TimeBuf, 100, "%F  %T", curTimeInfo);
    std::string curTime(TimeBuf);

    for(int i=INITIAL; i<=EVENT; i++)
    {
        std::string msgType;
        switch(i)
        {
            case INITIAL:
                msgType = "INITIAL";
                break;
            case UPDATE:
                msgType = "UPDATE";
                break;
            case TERMINATE:
                msgType = "TERMINATE";
                break;
            case EVENT:
                msgType = "EVENT";
                break;
        }

        std::cout << curTime << " Ip=" << node <<   " Ix=" << "Gy"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Att"  
                                                          << " Kpv=" << GyStats.attempts[i-1]     << std::endl;

        std::cout << curTime << " Ip=" << node <<   " Ix=" << "Gy"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Suc"
                                                          << " Kpv="  << GyStats.succCount[i-1]     << " " << std::endl;

       std::cout << curTime << " Ip=" << node <<   " Ix=" << "Gy"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Fail"
                                                          << " Kpv="      << GyStats.failCount[i-1]    << " " << std::endl;
       std::cout << curTime << " Ip=" << node <<   " Ix=" << "Gy"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Tout"
                                                          << " Kpv="   << GyStats.timeoutCount[i-1] << " " << std::endl;

       if(GyStats.latencySize[i-1] > 0)
       {
           std::cout << curTime << " Ip=" << node <<   " Ix=" << "Gy"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Laty"
                                                          << " Kpv=" << (int) ((GyStats.latency[i-1]/GyStats.latencySize[i-1])*1000) << std::endl; 
       }
    }

}

void GyInterface::clearStats()
{
    memset(&GyStats,0,sizeof(CCGyStats));
    //req.clear();
    //res.clear();
}

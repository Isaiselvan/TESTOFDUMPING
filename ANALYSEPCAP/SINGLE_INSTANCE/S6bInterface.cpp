#include <unordered_map>
#include <string>
#include <time.h>
#include "S6bInterface.h"

S6BInterface::S6BInterface(std::string &nodepair)
{
    memset(&s6bStats,0,sizeof(s6bStats));
    startTime = 0;
    endTime   = 0;
    TS        = 0;
    RTT       = 0;
}

int S6BInterface::addPkt(Diameter &pkt)
{
    msgType = DEFAULT;  

    switch(pkt.cc)
    {
        case S6BAA:
            msgType = AA;
            break;
        case S6BTERMINATE:
            msgType = TERM;
            break;
        default:
            return 1;
    }

    switch(pkt.request)
    {
        case 1:
            /* Handle Request */
            req[msgType][pkt.hopIdentifier] = pkt.timeStamp;
            break;

        case 0:
            /* Handle Response */
            TS = req[msgType][pkt.hopIdentifier];
            if(TS == 0)
            {
                 s6bStats.unKnwRes[msgType]++;
                 return 0;
            }

            // Increment attempts if a req is found for res 
            s6bStats.attempts[msgType]++;

            // Calculate latency
            RTT = pkt.timeStamp - TS;
            if(RTT < 0 || RTT > 40)
            {
                 s6bStats.timeoutCount[msgType]++;
                 return 0;
            }
            s6bStats.latency[msgType] += RTT;
            s6bStats.latencySize[msgType]++;

            if(pkt.resCode < 3000 || pkt.resCode == 70001)
            {
                s6bStats.succCount[msgType]++;
            }
            else
            {
                s6bStats.failCount[msgType]++;
            }

            req[msgType].erase(pkt.hopIdentifier);
            break;

        default:
            return 1;
    }
    return 0;
}

void S6BInterface::printStats(std::string &node)
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
                s6bStats.timeoutCount[(reqIt->first)]++;
                s6bStats.attempts[(reqIt->first)]++;
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

    for(int i=AA; i<= TERM; i++)
    {
        std::string msgType;
        switch(i)
        {
            case AA:
                msgType = "AA";
                break;
            case TERM:
                msgType = "TERMINATE";
                break;
        }

        std::cout << curTime << " " << node <<   " Ix=" << "S6B"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Att"  
                                                          << " Kpv=" << s6bStats.attempts[i]     << std::endl;

        std::cout << curTime << " " << node <<   " Ix=" << "S6B"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Suc"
                                                          << " Kpv="  << s6bStats.succCount[i]     << " " << std::endl;

       std::cout << curTime << " " << node <<   " Ix=" << "S6B"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Fail"
                                                          << " Kpv="      << s6bStats.failCount[i]    << " " << std::endl;
       std::cout << curTime << " " << node <<   " Ix=" << "S6B"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Tout"
                                                          << " Kpv="   << s6bStats.timeoutCount[i] << " " << std::endl;

       if(s6bStats.latencySize[i] > 0)
       {
          std::cout << curTime << " " << node <<   " Ix=" << "S6B"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Laty"
                                                          << " Kpv=" << (int) ((s6bStats.latency[i]/s6bStats.latencySize[i])*1000) << std::endl; 
       }
    }
}

void S6BInterface::clearStats()
{
    memset(&s6bStats,0,sizeof(s6bStats));
}

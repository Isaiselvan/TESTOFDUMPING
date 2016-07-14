#include <unordered_map>
#include <string>
#include <time.h>
#include "S6bInterface.h"

S6BInterface::S6BInterface()
{
    memset(&s6bStats,0,sizeof(s6bStats));
    startTime = 0;
    endTime   = 0;
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
            s6bStats.attempts[msgType]++;
            req[msgType][pkt.hopIdentifier] = pkt.timeStamp;
            break;

        case 0:
            /* Handle Response */
            if(pkt.resCode < 3000 || pkt.resCode == 70001)
            {
                s6bStats.succCount[msgType]++;
            }
            else
            {
                s6bStats.failCount[msgType]++;
            }
            res[msgType][pkt.hopIdentifier] = pkt.timeStamp;
            break;

        default:
            return 1;
    }
    return 0;
}

void S6BInterface::printStats(std::string &node)
{
    curT = startTime;
    static int RTTCount;
    /* Calculate latency */
    it=res.begin();
    while(it != res.end())
    {
        RTTCount = 0;
        tmp=&(it->second);
        it1=tmp->begin();
        while(it1 != tmp->end())
        {
           reqIt = req.find(it->first);
           if (reqIt != req.end())
           {
               reqTmp = &(reqIt->second);
               reqIt1 = reqTmp->find(it1->first);
               if(reqIt1 !=  reqTmp->end())
               {
                   s6bStats.latency[(it->first)] = ((RTTCount * s6bStats.latency[(it->first)]) + (it1->second)-(reqIt1->second))/(++RTTCount);
                   reqTmp->erase(reqIt1);
               }
               else
               {
                   s6bStats.timeoutCount[(it->first)]++;
               }                
           }
           else
           {
               s6bStats.timeoutCount[(it->first)]++;
           }
           it1++;
        }
        it++;
    }

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

        std::cout << curTime << " Ip=" << node <<   " Ix=" << "S6B"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Att"  
                                                          << " Kpv=" << s6bStats.attempts[i]     << std::endl;

        std::cout << curTime << " Ip=" << node <<   " Ix=" << "S6B"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Suc"
                                                          << " Kpv="  << s6bStats.succCount[i]     << " " << std::endl;

       std::cout << curTime << " Ip=" << node <<   " Ix=" << "S6B"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Fail"
                                                          << " Kpv="      << s6bStats.failCount[i]    << " " << std::endl;
       std::cout << curTime << " Ip=" << node <<   " Ix=" << "S6B"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Tout"
                                                          << " Kpv="   << s6bStats.timeoutCount[i] << " " << std::endl;

       std::cout << curTime << " Ip=" << node <<   " Ix=" << "S6B"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Laty"
                                                          << " Kpv=" << (int) (s6bStats.latency[i]*1000000) << std::endl; 
    }
}

void S6BInterface::clearStats()
{
    memset(&s6bStats,0,sizeof(s6bStats));
    req.clear();
    res.clear();
}

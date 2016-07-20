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
            GyStats.attempts[reqtype-1]++;
            req[reqtype][pkt.hopIdentifier] = pkt.timeStamp;
            break;

        case 0:
           if(pkt.resCode < 3000 || pkt.resCode == 70001)
           {
               GyStats.succCount[reqtype-1]++;
           }
           else
           {
                GyStats.failCount[reqtype-1]++;
           }
           res[reqtype][pkt.hopIdentifier] = pkt.timeStamp;
           break;

        default:
            return 1;
    }
    return 0;
}

void GyInterface::printStats(std::string &node)
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
                   GyStats.latency[(it->first)-1] = ((RTTCount * GyStats.latency[(it->first)-1]) + (it1->second)-(reqIt1->second))/(++RTTCount);
                   reqTmp->erase(reqIt1);
               } 
               else
               {
                   GyStats.timeoutCount[(it->first)-1]++;
               }
           }
           else
           {
               GyStats.timeoutCount[(it->first)-1]++;
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
                GyStats.timeoutCount[(reqIt->first)-1]++;
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

        std::cout << curTime << " " << node <<   " Ix=" << "Gy"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Att"  
                                                          << " Kpv=" << GyStats.attempts[i-1]     << std::endl;

        std::cout << curTime << " " << node <<   " Ix=" << "Gy"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Suc"
                                                          << " Kpv="  << GyStats.succCount[i-1]     << " " << std::endl;

       std::cout << curTime << " " << node <<   " Ix=" << "Gy"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Fail"
                                                          << " Kpv="      << GyStats.failCount[i-1]    << " " << std::endl;
       std::cout << curTime << " " << node <<   " Ix=" << "Gy"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Tout"
                                                          << " Kpv="   << GyStats.timeoutCount[i-1] << " " << std::endl;

       std::cout << curTime << " " << node <<   " Ix=" << "Gy"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Laty"
                                                          << " Kpv=" << (int) (GyStats.latency[i-1]*1000000) << std::endl; 
    }

}

void GyInterface::clearStats()
{
    memset(&GyStats,0,sizeof(CCGyStats));
    req.clear();
    res.clear();
}

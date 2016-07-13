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
    std::unordered_map<unsigned int, std::unordered_map<uint32_t, unsigned int> >::iterator it;
    std::unordered_map<uint32_t, unsigned int>::iterator it1;
    std::unordered_map<uint32_t, unsigned int> tmp;
    MSGType msgType = DEFAULT;  

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
    std::unordered_map<unsigned int, std::unordered_map<uint32_t, unsigned int> >::iterator it;
    std::unordered_map<uint32_t, unsigned int>::iterator it1;
    std::unordered_map<uint32_t, unsigned int> *tmp;

    std::unordered_map<unsigned int, std::unordered_map<uint32_t, unsigned int> >::iterator reqIt;
    std::unordered_map<uint32_t, unsigned int>::iterator reqIt1;
    std::unordered_map<uint32_t, unsigned int> *reqTmp;


    it=res.begin();
    while(it != res.end())
    {
        tmp=&(it->second);
        it1=tmp->begin();
        while(it1 != tmp->end())
        {
           reqIt = req.find(it->first);
           //std::cout << "ABHINAY:: After reqIt" << std::endl;
           if (reqIt != req.end())
           {
               reqTmp = &(reqIt->second);
               reqIt1 = reqTmp->find(it1->first);
               if(reqIt1 !=  reqTmp->end())
               {
                   s6bStats.latency[it->first] += (it1->second)-(reqIt1->second);
                   reqTmp->erase(reqIt1);
               } 
           }

           //if(req[it->first][it1->first] !=0)
           //{
           //    GxStats.latency[it->first] += (it1->second)-(req[it->first][it1->first]);
              //std::cout << "Diff:" << (it1->second)-(req[it->first][it1->first]) << std::endl;
           //}
           it1++;
        }

        //std::cout << "ABHINAY LATENCY is :" << GxStats.latency[it->first] << std::endl;
        it++;
    }

    char TimeBuf[300];
    time_t curT = startTime;
    struct tm * curTimeInfo;
    curTimeInfo = localtime(&curT);
    strftime(TimeBuf, 100, "%F  %T", curTimeInfo);
    std::string curTime(TimeBuf);

    for(int i=0; i< 2; i++)
    {
        std::string msgType;
        switch(i)
        {
            case 0:
                msgType = "AA";
                break;
            case 1:
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
                                                          << " Kpv=" <<  (float)s6bStats.latency[i] << std::endl; 
    }
}

void S6BInterface::clearStats()
{
    memset(&s6bStats,0,sizeof(s6bStats));
    req.clear();
    res.clear();
}

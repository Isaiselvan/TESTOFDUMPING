#include "GxInterface.h"
#include <unordered_map>
#include <string>

GxInterface::GxInterface()
{
    memset(&GxStats,0,sizeof(CCGxStats));
    startTime = 0;
    endTime   = 0;
    reqtype   = 0;
}

int GxInterface::addPkt(Diameter &pkt)
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
       default :
           return 1;
    }

    switch(pkt.request)
    {
        case 1:
            /* Handle Request */
            GxStats.attempts[reqtype-1]++;
            req[reqtype][pkt.hopIdentifier] = pkt.timeStamp;
            break;

        case 0:
            /* Handle Response */
            
            if(pkt.resCode < 3000 || pkt.resCode == 70001)
            {
                GxStats.succCount[reqtype-1]++;
            }
            else
            {
                GxStats.failCount[reqtype-1]++;
            }
            res[reqtype][pkt.hopIdentifier] = pkt.timeStamp;

            break;

        default:
            return 1;
    }
    return 0;
}

void GxInterface::printStats(std::string &node)
{
    curT = startTime;
    static int RTTCount;
    // Calculate latency 
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
                   GxStats.latency[(it->first)-1] = ((RTTCount * GxStats.latency[(it->first)-1]) + (it1->second)-(reqIt1->second))/(++RTTCount);
                   reqTmp->erase(reqIt1);
               }
               else
               {
                   GxStats.timeoutCount[(it->first)-1]++;
               }
           }
           else
           {
               GxStats.timeoutCount[(it->first)-1]++;
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
                GxStats.timeoutCount[(reqIt->first)-1]++;
                reqTmp->erase(reqIt1++);
            }
            else
            {
                reqIt1++;
            }
        }
        reqIt++;
    }

    // Print Stats
    curTimeInfo = localtime(&curT);
    strftime(TimeBuf, 100, "%F  %T", curTimeInfo);
    std::string curTime(TimeBuf);

    for(int i=INITIAL; i<= TERMINATE; i++)
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
        }
        std::cout << curTime << " Ip=" << node <<   " Ix=" << "Gx"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Att"  
                                                          << " Kpv=" << GxStats.attempts[i-1]     << std::endl;

        std::cout << curTime << " Ip=" << node <<   " Ix=" << "Gx"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Suc"
                                                          << " Kpv="  << GxStats.succCount[i-1]     << " " << std::endl;

       std::cout << curTime << " Ip=" << node <<   " Ix=" << "Gx"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Fail"
                                                          << " Kpv="      << GxStats.failCount[i-1]    << " " << std::endl;
       std::cout << curTime << " Ip=" << node <<   " Ix=" << "Gx"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Tout"
                                                          << " Kpv="      << GxStats.timeoutCount[i-1]    << " " << std::endl;


       std::cout << curTime << " Ip=" << node <<   " Ix=" << "Gx"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Laty"
                                                          << " Kpv=" <<  (int)(GxStats.latency[i-1] * 1000000)<< std::endl; 
    }
}

void GxInterface::clearStats()
{
    memset(&GxStats,0,sizeof(CCGxStats));
    //req.clear();
    res.clear();
}

#include <map>
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
    std::map<unsigned int, std::map<uint32_t, unsigned int> >::iterator it;
    std::map<uint32_t, unsigned int>::iterator it1;
    std::map<uint32_t, unsigned int> tmp;
    MSGType msgType = DEFAULT;  

    switch(pkt.getCC())
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

    switch(pkt.getRequest())
    {
        case 1:
            /* Handle Request */
            s6bStats.attempts[msgType]++;
            it = req.find(msgType);
            if(it != req.end())
            {
                tmp =it->second;
            }

            tmp[pkt.getHopIdentifier()] = pkt.getTimestamp();
            req[msgType] = tmp;
            break;

        case 0:
            /* Handle Response */
            it =  req.find(msgType);
            if(it != req.end())
            {
                tmp =it->second;
            }
            
            it1 = tmp.find(pkt.getHopIdentifier());
            if(it1 == tmp.end())
            {
                s6bStats.unKnwRes[msgType]++;
            }
            else
            {
                if(pkt.getResCode() < 3000 || pkt.getResCode() == 70001)
                {
                    s6bStats.succCount[msgType]++;
                }
                else
                {
                    s6bStats.failCount[msgType]++;
                }

                s6bStats.latency[msgType] = ((s6bStats.latency[msgType])*(s6bStats.latencySize[msgType]) + (pkt.getTimestamp()-(it1->second)) / (++s6bStats.latencySize[msgType]));
            }

            /* Delete the request from map */
            tmp.erase(pkt.getHopIdentifier()); 
            req[msgType] = tmp;
            break;

        default:
            return 1;
    }
    return 0;
}

void S6BInterface::printStats()
{
    char TimeBuf[300];
    time_t curT = startTime;
    struct tm * curTimeInfo;
    curTimeInfo = localtime(&curT);
    strftime(TimeBuf, 100, "%F  %T", curTimeInfo);
    std::string curTime(TimeBuf);

    for(int i=0; i<3; i++)
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

        std::cout << "splunk " << curTime << " DIAMETER " << "Interface=" << "S6B"                   << " "
                                                          << "Type="      << msgType                 << " "
                                                          << "Attempts=" << s6bStats.attempts[i]     << " "
                                                          << "Success="  << s6bStats.succCount[i]    << " " 
                                                          << "Fail="     << s6bStats.failCount[i]    << " " 
                                                          << "Timeout="  << s6bStats.timeoutCount[i] << " " 
                                                          << "Latency="  << s6bStats.latency[i]      << std::endl;
    }

    /*
    std::map<unsigned int, std::map<uint32_t, unsigned int> >::iterator it;
    std::map<uint32_t, unsigned int>::iterator it1;
    std::map<uint32_t, unsigned int> tmp;
    for(it = req.begin(); it != req.end(); it++)
    {
        int reqtype = it->first;
        tmp = it->second;
        switch(reqtype)
        {
            case 1:
                std::cout << "ABHINAY:: TIME OUT INITIAL REQUESTS" << std::endl;
                break;
            case 2:
                std::cout << "ABHINAY:: TIME OUT UPDATE REQUESTS" << std::endl;
                break;
            case 3:
                std::cout << "ABHINAY:: TIME OUT TERMINATE REQUESTS" << std::endl;
                break;
        }
        for(it1 = tmp.begin(); it1 != tmp.end(); it1++)
        {
            std::cout << "ABHINAY::hopId:" << it1->first << " Time:" << it1->second << std::endl; 
        }
    }
    */
}

void S6BInterface::clearStats()
{
    memset(&s6bStats,0,sizeof(s6bStats));
    std::map<unsigned int, std::map<uint32_t, unsigned int> >::iterator it;
    std::map<uint32_t, unsigned int>::iterator it1;
    std::map<uint32_t, unsigned int> tmp;
    for(it = req.begin(); it != req.end(); it++)
    {
        int reqtype = it->first;
        tmp = it->second;

        for(it1 = tmp.begin(); it1 != tmp.end(); it1++)
        {
            if(it1->second + DIAMETER_TIMEOUT < endTime)
            {
                s6bStats.timeoutCount[reqtype]++;
                tmp.erase(it1->first); 
            }
        }
    }
}

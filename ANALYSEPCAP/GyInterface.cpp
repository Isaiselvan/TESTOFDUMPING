#include "GyInterface.h"
#include <map>
#include <string>
#include <time.h>

GyInterface::GyInterface()
{
    memset(&GyStats,0,sizeof(CCGyStats));
    startTime = 0;
    endTime   = 0;
}

int GyInterface::addPkt(Diameter &pkt)
{
    std::map<unsigned int, std::map<uint32_t, unsigned int> >::iterator it;
    std::map<uint32_t, unsigned int>::iterator it1;
    std::map<uint32_t, unsigned int> tmp;

    if(pkt.cc != CCRorA)
    {
        return 1;
    }

    unsigned int reqtype = pkt.reqType;
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
            GyStats.attempts[reqtype-1]++;
            it = req.find(reqtype);
            if(it != req.end())
            {
                tmp =it->second;
            }

            tmp[pkt.hopIdentifier] = pkt.timeStamp;
            req[reqtype] = tmp;
            break;

        case 0:
            /* Handle Response */
            it =  req.find(reqtype);
            if(it != req.end())
            {
                tmp =it->second;
            }
            
            it1 = tmp.find(pkt.hopIdentifier);
            if(it1 == tmp.end())
            {
                GyStats.unKnwRes[reqtype-1]++;
            }
            else
            {
                if(pkt.resCode < 3000 || pkt.resCode == 70001)
                {
                    GyStats.succCount[reqtype-1]++;
                }
                else
                {
                    GyStats.failCount[reqtype-1]++;
                }

                GyStats.latency[reqtype-1] = ((GyStats.latency[reqtype-1])*(GyStats.latencySize[reqtype-1]) + (pkt.timeStamp-(it1->second)) / (++GyStats.latencySize[reqtype-1]));
            }

            /* Delete the request from map */
            tmp.erase(pkt.hopIdentifier); 
            req[reqtype] = tmp;
            break;

        default:
            return 1;
    }
    return 0;
}

void GyInterface::printStats()
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
                msgType = "INITIAL";
                break;
            case 1:
                msgType = "UPDATE";
                break;
            case 2:
                msgType = "TERMINATE";
                break;
        }

        std::cout << "splunk " << curTime << " DIAMETER " << "Interface=" << "Gy"                   << " "
                                                          << "Type="      << msgType                << " "
                                                          << "Attempts=" << GyStats.attempts[i]     << " "
                                                          << "Success="  << GyStats.succCount[i]    << " " 
                                                          << "Fail="     << GyStats.failCount[i]    << " " 
                                                          << "Timeout="  << GyStats.timeoutCount[i] << " " 
                                                          << "Latency="  << GyStats.latency[i]      << std::endl;
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

void GyInterface::clearStats()
{
    memset(&GyStats,0,sizeof(CCGyStats));
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
                GyStats.timeoutCount[reqtype-1]++;
                tmp.erase(it1->first); 
            }
        }
    }
}

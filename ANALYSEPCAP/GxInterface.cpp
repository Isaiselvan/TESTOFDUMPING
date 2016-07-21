#include "GxInterface.h"
#include <unordered_map>
#include <string>

GxInterface::GxInterface(std::string &nodepair)
{
    memset(&GxStats,0,sizeof(CCGxStats));
    startTime = 0;
    endTime   = 0;
    reqtype   = 0;
    TS        = 0;
    uid       = 0;
    RTT       = 0;
    initialiseShf(nodepair);
}

int GxInterface::addPkt(Diameter &pkt)
{
#if 0 
//Distinct HOH TEST starts here
    static uint64_t reqHopid= 0;
    static uint64_t resHopid= 10000;

    if(reqHopid == 10000)
	    reqHopid = 0;

    if(resHopid == 0)
	    resHopid = 10000;
    if(pkt.request)
    {
	    pkt.hopIdentifier = ++reqHopid;
    }
    else
    {
	    pkt.hopIdentifier = resHopid--;
    }
#endif 
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

    std::string key;
    std::string keyV;
    switch(pkt.request)
    {
        case 1:
            /* Handle Request */
            key = std::to_string(pkt.hopIdentifier) ;
            keyV = std::to_string(pkt.timeStamp) ;
#if 0
            std::cout <<  std::fixed << "RQ:: Buf Hop id is :" << key << std::endl;
            std::cout <<  std::fixed << "RQ:: Pkt Hop id is :" << pkt.hopIdentifier << std::endl;
            std::cout <<  std::fixed << "RQ:: Pkt time stamp is:" << pkt.timeStamp << std::endl;
            std::cout << "RQ:: Key time stamp is:" << keyV << std::endl;
#endif 
            shfrql->MakeHash(key.c_str(), key.length());
            static int count = 0;
#if 0
            std::cout << "RQ:: Got a request: hopid:" << pkt.hopIdentifier << " timestamp:" << keyV.c_str() << std::endl;
#endif 

//Testing 
            if(!shfrql->GetKeyValCopy())
            {
            req[reqtype][pkt.hopIdentifier] = shfrql->PutKeyVal(keyV.c_str(), keyV.length());
            } 

#if 0
            std::cout << "RQ:: Count = " << count++ << std::endl;
            std::cout << "RQ:: Uid inserted is: " << req[reqtype][pkt.hopIdentifier] << std::endl;
#endif 

            break;

        case 0:
            /* Handle Response */
            uid = 0;
            TS  = 0;
            RTT = 0;
            bzero(shf_val,sizeof(shf_val));
#if 0
            std::cout <<  std::fixed << "RES:: Pkt Hop id is :" << pkt.hopIdentifier << std::endl; 
#endif 
            uid = req[reqtype][pkt.hopIdentifier];
            if(uid <= 0)
            {
                shfrql->MakeHash(key.c_str(), key.length());
                if(shfrql->GetKeyValCopy())
                {
                    TS=atof(shf_val);
#if 0
                    std::cout << "RES:: TIme stamp from no UID of request is:" << shf_val << std::endl;
#endif 
                    while(shfrql->DelKeyVal());
                }
                else
                {
                    res[reqtype][pkt.hopIdentifier] = pkt.timeStamp;
                    return 0;
                }
            }
            else
            {
#if 0
                std::cout << "RES:: Uid from local map: " << uid << std::endl;
#endif 
                if(shfrql->GetUidValCopy(uid))
                {
                    TS=atof(shf_val);
#if 0
                    std::cout << "RES:: TIme stamp from UID of request is:" << shf_val << std::endl;
#endif
                    while(shfrql->DelUidVal(uid));
                }
            }
          
            // Sucess or failure stats 
            if(pkt.resCode < 3000 || pkt.resCode == 70001)
            {
                GxStats.succCount[reqtype-1]++;
            }
            else
            {
                GxStats.failCount[reqtype-1]++;
            }
            GxStats.attempts[reqtype-1]++;

            // Latency stats
            RTT = pkt.timeStamp - TS;
            if(RTT > 40)
            {
               GxStats.timeoutCount[reqtype-1]++;
               return 0;
            }
            else if(RTT < 0)
            {
               return 0;
            }

            GxStats.latency[reqtype-1] += RTT; 
            GxStats.latencySize[reqtype-1]++;
            break;

        default:
            return 1;
    }
    return 0;
}

void GxInterface::printStats(std::string &node)
{
    curT = startTime;
    // Print Stats
    curTimeInfo = localtime(&curT);
    strftime(TimeBuf, 100, "%F  %T", curTimeInfo);
    std::string curTime(TimeBuf);

    for(int i=INITIAL; i<= TERMINATE; i++)
    {
        tmp = req[i];
        for(it = tmp.begin(); it != tmp.end(); it++)
        {
            if(shfrql->GetUidValCopy(it->second))
            {
                TS=atof(shf_val);
                if((endTime-TS) > DIAMETER_TIMEOUT)
                {
                    GxStats.timeoutCount[i-1]++;
                    shfrql->DelUidVal(it->second);
                    req[i].erase(it->first);
                }
            }
        }

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
        std::cout << curTime << " " << node <<   " Ix=" << "Gx"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Att"  
                                                          << " Kpv=" << GxStats.attempts[i-1]     << std::endl;

        std::cout << curTime << " " << node <<   " Ix=" << "Gx"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Suc"
                                                          << " Kpv="  << GxStats.succCount[i-1]     << " " << std::endl;

       std::cout << curTime << " " << node <<   " Ix=" << "Gx"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Fail"
                                                          << " Kpv="      << GxStats.failCount[i-1]    << " " << std::endl;
       std::cout << curTime << " " << node <<   " Ix=" << "Gx"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Tout"
                                                          << " Kpv="      << GxStats.timeoutCount[i-1]    << " " << std::endl;


       if(GxStats.latencySize[i-1] > 0)
       {
           std::cout << curTime << " " << node <<   " Ix=" << "Gx"                    << " "
                                                           << "Ty="      << msgType                 << " "
                                                           << "Kp=Laty"
                                                           << " Kpv=" <<  (int)((GxStats.latency[i-1]/GxStats.latencySize[i-1]) * 1000)<< std::endl;
       }
    }
}

void GxInterface::clearStats()
{
    memset(&GxStats,0,sizeof(CCGxStats));
//    req.clear();
    res.clear();
}

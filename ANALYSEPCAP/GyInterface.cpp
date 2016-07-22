#include "GyInterface.h"
#include <unordered_map>
#include <string>
#include <time.h>

GyInterface::GyInterface(std::string &nodepair)
{
    memset(&GyStats,0,sizeof(CCGyStats));
    startTime = 0;
    endTime   = 0;
    reqtype   = 0;
    TS        = 0;
    uid       = 0;
    RTT       = 0;
    initialiseShf(nodepair);
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

    std::string key  = std::to_string(pkt.hopIdentifier);
    std::string keyV =  std::to_string(pkt.timeStamp);

    switch(pkt.request)
    {
        case 1:
            /* Handle Request */
            #if 0 
               if(!shfrql->AttachExisting(sharefolder.c_str(), sharefile.c_str()))
               {
                std::cout << "Error attaching the shared file hash" << std::endl;
                return -1;
               }
            #endif 
            shfrql->MakeHash(key.c_str(), key.length());
            if(!shfrql->GetKeyValCopy())
            {
            req[reqtype][pkt.hopIdentifier] = shfrql->PutKeyVal(keyV.c_str(), keyV.length());
            }
            //shfrql->Del();
            break;

        case 0:
            /* Handle Response */
             uid = 0;
             TS  = 0;
             RTT = 0;
             #if 0 
                if(!shfrql->AttachExisting(sharefolder.c_str(), sharefile.c_str()))
               {
                std::cout << "Error attaching the shared file hash" << std::endl;
                return -1;
               }
             #endif 

             bzero(shf_val,sizeof(shf_val));

             uid = req[reqtype][pkt.hopIdentifier];
             if(uid == 0)
             {
                shfrql->MakeHash(key.c_str(), key.length());
                if(shfrql->GetKeyValCopy())
                {
                    TS=atof(shf_val);
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
                if(shfrql->GetUidValCopy(uid))
                {
                    TS=atof(shf_val);
                    while(shfrql->DelUidVal(uid));
                    req[reqtype].erase(pkt.hopIdentifier);
                }
             }
             //shfrql->Del();
             // Sucess or failure stats 
             if(pkt.resCode < 3000 || pkt.resCode == 70001)
             {
                 GyStats.succCount[reqtype-1]++;
             }
             else
             {
                 GyStats.failCount[reqtype-1]++;
             }

             
             // Latency stats
             RTT = pkt.timeStamp - TS;
             if(RTT > 40)
             {
                 GyStats.timeoutCount[reqtype-1]++;
                 return 0;
             }
             else if(RTT < 0)
             {
                 return 0;
             }

             GyStats.latency[reqtype-1] += RTT; 
             GyStats.latencySize[reqtype-1]++;
             break;

        default:
            return 1;
    }
    return 0;
}

void GyInterface::printStats(std::string &node)
{
    curT = startTime;

    /* Print Stats */
    curTimeInfo = localtime(&curT);
    strftime(TimeBuf, 100, "%F  %T", curTimeInfo);
    std::string curTime(TimeBuf);
      #if 0 
          if(!shfrql->AttachExisting(sharefolder.c_str(), sharefile.c_str()))
        {
          std::cout << "Error attaching the shared file hash" << std::endl;
          return ;
        }
       #endif 
    for(int i=INITIAL; i<=EVENT; i++)
    {
        tmp = req[i];
        for(it = tmp.begin(); it != tmp.end(); it++)
        {
            if(shfrql->GetUidValCopy(it->second))
            {
                TS=atof(shf_val);
                if((endTime-TS) > DIAMETER_TIMEOUT)
                {
                    GyStats.timeoutCount[i-1]++;
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

       if(GyStats.latencySize[i-1] > 0)
       {
          std::cout << curTime << " " << node <<   " Ix=" << "Gy"                    << " "
                                                          << "Ty="      << msgType                 << " "
                                                          << "Kp=Laty"
                                                          << " Kpv=" << (int) ((GyStats.latency[i-1]/GyStats.latencySize[i-1])*1000) << std::endl; 
       }
    }

   //shfrql->Del();
}

void GyInterface::clearStats()
{
    memset(&GyStats,0,sizeof(CCGyStats));
    //req.clear();
    res.clear();
}

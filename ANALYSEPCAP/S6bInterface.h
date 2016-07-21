#ifndef S6BINTERFACE_BASE_H
#define S6BINTERFACE_BASE_H

#include <iostream>
#include <string>
#include <string.h>
#include <unordered_map>
#include <stdio.h>
#include "libtrace_parallel.h"
#include "Diameter.h"
#include "Interface.h"

enum MSGType
{
     AA = 0,
     TERM,
     DEFAULT
};

struct S6BStats
{
       unsigned int attempts[2];
       unsigned int succCount[2];
       unsigned int failCount[2];
       unsigned int timeoutCount[2];
       unsigned int unPairResCnt[3];
       unsigned int unKnwRes[2];
       unsigned int latencySize[2];
       double latency[2];
};

class S6BInterface:public Interface
{
     private:
       S6BStats s6bStats;
       double TS;
       int uid;
       double RTT;

       /* Local variables */
       char TimeBuf[300];
       time_t curT;
       struct tm * curTimeInfo;
       MSGType msgType;  

     public:
       std::unordered_map<unsigned int, std::unordered_map<uint32_t, long long int> > req;
       std::unordered_map<unsigned int, std::unordered_map<uint32_t, double> > res;
       
       std::unordered_map<uint32_t, long long int> tmp; 
       std::unordered_map<uint32_t, long long int>::iterator it; 

       int addPkt(Diameter &pkt);
       //void printStats();
       void printStats(std::string &node);
       void clearStats();

       S6BInterface(std::string &nodepair);
};

#endif

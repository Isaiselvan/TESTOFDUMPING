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
       unsigned int unKnwRes[2];
       unsigned int latencySize[2];
       double latency[2];
};

class S6BInterface:public Interface
{
     private:
       S6BStats s6bStats;

       /* Local variables */
       char TimeBuf[300];
       time_t curT;
       struct tm * curTimeInfo;
       MSGType msgType;  

     public:
       std::unordered_map<unsigned int, std::unordered_map<uint32_t, double> > req;
       std::unordered_map<unsigned int, std::unordered_map<uint32_t, double> > res;

       /* Iterators to traverse the req and res maps */
       std::unordered_map<unsigned int, std::unordered_map<uint32_t, double> >::iterator it;
       std::unordered_map<uint32_t, double>::iterator it1;
       std::unordered_map<uint32_t, double> *tmp;

       std::unordered_map<unsigned int, std::unordered_map<uint32_t, double> >::iterator reqIt;
       std::unordered_map<uint32_t, double>::iterator reqIt1;
       std::unordered_map<uint32_t, double> *reqTmp;
       
       int addPkt(Diameter &pkt);
       //void printStats();
       void printStats(std::string &node);
       void clearStats();

       S6BInterface();
};

#endif

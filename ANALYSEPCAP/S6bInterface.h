#ifndef S6BINTERFACE_BASE_H
#define S6BINTERFACE_BASE_H

#include <iostream>
#include <string>
#include <string.h>
#include <map>
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

     public:
       std::map<unsigned int, std::map<uint32_t, unsigned int> > req;

       int addPkt(Diameter &pkt);
       void printStats(std::string &node);
       void clearStats();

       S6BInterface();
};

#endif

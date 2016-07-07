#ifndef GYINTERFACE_BASE_H
#define GYINTERFACE_BASE_H

#include <iostream>
#include <string>
#include <string.h>
#include <map>
#include <stdio.h>
#include "libtrace_parallel.h"
#include "Diameter.h"
#include "Interface.h"

struct CCGyStats
{
       unsigned int attempts[4];
       unsigned int succCount[4];
       unsigned int failCount[4];
       unsigned int timeoutCount[4];
       unsigned int unKnwRes[4];
       unsigned int latencySize[4];
       double latency[4];
};

class GyInterface:public Interface
{
     private:
       CCGyStats GyStats;

     public:
       std::map<unsigned int, std::map<uint32_t, unsigned int> > req;

       int addPkt(Diameter &pkt);
       void printStats();
       void clearStats();

       GyInterface();
};

#endif

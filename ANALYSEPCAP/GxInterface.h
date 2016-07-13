#ifndef GXINTERFACE_BASE_H
#define GXINTERFACE_BASE_H

#include <iostream>
#include <string>
#include <string.h>
#include <map>
#include <stdio.h>
#include "libtrace_parallel.h"
#include "Diameter.h"
#include "Interface.h"

struct CCGxStats
{
       unsigned int attempts[3];
       unsigned int succCount[3];
       unsigned int failCount[3];
       unsigned int timeoutCount[3];
       unsigned int unKnwRes[3];
       unsigned int latencySize[3];
       double latency[3];
};

class GxInterface:public Interface
{
     private:
       CCGxStats GxStats;

     public:
       std::map<unsigned int, std::map<uint32_t, unsigned int> > req;
       int addPkt(Diameter &pkt);
       void printStats(std::string &node);
       void clearStats();

       GxInterface();
};

#endif

#ifndef GXINTERFACE_BASE_H
#define GXINTERFACE_BASE_H

#include <iostream>
#include <string>
#include <string.h>
#include <unordered_map>
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
       unsigned int unPairResCnt[3];
       unsigned int unKnwRes[3];
       unsigned int latencySize[3];
       double latency[3];
};

class GxInterface:public Interface
{
     private:
       CCGxStats GxStats;
       double TS;
       int uid;
       double RTT;

       /* Local variables */
       char TimeBuf[300];
       time_t curT;
       struct tm * curTimeInfo;
       unsigned int reqtype;

     public:
       std::unordered_map<unsigned int, std::unordered_map<uint32_t, long long int> > req;
       std::unordered_map<unsigned int, std::unordered_map<uint32_t, double> > res;

       std::unordered_map<uint32_t, long long int> tmp; 
       std::unordered_map<uint32_t, long long int>::iterator it; 

       int addPkt(Diameter &pkt);
       void printStats(std::string &node);
       void clearStats();

       GxInterface(std::string &nodepair);
};

#endif

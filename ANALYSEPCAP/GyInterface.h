#ifndef GYINTERFACE_BASE_H
#define GYINTERFACE_BASE_H

#include <iostream>
#include <string>
#include <string.h>
#include <unordered_map>
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

       /* Local Variables */
       unsigned int reqtype;
       char TimeBuf[300];
       time_t curT;
       struct tm * curTimeInfo;

     public:
       std::unordered_map<unsigned int, std::unordered_map<uint32_t, double> > req;
       std::unordered_map<unsigned int, std::unordered_map<uint32_t, double> > res;

       /* Iterators for traversing req and res maps */
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

       GyInterface();
};

#endif

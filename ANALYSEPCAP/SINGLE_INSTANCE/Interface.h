#ifndef INTERFACE_BASE_H
#define INTERFACE_BASE_H

#include <iostream>
#include <string>
#include <string.h>
#include "libtrace_parallel.h"
#include "Diameter.h"

#define DIAMETER_TIMEOUT 5

class Interface
{
     protected:
         unsigned long int startTime;
         unsigned long int endTime;
     public:

         virtual int addPkt(Diameter &pkt) =0;
         virtual void printStats(std::string &node) =0;
         virtual void clearStats() =0;

         inline int checkTime(unsigned int newTime) __attribute__((always_inline))
         {
             if(newTime < startTime)
             {
                 return 0;
             }

             if(newTime < endTime)
             {
                 return 1;
             }

             if(newTime > endTime)
             {
                startTime = newTime - (newTime % 10);
                endTime   = startTime + 10;
                return 2;
             }

             if(startTime == 0)
             {
                 startTime = newTime;
                 endTime   = startTime + 10;
                 return 1;
             }

             return 1;
         }
     Interface(){};
};

#endif

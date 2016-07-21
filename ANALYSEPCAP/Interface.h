#ifndef INTERFACE_BASE_H
#define INTERFACE_BASE_H

#include <iostream>
#include <string>
#include <string.h>
#include "libtrace_parallel.h"
#include "Diameter.h"
#include <shf.defines.h>
#include <SharedHashFile.hpp>

#define DIAMETER_TIMEOUT 5
#define SHF_DIR "/dev/shm"

class Interface
{
     protected:
         unsigned long int startTime;
         unsigned long int endTime;
   //Hash table implantation 
    std::string nodestr;
   // std::string nodekey;
   // static int nodecount;
    SharedHashFile * shfrql;
    SharedHashFile * NodeShf;
    SharedHashFile * initialiseShf(std::string &nodepair);
  //std::string hashtable_name;

     public:

         virtual int addPkt(Diameter &pkt) =0;
         virtual void printStats(std::string &node) =0;
         virtual void clearStats() =0;

         int checkTime(unsigned int newTime)
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
     Interface();
};

#endif

#ifndef DIAMETER_PROTO_H
#define DIAMETER_PROTO_H

#include <iostream>
#include <string>
#include <string.h>
#include <list>
#include "libtrace_parallel.h"
#include "packetCmm.h"

enum ReqType
{
     INITIAL = 1,
     UPDATE,
     TERMINATE,
     EVENT
};

enum Iface
{
     GX  = 16777238,
     GY  = 4,
     S6B = 16777999, 
     DefaultAppID
};

enum CC
{
        CCRorA         = 272, 
        S6BAA          = 265, 
        S6BTERMINATE   = 275, 
        DefaultCommandCode
};

enum AVPCode
{
        CC_REQUEST_TYPE   = 416,
        RESULT_CODE_TYPE  = 268,
        ORIGIN_HOST_TYPE  = 264,
        DefaultAVPCode
};


class Diameter
{
   public:
        unsigned int msgLength;
        uint8_t commandFlag;
        uint8_t request;
        unsigned int cc;
        unsigned int appId;
        uint32_t hopIdentifier;
        int resCode;
        char *originHost;
        unsigned int reqType;
        unsigned int timeStamp;

        Diameter(char* Msg);
        void printPkt();

        inline void setTimeStamp(unsigned int time)
        {
            timeStamp = time;
        }
};
#endif 

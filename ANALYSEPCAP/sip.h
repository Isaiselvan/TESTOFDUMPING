#ifndef SIP_H
#define SIP_H

#include <stdlib.h>
#include <map>
#include <string>
#include "libtrace_parallel.h"
#include "packetCmm.h"
using namespace std;

#define SIPREQUEST1  "INVITE"
#define SIPREQUEST2  "REGISTER"
#define SIPREQUEST3  "ACK"
#define SIPREQUEST4  "BYE"
#define SIPREQUEST5  "CANCEL"
#define SIPREQUEST6  "PRACK"
#define SIPREQUEST7  "SUBSCRIBE"
#define SIPREQUEST8  "NOTIFY"
#define SIPREQUEST9  "PUBLISH"
#define SIPREQUEST10 "INFO"
#define SIPREQUEST11 "REFER"
#define SIPREQUEST12 "MESSAGE"
#define SIPREQUEST13 "UPDATE"
#define SIPREQUEST14 "OPTIONS"

enum RESPONSECODE
{
    ONE = 0,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX
};

class sip
{
    private:
        static std::map<int,int> resPktsMap;
        static unsigned long int m_totPkts;

    public:
        static int processPkt(libtrace_packet_t *pkt, char *sipBuf);
        static void printStats(string &splunk);
        static void cleanStats();
};

#endif //SIP_H

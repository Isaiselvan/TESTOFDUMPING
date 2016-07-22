#include <ctype.h>
#include "sip.h"
using namespace std;

unsigned long int sip::m_totPkts = 0;
//unsigned long int sip::m_ResCodePkts[] = {0, 0, 0, 0, 0, 0};
std::map<int,int> sip::resPktsMap;

int sip::isInteger(char num[])
{
    int ret = 1;
    int i=0;
    while(num[i] != '\0')
    {
        if(!isdigit(num[i]))
        {
            ret = 0;
            break;
        }

        i++;
    }
 
    return ret;
}

int sip::processPkt(libtrace_packet_t * pkt, char *sipBuf)
{
    char buf[4];
    char responseCode[4];

    memset(buf, '\0', 4);
    memset(responseCode, '\0', 4);

    m_totPkts++;

    memcpy(buf, sipBuf, 3);

    if(!strcmp(buf, "SIP"))
    {
        memcpy(responseCode, sipBuf+8, 3);

        if(isInteger(responseCode))
        {
            int resCode = atoi(responseCode);
            std::map<int,int>::iterator it;
            it = resPktsMap.find(resCode);

            if(it != resPktsMap.end())
            {
                resPktsMap[resCode]++;
            }
            else
            {
                resPktsMap[resCode] = 1;
            }

            // m_ResCodePkts[((resCode/100)-1)]++;
        }
    }
}

void sip::printStats(string &splunk)
{
    std::map<int,int>::iterator it;
    for(it = resPktsMap.begin(); it != resPktsMap.end(); it++)
    {
        if(!it->second)
            continue;
        string responseName = "Unknown"; 
        if( it->first > 99 && it->first < 200)
         responseName = "Provisional-SipResponses";  
        else if(it->first > 199 && it->first < 300)
         responseName = "Successful-SipResponses";
        else if(it->first > 299 && it->first < 400)
         responseName =  "Redirection-SipResponses"; 
        else if(it->first > 399 && it->first < 500)
         responseName = "ClientFailure-SipResponses";
        else if(it->first > 499 && it->first < 600)
         responseName = "ServerFailure-SipResponses";
        else if(it->first > 599 && it->first < 700)
         responseName = "GlobalFailure-SipResponses";
 
        cout << splunk << " SipCodeString=" << responseName <<" ResponseCode=" << it->first << " Error_pckt_count="
             << it->second << std::endl;
    }

    cleanStats();
}

void sip::cleanStats()
{
    //memset(m_ResCodePkts, 0, 6);
    m_totPkts = 0;
    std::map<int,int>::iterator it;
    for(it = resPktsMap.begin(); it != resPktsMap.end(); it++)
    {
        it->second = 0;
    }
}

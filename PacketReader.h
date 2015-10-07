
#include <iostream>
#include <pthread.h>
#include "readerwriterqueue.h"
#include <sys/resource.h>
#include <pcap.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "Common.h"

#define IFSZ 16
#define FLTRSZ 120
#define MAXHOSTSZ 256
#define SNAP_LEN 65535 // Full packet reading 
#define PCAPDBUF_LEN 819200 // 10 * 8192 


using namespace moodycamel;
extern ReaderWriterQueue <struct pcktPLUSEpcaphd> buffer_;



class PacketReader
{
 private:

        char m_interfacename[IFSZ];
        pcap_t *m_pcapHandle;       /* packet capture descriptor */
        struct pcap_stat m_pcapstatus;     /* packet statistics */

        char errbuf[PCAP_ERRBUF_SIZE];  /* buffer to hold error text */
        char lohost[MAXHOSTSZ];   /* local host name */
        char fliterstr[FLTRSZ];     /* bpf filter string */

        char prestr[80]; /* prefix string for errors from pcap_perror */
        struct bpf_program prog; /* compiled bpf filter program */
        // C++11 supports initialising the members
        int optimize = 1;/* passed to pcap_compile to do optimization */
        int snaplen = SNAP_LEN;/* amount of data per packet */
        int promisc = 1; /* do not change mode; if in promiscuous */
 /* mode, stay in it, otherwise, do not */
        int to_ms = 1000;/* timeout, in milliseconds */
        int m_numberofpackets ;  /*for continous number of packets to capture */
        u_int32_t net = 0; /* network IP address */
        u_int32_t mask = 0;/* network address mask */
        char maskstr[INET_ADDRSTRLEN];  /* dotted decimal form of net mask */
        char netstr[INET_ADDRSTRLEN];   /* dotted decimal form of address */
        int linktype = 0;        /* data link type */
        int pcount = 0;          /* number of packets actually read */

        // Control thread stop
        bool m_threadStop = false; 
        static int countPkt;

        bool m_bflag;
        unsigned long int m_buflen;
 
 public:
       static PacketReader * GetInstance(){
          if( NULL == m_onlyreader )
           {
             m_onlyreader = new PacketReader();
           }
           return m_onlyreader;
          }
        virtual ~PacketReader(){
        }
        void Reader_Run ();
        void SetAffinityValue(int aff) { m_readeraffinity = aff ;}
        void SetPriority(int prio) {m_readerpriority = prio;}
        void SetInterface(const char * ifs ) {
          strcpy(m_interfacename, ifs);}
        void SetFilterBSF(std::string bsf){sprintf(fliterstr,bsf.c_str());}
        void SetSnapLen(int slen) {snaplen = slen; }
        int GetSnapLen(){ return snaplen;}
        void cleanup();
        void PrintStat();
        void StopThread(){m_threadStop = true; }
        void SetpcapBufl(unsigned long int blen){
         m_bflag = true;   
         m_buflen = blen;
        }
 private:

       static PacketReader * m_onlyreader;
       int m_readeraffinity;
       int m_readerpriority;
 private:
        PacketReader (){
         // Will not change the affinity if the value is -1 
         m_readeraffinity = -1; 
         m_readerpriority = -1;
         m_bflag = false;
         strcpy(fliterstr, "");
         m_buflen = PCAPDBUF_LEN;
        }
        bool SetThreadAttributes();
        void PcapStartUp();
        void createPcapFheader();
        static void WritePKTtoBuf(u_char *, const struct pcap_pkthdr *, const u_char *);
   //TODO
   //   //Write filters and controls
   
};


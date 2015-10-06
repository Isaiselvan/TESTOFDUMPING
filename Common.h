#ifndef COMMON_PCAP
#define COMMON_PCAP

#include <pcap.h>
#include <fcntl.h>
#include <cstdint>
#include <cstring>

#define MAX_PCKT_LEN 9198 //Jumbo frame MTU 1501 - 9198
#define PRACTICAL_LEN 65535 // snap length default
#define READ_PRIO       -15     /* niceness value for Reader thread */
#define WRITE_PRIO      10      /* niceness value for Writer thread */
#define WAITUSEC        10     /*Sleep while buffer empty*/
#define READ_AFF 	2    /* default Affinity of Read thread */
#define WRITE_AFF	3    /* default Affinity of Write thread */ 

#if 0
struct pcap_file_header {
	u_int32_t magic;
	u_short version_major;
	u_short version_minor;
	int32_t thiszone; /* gmt to local correction */
	u_int32_t sigfigs;    /* accuracy of timestamps */
	u_int32_t snaplen;    /* max length saved portion of each pkt */
	u_int32_t linktype;   /* data link type (LINKTYPE_*) */
};
#endif 

struct pcap_timeval {
    bpf_int32 tv_sec;           /* seconds */
    bpf_int32 tv_usec;          /* microseconds */
};

struct pcap_sf_pkthdr {
    struct pcap_timeval ts;     /* time stamp */
    bpf_u_int32 caplen;         /* length of portion present */
    bpf_u_int32 len;            /* length this packet (off wire) */
};

struct pcktPLUSEpcaphd
{
  pcap_sf_pkthdr sf_hdr; // Pcap packet header
  char pktdata[PRACTICAL_LEN];  // Not interested in data so keep it minimum 1501 //TODO need to adjusted for memory consumption
};


#endif

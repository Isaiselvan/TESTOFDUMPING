
#include "main.h"
#include <pcap.h>
#include <arpa/inet.h>
#include <stdbool.h>


/* Constants of the system */
#define MEMPOOL_NAME "cluster_mem_pool"                         // Name of the NICs' mem_pool
#define MEMPOOL_ELEM_SZ  2048                                    // Power of two greater than 1500
#define MEMPOOL_CACHE_SZ 512                                    // Max is 512

#define INTERMEDIATERING_NAME "intermedate_ring"

#define RX_QUEUE_SZ 4096                        // The size of rx queue. Max is 4096 and is the one you'll have best performances with. Use lower if you want to use Burst Bulk Alloc.
#define TX_QUEUE_SZ 256                 // Unused, you don't tx packets
#define PKT_BURST_SZ 4096               // The max size of batch of packets retreived when invoking the receive function. Use the RX_QUEUE_SZ for high speed

#define IFSZ 16
#define FLTRSZ 120
#define MAXHOSTSZ 256
#define SNAP_LEN 65535 // Full packet reading
#define PCAPDBUF_LEN 819200 // 10 * 8192


#define RTE_LOGTYPE_ICIS RTE_LOGTYPE_USER1
#define PRINT_INFO(fmt, args...)        RTE_LOG(INFO, ICIS, fmt "\n", ##args)

char m_interfacename[IFSZ] = "lo" ;
pcap_t *m_pcapHandle;       /* packet capture descriptor */
struct pcap_stat m_pcapstatus;     /* packet statistics */

char errbuf[PCAP_ERRBUF_SIZE];  /* buffer to hold error text */
char lohost[MAXHOSTSZ];   /* local host name */
char fliterstr[FLTRSZ];     /* bpf filter string */

char prestr[80]; /* prefix string for errors from pcap_perror */
struct bpf_program prog; /* compiled bpf filter program */
// C++11 supports initialising the members
int optimize = 1;/* passed to pcap_compile to do optimization */
int snaplen =  SNAP_LEN;/* amount of data per packet */
int promisc = 0; /* 1 do not change mode; if in promiscuous */
/* mode, stay in it, otherwise, do not */
int to_ms = 1;/* timeout, in milliseconds */
int m_numberofpackets  = 0;  /*for continous number of packets to capture */
u_int32_t net = 0; /* network IP address */
u_int32_t mask = 0;/* network address mask */
char maskstr[INET_ADDRSTRLEN];  /* dotted decimal form of net mask */
char netstr[INET_ADDRSTRLEN];   /* dotted decimal form of address */
int linktype = 0;        /* data link type */
int pcount = 0;          /* number of packets actually read */

// Control thread stop
bool m_threadStop = false;
//static int countPkt;


unsigned long int m_buflen = 2097152; // 2 * 1024 * 1024
/* Global vars */
char * file_name = NULL;
char file_name_rotated [1000];
pcap_dumper_t * pcap_file_p;
uint64_t max_packets = 0 ;
uint64_t buffer_size =   262144; 
uint64_t seconds_rotation = 0;
uint64_t last_rotation = 0;
int64_t  nb_rotations=0;
int64_t  max_rotations = -1 ;
uint64_t max_size = 0 ;
uint64_t nb_captured_packets = 0;
uint64_t nb_dumped_packets = 0;
uint64_t sz_dumped_file = 0;
uint64_t start_secs;
int do_shutdown = 0;
pcap_t *pd;
int nb_sys_ports;
static struct rte_mempool * pktmbuf_pool;
static struct rte_ring    * intermediate_ring;

/* Main function */
int main(int argc, char **argv)
{
        int ret;
        //int i;

        /* Create handler for SIGINT for CTRL + C closing and SIGALRM to print stats*/
        signal(SIGINT, sig_handler);
        signal(SIGALRM, alarm_routine);

        /* Initialize DPDK enviroment with args, then shift argc and argv to get application parameters */
        ret = rte_eal_init(argc, argv);
        if (ret < 0) FATAL_ERROR("Cannot init EAL\n");
        argc -= ret;
        argv += ret;

        /* Check if this application can use two cores*/
        ret = rte_lcore_count ();
        if (ret != 2) FATAL_ERROR("This application needs exactly two (2) cores.");

        /* Parse arguments */
        parse_args(argc, argv);
        if (ret < 0) FATAL_ERROR("Wrong arguments\n");
#if 0
        /* Probe PCI bus for ethernet devices, mandatory only in DPDK < 1.8.0 */
        #if RTE_VER_MAJOR == 1 && RTE_VER_MINOR < 8
                ret = rte_eal_pci_probe();
                if (ret < 0) FATAL_ERROR("Cannot probe PCI\n");
        #endif
#endif 

  
        //pktmbuf_pool = rte_mempool_create(MEMPOOL_NAME, buffer_size-1, MEMPOOL_ELEM_SZ, MEMPOOL_CACHE_SZ, sizeof(struct rte_pktmbuf_pool_private), rte_pktmbuf_pool_init, NULL, rte_pktmbuf_init, NULL,rte_socket_id(), 0);
        pktmbuf_pool = rte_pktmbuf_pool_create(MEMPOOL_NAME,buffer_size-1, MEMPOOL_CACHE_SZ, 0, /*snaplen + RTE_PKTMBUF_HEADROOM*/MEMPOOL_ELEM_SZ, rte_socket_id());
        if (pktmbuf_pool == NULL) FATAL_ERROR("Cannot create cluster_mem_pool. Errno: %d [ENOMEM: %d, ENOSPC: %d, E_RTE_NO_TAILQ: %d, E_RTE_NO_CONFIG: %d, E_RTE_SECONDARY: %d, EINVAL: %d, EEXIST: %d]\n", rte_errno, ENOMEM, ENOSPC, RTE_MAX_TAILQ/*E_RTE_NO_TAILQ*/, E_RTE_NO_CONFIG, E_RTE_SECONDARY, EINVAL, EEXIST  );

        /* Init intermediate queue data structures: the ring. */
        intermediate_ring = rte_ring_create (INTERMEDIATERING_NAME, buffer_size, rte_socket_id(), RING_F_SP_ENQ | RING_F_SC_DEQ );
        if (intermediate_ring == NULL ) FATAL_ERROR("Cannot create ring");


        /* Start consumer and producer routine on 2 different cores: consumer launched first... */
        ret =  rte_eal_mp_remote_launch (packet_producer, NULL, SKIP_MASTER);
        if (ret != 0) FATAL_ERROR("Cannot start consumer thread\n");

        /* ... and then loop in consumer */
        //packet_producer ( NULL );
        packet_consumer(NULL);

        return 0;
}



/* Loop function, batch timing implemented */
static int packet_producer(__attribute__((unused)) void * arg){
//      struct rte_mbuf * pkts_burst[PKT_BURST_SZ];
//      struct timeval t_pack;
        struct rte_mbuf * m;
//      int read_from_port = 0;
        //int i;
        //, ret;

        PRINT_INFO("Lcore id of producer %d\n", rte_lcore_id());
        /* Start stats */
        alarm(1);

       // for (i=0;i<nb_sys_ports; i++)
         //       rte_eth_stats_reset ( i );
        PcapStartUp();

       struct pcap_pkthdr *PcapHdr;
       const u_char * data;
       int status;
       uint16_t buf_size; 
        /* Infinite loop */
        for (;;) {

                /* Read a burst for current port at queue 'nb_istance'*/
                //nb_rx = rte_eth_rx_burst(read_from_port, 0, pkts_burst, PKT_BURST_SZ);
            status = pcap_next_ex (m_pcapHandle, &PcapHdr, &data);
           // printf("Step 1\n"); 
           buf_size = (uint16_t)(rte_pktmbuf_data_room_size(pktmbuf_pool) - RTE_PKTMBUF_HEADROOM);
           if(unlikely(buf_size < PcapHdr->caplen) ) 
              continue ;
            if(status == 1)
            {
                    //printf("step 2 \n");
                    m = rte_pktmbuf_alloc(pktmbuf_pool);
                    rte_memcpy(rte_pktmbuf_mtod(m, void *), data,
                                        PcapHdr->caplen);
                    //printf("step 3 \n");
                   //m = data;
                    nb_captured_packets++;
                    m->tx_offload = PcapHdr->ts.tv_sec;;
                    m->udata64 =  PcapHdr->ts.tv_usec;
                    m->data_len = (uint16_t)PcapHdr->caplen;
                    m->pkt_len = (uint16_t) PcapHdr->len;
                      /*Enqueieing buffer */
                    rte_ring_enqueue (intermediate_ring, m);
                    m_numberofpackets++;
           }
           else if (status == -1)
           {
                    PRINT_INFO("\nError while reading the packets\t from dev:%s %s ",m_interfacename,pcap_geterr(m_pcapHandle));
                    FATAL_ERROR("Error while reading the packets\n");

           }


        }
        return 0;
}

void PcapStartUp(void)
{


        int status ;
        char * cp;

        if(!(m_pcapHandle = pcap_create(m_interfacename, errbuf)))
          fprintf(stderr,"%s",errbuf);

        status = pcap_set_snaplen(m_pcapHandle, snaplen);
        if (status != 0)
          fprintf(stderr,"%s: Can't set snapshot length: %s",
                                m_interfacename, pcap_statustostr(status));

        status = pcap_set_promisc(m_pcapHandle, promisc);
        if (status != 0)
          fprintf(stderr,"%s: Can't set promiscuous mode: %s",
                                m_interfacename, pcap_statustostr(status));

        status = pcap_set_timeout(m_pcapHandle, to_ms);
        if (status != 0)
         fprintf(stderr,"%s: pcap_set_timeout failed: %s",
                            m_interfacename, pcap_statustostr(status));
        
                status = pcap_set_buffer_size(m_pcapHandle, m_buflen);
        if (status != 0)
        fprintf(stderr,"%s: Can't set buffer size: %s",
                            m_interfacename, pcap_statustostr(status));
                
        status = pcap_activate(m_pcapHandle);
        if (status < 0) {
                /*
                 * pcap_activate() failed.
                 */
                cp = pcap_geterr(m_pcapHandle);
                if (status == PCAP_ERROR)
                        fprintf(stderr,"%s", cp);
                else if ((status == PCAP_ERROR_NO_SUCH_DEVICE ||
                                        status == PCAP_ERROR_PERM_DENIED) &&
                                *cp != '\0')
                        fprintf(stderr,"%s: %s\n(%s)", m_interfacename,
                                        pcap_statustostr(status), cp);
                else
                        fprintf(stderr,"%s: %s", m_interfacename,
                                        pcap_statustostr(status));
                 exit(0);//
        } else if (status > 0) {
                /*
                 * pcap_activate() succeeded, but it's warning us
                 * of a problem it had.
                 */
                cp = pcap_geterr(m_pcapHandle);
                if (status == PCAP_WARNING)
                        fprintf(stderr,"%s", cp);
                else if (status == PCAP_WARNING_PROMISC_NOTSUP &&
                                *cp != '\0')
                        fprintf(stderr,"%s: %s\n(%s)", m_interfacename,
                                        pcap_statustostr(status), cp);
        }

#if 0
 /*
  * Open the network device for packet capture. This must be called
  * before any packets can be captured on the network device.
  * */
        if (!(m_pcapHandle = pcap_open_live(m_interfacename, snaplen, promisc, to_ms, errbuf))) {
                std::cout << m_interfacename << std::endl;
                fprintf(stderr, "Error opening interface %s: %s\n",
                        m_interfacename, errbuf);
                exit(2);
        }
#endif

 /*
 *  Look up the network address and subnet mask for the network device
 *  returned by pcap_lookupdev(). The network mask will be used later
 *  in the call to pcap_compile().
 * */
        if (pcap_lookupnet(m_interfacename, &net, &mask, errbuf) < 0) {
                fprintf(stderr, "Error looking up network: %s\n", errbuf);
                exit(3);
        }

if (gethostname(lohost,sizeof(lohost)) < 0) {
                fprintf(stderr, "Error getting hostname.\n");
                exit(4);
        }

 /*
 * Second, get the dotted decimal representation of the network address
 * and netmask. These will be used as part of the filter string.
 **/
        inet_ntop(AF_INET, (char*) &net, netstr, sizeof netstr);
        inet_ntop(AF_INET, (char*) &mask, maskstr, sizeof maskstr);

if (pcap_compile(m_pcapHandle,&prog,fliterstr,optimize,mask) < 0) {
 /*
 * Print out appropriate text, followed by the error message
 * generated by the packet capture library.
 */
                fprintf(stderr, "Error compiling bpf filter on %s: %s\n",
                        m_interfacename, pcap_geterr(m_pcapHandle));
                exit(5);
        }


/*
 * Load the compiled filter program into the packet capture device.
 * This causes the capture of the packets defined by the filter
 * program, prog, to begin.
 */
        if (pcap_setfilter(m_pcapHandle, &prog) < 0) {
                /* Copy appropriate error text to prefix string, prestr */
                sprintf(prestr, "Error installing bpf filter on interface %s",
                        m_interfacename);
 /*
 *Print error to screen. The format will be the prefix string,
 *created above, followed by the error message that the packet
 *capture library generates.
 **/
                pcap_perror(m_pcapHandle,prestr);
                exit(6);
        }
}

static int packet_consumer(__attribute__((unused)) void * arg){

        struct timeval t_pack;
        struct rte_mbuf * m;
        u_char * packet;
        char file_name_move[1000];
        int ret;
        struct pcap_pkthdr pcap_hdr;

        /* Init first rotation */
        ret = gettimeofday(&t_pack, NULL);
        if (ret != 0) FATAL_ERROR("Error: gettimeofday failed. Quitting...\n");
        last_rotation = t_pack.tv_sec;
        start_secs = t_pack.tv_sec;

        /* Open pcap file for writing */
        pd = pcap_open_dead(DLT_EN10MB, 65535 );
        sprintf(file_name_rotated, "%s%ld",file_name,last_rotation);
        pcap_file_p = pcap_dump_open(pd, file_name_rotated);
        if(pcap_file_p==NULL)
                FATAL_ERROR("Error in opening pcap file\n");
        printf("Opened file %s\n", file_name_rotated);


        /* Infinite loop for consumer thread */
        for(;;){

                /* Dequeue packet */
                ret = rte_ring_dequeue(intermediate_ring, (void**)&m);

                /* Continue polling if no packet available */
                if( unlikely (ret != 0)) {
                usleep (100);
                continue;
                }

                /* Read timestamp of the packet */
                t_pack.tv_usec = m->udata64;
                t_pack.tv_sec = m->tx_offload;

                /* Rotate if needed */
                if ( seconds_rotation > 0 && t_pack.tv_sec - last_rotation > seconds_rotation ){

                        last_rotation = t_pack.tv_sec;
                        nb_rotations ++;

                        /* Quit if the number of rotations is the max */
                        if (max_rotations != -1 && nb_rotations > max_rotations)
                                sig_handler(SIGINT);

                        /* Close the pcap file */
                        pcap_close(pd);
                        pcap_dump_close(pcap_file_p);
                        sprintf(file_name_move, "%s%s", file_name_rotated, "ready");  
                        if (rename (file_name_rotated, file_name_move))
                        printf("\n failed to rename file %s\n", file_name_rotated); 
                        /* Open pcap file for writing */
                        sprintf(file_name_rotated, "%s%ld", file_name, last_rotation);
                        pd = pcap_open_dead(DLT_EN10MB, 65535 );
                        pcap_file_p = pcap_dump_open(pd, file_name_rotated);
                        if(pcap_file_p==NULL)
                                FATAL_ERROR("Error in opening pcap file\n");
                        printf("\nOpened file %s\n", file_name_rotated);
                }

                /* Compile pcap header */
                pcap_hdr.ts = t_pack;
                pcap_hdr.caplen = rte_pktmbuf_data_len(m);
                pcap_hdr.len = m->pkt_len;
                packet = rte_pktmbuf_mtod(m, u_char * );

                /* Write on pcap */
                pcap_dump ((u_char *)pcap_file_p, & pcap_hdr,  packet);
                nb_dumped_packets++;
                sz_dumped_file += rte_pktmbuf_data_len(m) + sizeof (pcap_hdr) ;

                /* Quit if reached the size threshold */
                if (max_size != 0 && sz_dumped_file >= max_size*1024)
                        sig_handler(SIGINT);

                /* Quit if reached the packet threshold */
                if (max_packets != 0 && nb_dumped_packets >= max_packets)
                        sig_handler(SIGINT);

                /* Free the buffer */
                rte_pktmbuf_free((struct rte_mbuf *)m);
        }
}

void print_stats (void){
       if (!(linktype = pcap_datalink(m_pcapHandle))) {
                fprintf(stderr,
                        "Error getting link layer type for interface %s",
                        m_interfacename);
                exit(9);
        }
        PRINT_INFO("The link layer type for packet capture device %s is: %d.\n",
                m_interfacename, linktype);


      if (pcap_stats(m_pcapHandle, &m_pcapstatus) != 0) {
                fprintf(stderr, "Error getting Packet Capture stats: %s\n",
                        pcap_geterr(m_pcapHandle));
                exit(10);
        }

        /* Print the statistics out */
        PRINT_INFO("Packet Capture Statistics:\n");
        PRINT_INFO("%d packets received by filter\n", m_pcapstatus.ps_recv);
        PRINT_INFO("%d packets dropped by kernel\n", m_pcapstatus.ps_drop);
        PRINT_INFO("%d Packets queued for write opt\n", m_numberofpackets); 



}

void alarm_routine (__attribute__((unused)) int unused){

        /* If the program is quitting don't print anymore */
        if(do_shutdown) return;

        /* Print per port stats */
        print_stats();

        /* Schedule an other print */
        alarm(60);
        //signal(SIGALRM, alarm_routine);

}

/* Signal handling function */
static void sig_handler(int signo)
{
        uint64_t diff;
        int ret;
        struct timeval t_end;
        char file_name_move[1000];

        /* Catch just SIGINT */
        if (signo == SIGINT){

                /* Signal the shutdown */
                do_shutdown=1;

                /* Print the per port stats  */
                printf("\n\nQUITTING...\n");

                ret = gettimeofday(&t_end, NULL);
                if (ret != 0) FATAL_ERROR("Error: gettimeofday failed. Quitting...\n");
                diff = t_end.tv_sec - start_secs;
                printf("The capture lasted %ld seconds.\n", diff);
                print_stats();

                /* Close the pcap file */
                pcap_close(pd);
                pcap_dump_close(pcap_file_p);
                sprintf(file_name_move, "%s%s", file_name_rotated, "ready");
                        if (rename (file_name_rotated, file_name_move))
                printf("\n failed to rename file %s\n", file_name_rotated);
                exit(0);
        }
}


static int parse_args(int argc, char **argv)
{
        int option;


        /* Retrive arguments */
        while ((option = getopt(argc, argv,"w:c:B:G:W:C:S:i:f:b:")) != -1) {
                switch (option) {
                        case 'w' : file_name = strdup(optarg); /* File name, mandatory */
                                break;
                        case 'c': max_packets = atol (optarg); /* Max number of packets to save; default is infinite */
                                break;
                        case 'B': buffer_size = atoi (optarg); /* Buffer size in packets. Must be a power of two . Default is 1048576 */
                                break;
                        case 'G': seconds_rotation = atoi (optarg); /* Rotation of output in seconds. A progressive number will be added to file name */
                                break;
                        case 'W': max_rotations = atoi (optarg); /* Max rotations done. In case of 0, the program quits after first rotation time */
                                break;
                        case 'C': max_size = atoi (optarg); /* Max file size in KB. When reached, the program quits */
                                break;
                        case 'S': snaplen = atoi (optarg); /* Snap lenght default 96  */
                                break;
                        case 'i': strcpy(m_interfacename, optarg); /* Interface name */
                                break;  
                        case 'f': strcpy(fliterstr, optarg); /* BFS filter */ 
                                break;
                        case 'b': m_buflen = atoi(optarg);
                                break;
                        default: return -1;
                }
        }

        /* Returning bad value in case of wrong arguments */
        if(file_name == NULL || isPowerOfTwo (buffer_size)!=1 )
                return -1;

        return 0;

}

int isPowerOfTwo (unsigned int x)
{
  return ((x != 0) && !(x & (x - 1)));
}




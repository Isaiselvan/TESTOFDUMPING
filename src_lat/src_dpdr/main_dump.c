
#include "main.h"
#include <pcap.h>
#include <arpa/inet.h>
#include <stdbool.h>


/* Constants of the system */
#define MEMPOOL_NAME "cluster_mem_pool"                         // Name of the NICs' mem_pool
#define MEMPOOL_ELEM_SZ  256                                    // Power of two greater than 1500
#define MEMPOOL_CACHE_SZ 512 
#define INTERMEDIATERING_NAME "intermedate_ring"

#define SNAP_LEN 512 // Full packet reading to get host info on Sandbox min is 512


#define RTE_LOGTYPE_FBM RTE_LOGTYPE_USER1
#define PRINT_INFO(fmt, args...)        RTE_LOG(INFO, FBM, fmt "\n", ##args)



int snaplen =  SNAP_LEN;/* amount of data per packet */
long long int m_numberofpackets  = 0;  /*for continous number of packets to capture */
long long int missedouts = 0;


/* Global vars */
char * file_name = NULL;
char file_name_rotated [1000];
pcap_dumper_t * pcap_file_p;
uint64_t max_packets = 0 ;
uint64_t buffer_size = 1048576 ; //Ring size
uint64_t seconds_rotation = 0;
uint64_t last_rotation = 0;
int64_t  nb_rotations=0;
int64_t  max_rotations = -1 ;
uint64_t max_size = 0 ;
uint64_t nb_captured_packets = 0;
uint64_t nb_dumped_packets = 0;
//uint64_t sz_dumped_file = 0;
uint64_t start_secs;
int do_shutdown = 0;
pcap_t *pd;
int nb_sys_ports , en_sys_ports;
static struct rte_mempool * pktmbuf_pool;
static struct rte_ring    * intermediate_ring;
static int readportid[MAX_PORT];
//static int test1=0, test2=0, test3=0, test4 =0;
/* Main function */
int main(int argc, char **argv)
{
        int ret;
        uint8_t nb_ports;
        uint8_t nb_ports_available;
        uint8_t portid ;
	struct lcore_queue_conf *qconf;
	unsigned lcore_id, rx_lcore_id;
        //int i;


        /* Initialize DPDK enviroment with args, then shift argc and argv to get application parameters */
        ret = rte_eal_init(argc, argv);
        if (ret < 0) FATAL_ERROR("Cannot init EAL\n");
        argc -= ret;
        argv += ret;

        /* Check if this application can use three cores*/
        ret = rte_lcore_count ();
        if (ret != 3) FATAL_ERROR("This application needs exactly three (3) cores.");

        /* Parse arguments */
        parse_args(argc, argv);
        if (ret < 0) FATAL_ERROR("Wrong arguments\n");

        nb_ports = rte_eth_dev_count();
        nb_sys_ports = nb_ports;
        if (nb_ports == 0)
          rte_exit(EXIT_FAILURE, "No Ethernet ports - bye\n");          
 
       pktmbuf_pool = NULL;
       int msize = buffer_size;
       while (pktmbuf_pool == NULL)
       { 
        pktmbuf_pool = rte_mempool_create(MEMPOOL_NAME, msize + (nb_ports * 1 * RTE_TEST_RX_DESC_DEFAULT), snaplen + RTE_PKTMBUF_HEADROOM/* (snaplen + 128 + RTE_PKTMBUF_HEADROOM)*/, MEMPOOL_CACHE_SZ, sizeof(struct rte_pktmbuf_pool_private), rte_pktmbuf_pool_init, NULL, rte_pktmbuf_init, NULL,rte_socket_id(), MEMPOOL_F_SP_PUT | MEMPOOL_F_SC_GET);
        //pktmbuf_pool = rte_pktmbuf_pool_create(MEMPOOL_NAME,70000, 64, 0, snaplen + RTE_PKTMBUF_HEADROOM /* RTE_PKTMBUF_HEADROOM MEMPOOL_ELEM_SZ*/, SOCKET_ID_ANY);
        //if (pktmbuf_pool == NULL) FATAL_ERROR("Cannot create cluster_mem_pool. Errno: %d [ENOMEM: %d, ENOSPC: %d, E_RTE_NO_TAILQ: %d, E_RTE_NO_CONFIG: %d, E_RTE_SECONDARY: %d, EINVAL: %d, EEXIST: %d]\n", rte_errno, ENOMEM, ENOSPC, RTE_MAX_TAILQ/*E_RTE_NO_TAILQ*/, E_RTE_NO_CONFIG, E_RTE_SECONDARY, EINVAL, EEXIST  );
        //
          msize = msize - 128;      
        }
         PRINT_INFO("MemPool Size allocated %d\n", (msize + 128));  
        /* Init intermediate queue data structures: the ring. */
        intermediate_ring = NULL;
        msize = buffer_size;
        while (intermediate_ring == NULL)
        {
        intermediate_ring = rte_ring_create (INTERMEDIATERING_NAME, msize ,rte_socket_id(), RING_F_SP_ENQ | RING_F_SC_DEQ );
        msize = msize / 2 ;     
   
        //if (intermediate_ring == NULL ) FATAL_ERROR("Cannot create ring");
        }
         PRINT_INFO("RteRing Size allocated%d\n" , (msize * 2));
        //store_ring = rte_ring_create ("Store_ring",  buffer_size ,rte_socket_id(), RING_F_SP_ENQ | RING_F_SC_DEQ );
          //if (store_ring == NULL ) FATAL_ERROR("Cannot create store ring ");
 
       
        /* Start consumer and producer routine on 2 different cores: consumer launched first... */
        //ret =  rte_eal_mp_remote_launch (packet_consumer, NULL, SKIP_MASTER);
        //if (ret != 0) FATAL_ERROR("Cannot start consumer thread\n");
        rx_lcore_id = 0;
        qconf = NULL;
        /* Initialize the port/queue configuration of each logical core */
        for (portid = 0; portid < nb_ports; portid++) {
                
                /* skip ports that are not enabled */
                if ((l2fwd_enabled_port_mask & (1 << portid)) == 0)
                        continue;

                /* get the lcore_id for this port */
               /* while (rte_lcore_is_enabled(rx_lcore_id) == 0 ||
                       lcore_queue_conf[rx_lcore_id].n_rx_port ==
                       l2fwd_rx_queue_per_lcore) {
                        rx_lcore_id++;
                        //printf("core test  port\n");
                        if (rx_lcore_id >= RTE_MAX_LCORE)
                                rte_exit(EXIT_FAILURE, "Not enough cores\n");
                }*/

                if (qconf != &lcore_queue_conf[rx_lcore_id])
                        /* Assigned a new logical core in the loop above. */
                        qconf = &lcore_queue_conf[rx_lcore_id];

                qconf->rx_port_list[qconf->n_rx_port] = portid;
                qconf->n_rx_port++;
                printf("Lcore %u: RX port %u\n", rx_lcore_id, (unsigned) portid);
                
        }
       nb_ports_available = nb_ports;

/* Initialise each port */
        for (portid = 0; portid < nb_ports; portid++){

 /* skip ports that are not enabled */
                if ((l2fwd_enabled_port_mask & (1 << portid)) == 0) {
                        printf("Skipping disabled port %u\n", (unsigned) portid);
                        nb_ports_available--;
                        continue;
                }
                /* init port */
                printf("Initializing port %u... ", (unsigned) portid);
                fflush(stdout);
                ret = rte_eth_dev_configure(portid, 1, 1, &port_conf);
                if (ret < 0)
                        rte_exit(EXIT_FAILURE, "Cannot configure device: err=%d, port=%u\n",
                                  ret, (unsigned) portid);

                rte_eth_macaddr_get(portid,&l2fwd_ports_eth_addr[portid]);

                /* init one RX queue */
                fflush(stdout);
                ret = rte_eth_rx_queue_setup(portid, 0, nb_rxd,
                                             rte_eth_dev_socket_id(portid),
                                             &rx_conf,
                                             pktmbuf_pool);
                if (ret < 0)
                        rte_exit(EXIT_FAILURE, "rte_eth_rx_queue_setup:err=%d, port=%u\n",
                                  ret, (unsigned) portid);

                /* init one TX queue on each port */
                fflush(stdout);
                ret = rte_eth_tx_queue_setup(portid, 0, nb_txd,
                                rte_eth_dev_socket_id(portid),
                                NULL);
                if (ret < 0)
                        rte_exit(EXIT_FAILURE, "rte_eth_tx_queue_setup:err=%d, port=%u\n",
                                ret, (unsigned) portid);
                /* Start device */
                ret = rte_eth_dev_start(portid);
                if (ret < 0)
                        rte_exit(EXIT_FAILURE, "rte_eth_dev_start:err=%d, port=%u\n",
                                  ret, (unsigned) portid);

                printf("done: \n");

                rte_eth_promiscuous_enable(portid);

                printf("Port %u, MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n\n",
                                (unsigned) portid,
                                l2fwd_ports_eth_addr[portid].addr_bytes[0],
                                l2fwd_ports_eth_addr[portid].addr_bytes[1],
                                l2fwd_ports_eth_addr[portid].addr_bytes[2],
                                l2fwd_ports_eth_addr[portid].addr_bytes[3],
                                l2fwd_ports_eth_addr[portid].addr_bytes[4],
                                l2fwd_ports_eth_addr[portid].addr_bytes[5]);
}

        if (!nb_ports_available) {
                rte_exit(EXIT_FAILURE,
                        "All available ports are disabled. Please set portmask.\n");
        }

        check_all_ports_link_status(nb_ports, l2fwd_enabled_port_mask);

   
//Statistics_lcore        
        lcore_id =0 ;
        int coreVcount = 0;
        RTE_LCORE_FOREACH_SLAVE(lcore_id) {
              if(coreVcount++ == 0) 
              {
               ret = rte_eal_remote_launch(Statistics_lcore, NULL,lcore_id);
               if (ret != 0) FATAL_ERROR("Cannot start consumer thread\n");
              }
              else
              {
               ret = rte_eal_remote_launch(packet_consumer, NULL,lcore_id);
               if (ret != 0) FATAL_ERROR("Cannot start Statistics thread\n");
              } 
        } 
          
        /* Master as producer */
        packet_producer ( NULL );
        //packet_consumer(NULL);
        //Statistics_lcore(NULL);
       
        rte_eal_mp_wait_lcore();
        return 0;
}



static int packet_producer(__attribute__((unused)) void * arg){
        struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
        //struct rte_mbuf * m;
        unsigned lcore_id = rte_lcore_id();
        PRINT_INFO("Lcore id of producer %d\n", lcore_id);
        unsigned int i;
        int idx, portid, nb_rx, ret, pidx =0;
        struct lcore_queue_conf *qconf;
        struct timeval t_pack;

        qconf = &lcore_queue_conf[lcore_id];

        //if (qconf->n_rx_port == 0) {
         //       PRINT_INFO("lcore %u has nothing to do\n", lcore_id);
           //     return -1;
       // }
        PRINT_INFO( "entering main loop on lcore %u\n", lcore_id);
        for (i = 0; i < qconf->n_rx_port; i++) {

                portid = qconf->rx_port_list[i];
                PRINT_INFO(" -- lcoreid=%u portid=%u\n", lcore_id,
                        portid);

        }
      // int debug =0 ;
        /* Infinite loop */
           

        for (;;) {
                          //PRINT_INFO( "portid%d \n", readportid);
			/* Timestamp the packet */
			ret = gettimeofday(&t_pack, NULL);
			if (ret != 0) FATAL_ERROR("Error: gettimeofday failed. Quitting...\n");
                //while ((m = rte_pktmbuf_alloc(pktmbuf_pool) ) == NULL );   

		//status = pcap_next_ex (m_pcapHandle, &PcapHdr, &data);
            // for (i = 0; i < qconf->n_rx_port; i++) {

                        //portid = qconf->rx_port_list[i];
                        nb_rx = rte_eth_rx_burst((uint8_t) readportid[pidx++], 0,
                                                 pkts_burst, MAX_PKT_BURST);
                        if(likely (nb_rx > 0))
                        {

                           for (idx= 0;idx<nb_rx;idx++)
                           {
                             pkts_burst[idx]->tx_offload = t_pack.tv_sec;
                             pkts_burst[idx]->udata64 =  t_pack.tv_usec;
                           }
                           nb_captured_packets += nb_rx;
                           ret = rte_ring_enqueue_burst(intermediate_ring, (void*)pkts_burst,nb_rx);
                                             
                            m_numberofpackets += ret;
                            missedouts += nb_rx -ret;
                         }
    
                         for(;ret<nb_rx;ret++) 
                            rte_pktmbuf_free((struct rte_mbuf *)pkts_burst[ret]);


                  //Multi port read
                          if(pidx == en_sys_ports || pidx == MAX_PORT)  
                              pidx = 0;
                          
                          //readportid = (readportid + 1) % nb_sys_ports - 1;
                          //PRINT_INFO( "portidx %d portno%d numofports%d numofen%d\n", pidx,readportid[pidx], nb_sys_ports, en_sys_ports);
                       /* for (j = 0; j < nb_rx; j++) {
                                m = pkts_burst[j];
                                //rte_prefetch0(rte_pktmbuf_mtod(m, void *));
                                m->tx_offload = t_pack.tv_sec;
                        	m->udata64 =  t_pack.tv_usec;
                                //m->data_len = (uint16_t)PcapHdr->caplen;
                                //m->pkt_len = (uint16_t) PcapHdr->len;

                            while(ENOBUFS == rte_ring_enqueue (intermediate_ring, m) );
                         m_numberofpackets++;
                        }*/
                   //Burst
                    /*
                      ret =0 ;
                        int startidx = 0;
                      while(( ret = rte_ring_sp_enqueue_burst(
                                                intermediate_ring,
                                                (void **) &pkts_burst[startidx],
                                                nb_rx) ) != nb_rx){
                           startidx = startidx + ret ;
                           nb_rx = nb_rx - ret;
                           //pkts_burst = pkts_burst[ret -1];
                           m_numberofpackets += ret;     
                      }
                           m_numberofpackets += ret;     
                    */
                   //rte_ring_sp_enqueue_bulk 	
                   /*if(likely (nb_rx > 0))
                   { 
                    while(-ENOBUFS == rte_ring_sp_enqueue_bulk(intermediate_ring,(void*)pkts_burst,nb_rx));
                    m_numberofpackets += nb_rx;
                   }*/
                                

                //}

           
        }
        return 0;
}

static  int packet_consumer(__attribute__((unused)) void * arg){

        struct timeval t_pack;
        struct rte_mbuf * m[MAX_PKT_BURST];
        //u_char * packet;
        char file_name_move[1000];
        int ret, idx;
        struct pcap_pkthdr pcap_hdr;
        PRINT_INFO("Lcore id of consumer %d\n", rte_lcore_id());
        /* Init first rotation */
        ret = gettimeofday(&t_pack, NULL);
        if (ret != 0) FATAL_ERROR("Error: gettimeofday failed. Quitting...\n");

        last_rotation = t_pack.tv_sec;
        start_secs = t_pack.tv_sec;

        /* Open pcap file for writing */
        pd = pcap_open_dead(DLT_EN10MB, snaplen);
        sprintf(file_name_rotated, "%s%ld",file_name,last_rotation);
        pcap_file_p = pcap_dump_open(pd, file_name_rotated);
        if(pcap_file_p==NULL)
                FATAL_ERROR("Error in opening pcap file\n");
        PRINT_INFO("Opened file %s\n", file_name_rotated);
         /* Start stats */
        //alarm(1);


        /* Infinite loop for consumer thread */
        for(;;){


                /* Dequeue packet */
                //ret = rte_ring_dequeue(intermediate_ring, (void**)&m);
                 while(( ret = rte_ring_sc_dequeue_burst(intermediate_ring, (void **)m,
                                MAX_PKT_BURST)) <= 0);
                //ring_full = false;
                /* Continue polling if no packet available */
               /* if( ret < 0) {
                //usleep(5);
                continue;
                }*/
//TESTING 
                /*for(idx = 0; idx < ret ; idx++)
                {
                rte_pktmbuf_free( (struct rte_mbuf *)m[idx]);
                m[idx] = NULL;
                }
                continue;*/
///END
                for(idx = 0; idx < ret ; idx++)
                {
                rte_prefetch0(rte_pktmbuf_mtod(m[idx], struct rte_mbuf *));
                /* Read timestamp of the packet */
                t_pack.tv_usec = m[idx]->udata64;
                t_pack.tv_sec = m[idx]->tx_offload;

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
                        sprintf(file_name_move, "%s%s", file_name_rotated, "ready.pcap");  
                        if (rename (file_name_rotated, file_name_move))
                        PRINT_INFO("\n failed to rename file %s\n", file_name_rotated); 
                        /* Open pcap file for writing */
                        sprintf(file_name_rotated, "%s%ld", file_name, last_rotation);
                        pd = pcap_open_dead(DLT_EN10MB, snaplen);
                        pcap_file_p = pcap_dump_open(pd, file_name_rotated);
                        if(pcap_file_p==NULL)
                                FATAL_ERROR("Error in opening pcap file\n");
                     //   PRINT_INFO("Opened file %s\n", file_name_rotated);
                }

                /* Compile pcap header */
                pcap_hdr.ts = t_pack;
                pcap_hdr.caplen = (rte_pktmbuf_data_len(m[idx]) < snaplen) ? rte_pktmbuf_data_len(m[idx]): snaplen;
                pcap_hdr.len = m[idx]->pkt_len;
                //packet = rte_pktmbuf_mtod(m[idx], u_char *);

                /* Write on pcap */
                pcap_dump ((u_char *)pcap_file_p, & pcap_hdr,  rte_pktmbuf_mtod(m[idx], u_char *));
                nb_dumped_packets++;

                /* Free the buffer */
                rte_pktmbuf_free( (struct rte_mbuf *)m[idx]);
                m[idx] = NULL;
              }
        }
}

void print_stats (void){
        
    static time_t curT, prevT ;//= last_rotation;
    struct timeval t_pack; 
    int ret;
    ret = gettimeofday(&t_pack, NULL); 
    if (ret != 0) FATAL_ERROR("Error: gettimeofday failed. Quitting...\n"); 
    curT = t_pack.tv_sec;
    struct tm * curTimeInfo;
    char TimeBuf[300];
    curTimeInfo = localtime(&curT);
    strftime(TimeBuf, 100, "%F  %T", curTimeInfo);
    static long long int prvrecevied = 0 , prvdrop = 0, prvprocessed = 0;  

        /* Print the statistics out */
        //PRINT_INFO("%d packets received by filter\n", m_pcapstatus.ps_recv);
        //PRINT_INFO("%d packets dropped by kernel\n", m_pcapstatus.ps_drop);
        //PRINT_INFO("%d packets dropped by network/driver\n", m_pcapstatus.ps_ifdrop);
       // PRINT_INFO("%lld Packets queued for write opt\n", (m_numberofpackets - prvprocessed)/INTERVAL_STATS); 

	FILE *f = fopen("DumperStat.log", "a+");
	if (f == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}

        
 

	struct rte_eth_stats stat; 
	int i; 
	long int  good_pkt = 0, miss_pkt = 0, st_pktproc =0, st_missout =0, timeInt = 0; 
        st_pktproc = m_numberofpackets;
        st_missout = missedouts;

	/* Print per port stats */ 
	for (i = 0; i < nb_sys_ports; i++){	 
		rte_eth_stats_get(i, &stat); 
		good_pkt += stat.ipackets; 
		miss_pkt += stat.imissed; 
		//printf("\nPORT: %2d Rx: %ld Drp: %ld Tot: %ld Perc: %.3f%%", i, stat.ipackets, stat.imissed, stat.ipackets+stat.imissed, (float)stat.imissed/(stat.ipackets+stat.imissed)*100 ); 
                //rte_eth_stats_reset ( i ); 
	}
        // Clean before intrrupt 
        //m_numberofpackets = 0;
        missedouts = 0;
         timeInt = curT - prevT;
        PRINT_INFO("Packet Capture Statistics:\n");
	printf("\n-------------------------------------------------"); 
	printf("\nTOT:     Rx: %ld Drp: %ld Tot: %ld Perc: %.3f%%", good_pkt, miss_pkt, good_pkt+miss_pkt, (float)miss_pkt/(good_pkt+miss_pkt)*100 ); 
	printf("\n"); 
        fprintf(f, "Splunk %s Appname=FBMDump pktrecv=%lld pktdrop=%lld  pktprocss=%lld \n ", TimeBuf, 
                (good_pkt - prvrecevied)/timeInt, (miss_pkt - prvdrop)/timeInt, (st_pktproc - prvprocessed)/timeInt);           
        //fprintf(f, "Splunk %s Appname=FBMDump pktrecv=%ld pktdrop=%ld  pktprocss=%ld \n ", TimeBuf,
          //      good_pkt/INTERVAL_STATS, miss_pkt/INTERVAL_STATS, st_pktproc/INTERVAL_STATS);
        fprintf(f,"Missedby enqueue%ld\n",st_missout); 
        prvrecevied = good_pkt;
        prvdrop = miss_pkt;
        prvprocessed = st_pktproc;
        prevT = curT;
	fclose(f);
     
}

static  int Statistics_lcore(__attribute__((unused)) void * arg){
        /* Create handler for SIGINT for CTRL + C closing and SIGALRM to print stats*/
        signal(SIGINT, sig_handler);
        //signal(SIGALRM, alarm_routine);

        //alarm(1);
         rte_eal_alarm_set(INTERVAL_STATS * MS_PER_S, alarm_routine, NULL);
        while(1)
        {
         usleep(10);
        }
        return 0; 
}

static void alarm_routine (__rte_unused void *param){

        /* If the program is quitting don't print anymore */
        if(do_shutdown) return;

        /* Print per port stats */
        print_stats();

        /* Schedule an other print */
        //alarm(INTERVAL_STATS);
        //signal(SIGALRM, alarm_routine);
        rte_eal_alarm_set(INTERVAL_STATS * US_PER_S, alarm_routine, NULL);
        
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
                sprintf(file_name_move, "%s%s", file_name_rotated, "ready.pcap");
                        if (rename (file_name_rotated, file_name_move))
                printf("\n failed to rename file %s\n", file_name_rotated);
                exit(0);
        }
}


static int parse_args(int argc, char **argv)
{
        int option;


        /* Retrive arguments */
        while ((option = getopt(argc, argv,"w:c:B:G:W:C:S:i:f:b:q:p:")) != -1) {
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
                                        /* portmask */
			case 'p':
				  l2fwd_enabled_port_mask = l2fwd_parse_portmask(optarg);
				  if (l2fwd_enabled_port_mask == 0) {
					  printf("invalid portmask\n");
					  return -1;
				  }
				  break;

				  /* nqueue */
			case 'q':
				  l2fwd_rx_queue_per_lcore = l2fwd_parse_nqueue(optarg);
				  if (l2fwd_rx_queue_per_lcore == 0) {
					  printf("invalid queue number\n");
					  return -1;
				  }
				  break;
				  /* long options */
			case 0:
				  return -1;

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


static int
l2fwd_parse_portmask(const char *portmask)
{
        char *end = NULL;
        unsigned long pm;

        /* parse hexadecimal string */
        pm = strtoul(portmask, &end, 16);
        if ((portmask[0] == '\0') || (end == NULL) || (*end != '\0'))
                return -1;

        if (pm == 0)
                return -1;

        return pm;
}

static unsigned int
l2fwd_parse_nqueue(const char *q_arg)
{
        char *end = NULL;
        unsigned long n;

        /* parse hexadecimal string */
        n = strtoul(q_arg, &end, 10);
        if ((q_arg[0] == '\0') || (end == NULL) || (*end != '\0'))
                return 0;
        if (n == 0)
                return 0;
        if (n >= MAX_RX_QUEUE_PER_LCORE)
                return 0;

        return n;
}

/* Check the link status of all ports in up to 9s, and print them finally */
static void
check_all_ports_link_status(uint8_t port_num, uint32_t port_mask)
{
#define CHECK_INTERVAL 100 /* 100ms */
#define MAX_CHECK_TIME 90 /* 9s (90 * 100ms) in total */
        uint8_t portid, count, all_ports_up, print_flag = 0;
        struct rte_eth_link link;
        int i =0;
        printf("\nChecking link status");
        fflush(stdout);
        for (count = 0; count <= MAX_CHECK_TIME; count++) {
                all_ports_up = 1;
                for (portid = 0; portid < port_num; portid++) {
                        
                                
                        if ((port_mask & (1 << portid)) == 0)
                                continue;
                        memset(&link, 0, sizeof(link));
                        rte_eth_link_get_nowait(portid, &link);
                        /* print link status if flag set */
                        if (print_flag == 1) {
                                if (link.link_status)
                                   {   
                                        printf("Port %d Link Up - speed %u "
                                                "Mbps - %s\n", (uint8_t)portid,
                                                (unsigned)link.link_speed,
                                (link.link_duplex == ETH_LINK_FULL_DUPLEX) ?
                                        ("full-duplex") : ("half-duplex\n"));
                                         readportid[i++] = portid;
                                         en_sys_ports = i;
                                   }
                                else
                                        printf("Port %d Link Down\n",
                                                (uint8_t)portid);
                                continue;
                        }
                        /* clear all_ports_up flag if any link down */
                        if (link.link_status == ETH_LINK_DOWN) {
                                all_ports_up = 0;
                                break;
                        }
                }
                /* after finally printing all link status, get out */
                if (print_flag == 1)
                        break;

                if (all_ports_up == 0) {
                        printf(".");
                        fflush(stdout);
                        rte_delay_ms(CHECK_INTERVAL);
                }

                /* set the print_flag if all ports up or timeout */
                if (all_ports_up == 1 || count == (MAX_CHECK_TIME - 1)) {
                        print_flag = 1;
                        printf("done\n");
                }
        }
}


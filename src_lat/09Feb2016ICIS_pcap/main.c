/*-
 *   BSD LICENSE
 *
 *   Copyright(c) 2010-2014 Intel Corporation. All rights reserved.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <netinet/in.h>
#include <setjmp.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>

#include <rte_common.h>
#include <rte_log.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_memzone.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_launch.h>
#include <rte_atomic.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_per_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_interrupts.h>
#include <rte_pci.h>
#include <rte_random.h>
#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>

#include <pcap.h>

#define RTE_LOGTYPE_ICIS RTE_LOGTYPE_USER1
#define PRINT_INFO(fmt, args...)        RTE_LOG(INFO, ICIS, fmt "\n", ##args)
#define FATAL_ERROR(fmt, args...)       rte_exit(EXIT_FAILURE, fmt "\n", ##args)

#define NB_MBUF   8192

#define MAX_PKT_BURST 32
#define BURST_TX_DRAIN_US 100 /* TX drain every ~100us */
/*
 * App specific variables 
 */
#define IFSZ 16
#define FLTRSZ 120
#define MAXHOSTSZ 256
#define SNAP_LEN 65535 // Full packet reading
#define PCAPDBUF_LEN 819200 // 10 * 8192
unsigned long int m_buflen = 2097152; // 2 * 1024 * 1024
/* Global vars */
char * file_name = NULL;
pcap_dumper_t * pcap_file_p;
uint64_t max_packets = 0 ;
uint64_t buffer_size =   8192;
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
//static struct rte_mempool * pktmbuf_pool;
//static struct rte_ring    * intermediate_ring;
int snaplen =  SNAP_LEN;/* amount of data per packet */
char fliterstr[FLTRSZ];     /* bpf filter string */

void alarm_routine (__attribute__((unused)) int unused);





/*
 * Configurable number of RX/TX ring descriptors
 */
#define RTE_TEST_RX_DESC_DEFAULT 128
#define RTE_TEST_TX_DESC_DEFAULT 512
static uint16_t nb_rxd = RTE_TEST_RX_DESC_DEFAULT;
static uint16_t nb_txd = RTE_TEST_TX_DESC_DEFAULT;

/* ethernet addresses of ports */
static struct ether_addr icis_ports_eth_addr[RTE_MAX_ETHPORTS];

/* mask of enabled ports */
static uint32_t icis_enabled_port_mask = 0;

/* list of enabled ports */
static uint32_t icis_dst_ports[RTE_MAX_ETHPORTS];

static unsigned int icis_rx_queue_per_lcore = 1;

struct mbuf_table {
	unsigned len;
	struct rte_mbuf *m_table[MAX_PKT_BURST];
};

#define MAX_RX_QUEUE_PER_LCORE 16
#define MAX_TX_QUEUE_PER_PORT 16
struct lcore_queue_conf {
	unsigned n_rx_port;
	unsigned rx_port_list[MAX_RX_QUEUE_PER_LCORE];
	struct mbuf_table tx_mbufs[RTE_MAX_ETHPORTS];

} __rte_cache_aligned;
struct lcore_queue_conf lcore_queue_conf[RTE_MAX_LCORE];

static const struct rte_eth_conf port_conf = {
	.rxmode = {
		.split_hdr_size = 0,
		.header_split   = 0, /**< Header Split disabled */
		.hw_ip_checksum = 0, /**< IP checksum offload disabled */
		.hw_vlan_filter = 0, /**< VLAN filtering disabled */
		.jumbo_frame    = 0, /**< Jumbo Frame Support disabled */
		.hw_strip_crc   = 0, /**< CRC stripped by hardware */
	},
	.txmode = {
		.mq_mode = ETH_MQ_TX_NONE,
	},
};

struct rte_mempool * icis_pktmbuf_pool = NULL;
static struct rte_ring    * intermediate_ring = NULL;

/* Per-port statistics struct */
struct icis_port_statistics {
	uint64_t tx;
	uint64_t rx;
	uint64_t dropped;
} __rte_cache_aligned;
struct icis_port_statistics port_statistics[RTE_MAX_ETHPORTS];

/* A tsc-based timer responsible for triggering statistics printout */
#define TIMER_MILLISECOND 2000000ULL /* around 1ms at 2 Ghz */
#define MAX_TIMER_PERIOD 86400 /* 1 day max */
static int64_t timer_period = 10 * TIMER_MILLISECOND * 1000; /* default period is 10 seconds */

/* Print out statistics on packets dropped */
static void
print_stats(void)
{
	uint64_t total_packets_dropped, total_packets_tx, total_packets_rx;
	unsigned portid;

	total_packets_dropped = 0;
	total_packets_tx = 0;
	total_packets_rx = 0;

	const char clr[] = { 27, '[', '2', 'J', '\0' };
	const char topLeft[] = { 27, '[', '1', ';', '1', 'H','\0' };

		/* Clear screen and move to top left */
	printf("%s%s", clr, topLeft);

	printf("\nPort statistics ====================================");

	for (portid = 0; portid < RTE_MAX_ETHPORTS; portid++) {
		/* skip disabled ports */
		if ((icis_enabled_port_mask & (1 << portid)) == 0)
			continue;
		printf("\nStatistics for port %u ------------------------------"
			   "\nPackets sent: %24"PRIu64
			   "\nPackets received: %20"PRIu64
			   "\nPackets dropped: %21"PRIu64,
			   portid,
			   port_statistics[portid].tx,
			   port_statistics[portid].rx,
			   port_statistics[portid].dropped);

		total_packets_dropped += port_statistics[portid].dropped;
		total_packets_tx += port_statistics[portid].tx;
		total_packets_rx += port_statistics[portid].rx;
	}
	printf("\nAggregate statistics ==============================="
		   "\nTotal packets sent: %18"PRIu64
		   "\nTotal packets received: %14"PRIu64
		   "\nTotal packets dropped: %15"PRIu64,
		   total_packets_tx,
		   total_packets_rx,
		   total_packets_dropped);
	printf("\n====================================================\n");
}

/* main processing loop */
static void
packet_producer(void)
{
	struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
	struct rte_mbuf *m;
	unsigned lcore_id;
	unsigned i, j, portid, nb_rx; 
	struct lcore_queue_conf *qconf;
        struct timeval t_pack;


	lcore_id = rte_lcore_id();
	qconf = &lcore_queue_conf[lcore_id];

	if (qconf->n_rx_port == 0) {
		RTE_LOG(INFO, ICIS, "lcore %u has nothing to do\n", lcore_id);
		return;
	}

	RTE_LOG(INFO, ICIS, "entering main loop on lcore %u\n", lcore_id);

	for (i = 0; i < qconf->n_rx_port; i++) {

		portid = qconf->rx_port_list[i];
		RTE_LOG(INFO, ICIS, " -- lcoreid=%u portid=%u\n", lcore_id,
			portid);
	}

	while (1) {


		/*
		 * Read packet from RX queues
		 */
		for (i = 0; i < qconf->n_rx_port; i++) {

			portid = qconf->rx_port_list[i];
			nb_rx = rte_eth_rx_burst((uint8_t) portid, 0,
						 pkts_burst, MAX_PKT_BURST);

			port_statistics[portid].rx += nb_rx;

			for (j = 0; j < nb_rx; j++) {
				m = pkts_burst[j];
                                gettimeofday(&t_pack, NULL);
                                m->tx_offload = t_pack.tv_sec;
                                m->udata64 =  t_pack.tv_usec;
				rte_prefetch0(rte_pktmbuf_mtod(m, void *));
                                rte_ring_enqueue (intermediate_ring, m);
				//icis_simple_forward(m, portid);
			}
		}
	}
}

static int
icis_launch_one_lcore(__attribute__((unused)) void *dummy)
{
	packet_producer();
	return 0;
}

/* display usage */
static void
icis_usage(const char *prgname)
{
	printf("%s [EAL options] -- -p PORTMASK [-q NQ]\n"
	       "  -p PORTMASK: hexadecimal bitmask of ports to configure\n"
	       "  -q NQ: number of queue (=ports) per lcore (default is 1)\n"
		   "  -T PERIOD: statistics will be refreshed each PERIOD seconds (0 to disable, 10 default, 86400 maximum)\n",
	       prgname);
}

static int
icis_parse_portmask(const char *portmask)
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
icis_parse_nqueue(const char *q_arg)
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

static int
icis_parse_timer_period(const char *q_arg)
{
	char *end = NULL;
	int n;

	/* parse number string */
	n = strtol(q_arg, &end, 10);
	if ((q_arg[0] == '\0') || (end == NULL) || (*end != '\0'))
		return -1;
	if (n >= MAX_TIMER_PERIOD)
		return -1;

	return n;
}

/* Parse the argument given in the command line of the application */
static int
icis_parse_args(int argc, char **argv)
{
	int opt, ret;
	char **argvopt;
	int option_index;
	char *prgname = argv[0];
	static struct option lgopts[] = {
		{NULL, 0, 0, 0}
	};

	argvopt = argv;

	while ((opt = getopt_long(argc, argvopt, "p:q:T:w:c:B:G:W:C:S:f:b:",
				  lgopts, &option_index)) != EOF) {

		switch (opt) {
		/* portmask */
		case 'p':
			icis_enabled_port_mask = icis_parse_portmask(optarg);
			if (icis_enabled_port_mask == 0) {
				printf("invalid portmask\n");
				icis_usage(prgname);
				return -1;
			}
			break;

		/* nqueue */
		case 'q':
			icis_rx_queue_per_lcore = icis_parse_nqueue(optarg);
			if (icis_rx_queue_per_lcore == 0) {
				printf("invalid queue number\n");
				icis_usage(prgname);
				return -1;
			}
			break;

		/* timer period */
		case 'T':
			timer_period = icis_parse_timer_period(optarg) * 1000 * TIMER_MILLISECOND;
			if (timer_period < 0) {
				printf("invalid timer period\n");
				icis_usage(prgname);
				return -1;
			}
			break;

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
                case 'f': strcpy(fliterstr, optarg); /* BFS filter */
                                break;
                case 'b': m_buflen = atoi(optarg);
                                break;

		/* long options */
		case 0:
			icis_usage(prgname);
			return -1;

		default:
			icis_usage(prgname);
			return -1;
		}
	}

	if (optind >= 0)
		argv[optind-1] = prgname;

	ret = optind-1;
	optind = 0; /* reset getopt lib */
	return ret;
}

/* Check the link status of all ports in up to 9s, and print them finally */
static void
check_all_ports_link_status(uint8_t port_num, uint32_t port_mask)
{
#define CHECK_INTERVAL 100 /* 100ms */
#define MAX_CHECK_TIME 90 /* 9s (90 * 100ms) in total */
	uint8_t portid, count, all_ports_up, print_flag = 0;
	struct rte_eth_link link;

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
					printf("Port %d Link Up - speed %u "
						"Mbps - %s\n", (uint8_t)portid,
						(unsigned)link.link_speed,
				(link.link_duplex == ETH_LINK_FULL_DUPLEX) ?
					("full-duplex") : ("half-duplex\n"));
				else
					printf("Port %d Link Down\n",
						(uint8_t)portid);
				continue;
			}
			/* clear all_ports_up flag if any link down */
			if (link.link_status == 0) {
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


/* Signal handling function */
static void sig_handler(int signo)
{
        uint64_t diff;
        int ret;
        struct timeval t_end;

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
                exit(0);
        }
}


void alarm_routine (__attribute__((unused)) int unused){

        /* If the program is quitting don't print anymore */
        if(do_shutdown) return;

        /* Print per port stats */
        print_stats();

        /* Schedule an other print */
        alarm(10);
        //signal(SIGALRM, alarm_routine);

}


static int packet_consumer(__attribute__((unused)) void * arg){

        struct timeval t_pack;
        struct rte_mbuf * m;
        u_char * packet;
        char file_name_rotated [1000];
        int ret;
        struct pcap_pkthdr pcap_hdr;

        /* Open pcap file for writing */
        pd = pcap_open_dead(DLT_EN10MB, 65535 );
        pcap_file_p = pcap_dump_open(pd, file_name);
        if(pcap_file_p==NULL)
                FATAL_ERROR("Error in opening pcap file\n");
        printf("Opened file %s\n", file_name);

        /* Init first rotation */
        ret = gettimeofday(&t_pack, NULL);
        if (ret != 0) FATAL_ERROR("Error: gettimeofday failed. Quitting...\n");
        last_rotation = t_pack.tv_sec;
        start_secs = t_pack.tv_sec;
        //Print stats
        alarm_routine(0);
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

                        /* Open pcap file for writing */
                        sprintf(file_name_rotated, "%s%ld", file_name, nb_rotations);
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


int
main(int argc, char **argv)
{
	struct lcore_queue_conf *qconf;
	struct rte_eth_dev_info dev_info;
	int ret;
	uint8_t nb_ports;
	uint8_t nb_ports_available;
	uint8_t portid, last_port;
	unsigned lcore_id, rx_lcore_id;
	unsigned nb_ports_in_mask = 0;
        /* Create handler for SIGINT for CTRL + C closing and SIGALRM to print stats*/
        signal(SIGINT, sig_handler);
        signal(SIGALRM, alarm_routine);

	/* init EAL */
	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Invalid EAL arguments\n");
	argc -= ret;
	argv += ret;

	/* parse application arguments (after the EAL ones) */
	ret = icis_parse_args(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Invalid ICIS arguments\n");

	/* create the mbuf pool */
	icis_pktmbuf_pool = rte_pktmbuf_pool_create("mbuf_pool", buffer_size, 32,
		0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
	if (icis_pktmbuf_pool == NULL)
		rte_exit(EXIT_FAILURE, "Cannot init mbuf pool\n");
   
        /* Create intermediate RING */
        intermediate_ring = rte_ring_create ("intermedate_ring", buffer_size, rte_socket_id(), RING_F_SP_ENQ | RING_F_SC_DEQ );
        if (intermediate_ring == NULL ) FATAL_ERROR("Cannot create ring");

	nb_ports = rte_eth_dev_count();
	if (nb_ports == 0)
		rte_exit(EXIT_FAILURE, "No Ethernet ports - bye\n");

	if (nb_ports > RTE_MAX_ETHPORTS)
		nb_ports = RTE_MAX_ETHPORTS;

	/* reset icis_dst_ports */
	for (portid = 0; portid < RTE_MAX_ETHPORTS; portid++)
		icis_dst_ports[portid] = 0;
	last_port = 0;

	/*
	 * Each logical core is assigned a dedicated TX queue on each port.
	 */
	for (portid = 0; portid < nb_ports; portid++) {
		/* skip ports that are not enabled */
		if ((icis_enabled_port_mask & (1 << portid)) == 0)
			continue;

		if (nb_ports_in_mask % 2) {
			icis_dst_ports[portid] = last_port;
			icis_dst_ports[last_port] = portid;
		}
		else
			last_port = portid;

		nb_ports_in_mask++;

		rte_eth_dev_info_get(portid, &dev_info);
	}
	if (nb_ports_in_mask % 2) {
		printf("Notice: odd number of ports in portmask.\n");
		icis_dst_ports[last_port] = last_port;
	}

	rx_lcore_id = 0;
	qconf = NULL;

	/* Initialize the port/queue configuration of each logical core */
	for (portid = 0; portid < nb_ports; portid++) {
		/* skip ports that are not enabled */
		if ((icis_enabled_port_mask & (1 << portid)) == 0)
			continue;

		/* get the lcore_id for this port */
		while (rte_lcore_is_enabled(rx_lcore_id) == 0 ||
		       lcore_queue_conf[rx_lcore_id].n_rx_port ==
		       icis_rx_queue_per_lcore) {
			rx_lcore_id++;
			if (rx_lcore_id >= RTE_MAX_LCORE)
				rte_exit(EXIT_FAILURE, "Not enough cores\n");
		}

		if (qconf != &lcore_queue_conf[rx_lcore_id])
			/* Assigned a new logical core in the loop above. */
			qconf = &lcore_queue_conf[rx_lcore_id];

		qconf->rx_port_list[qconf->n_rx_port] = portid;
		qconf->n_rx_port++;
		printf("Lcore %u: RX port %u\n", rx_lcore_id, (unsigned) portid);
	}

	nb_ports_available = nb_ports;

	/* Initialise each port */
	for (portid = 0; portid < nb_ports; portid++) {
		/* skip ports that are not enabled */
		if ((icis_enabled_port_mask & (1 << portid)) == 0) {
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

		rte_eth_macaddr_get(portid,&icis_ports_eth_addr[portid]);

		/* init one RX queue */
		fflush(stdout);
		ret = rte_eth_rx_queue_setup(portid, 0, nb_rxd,
					     rte_eth_dev_socket_id(portid),
					     NULL,
					     icis_pktmbuf_pool);
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
				icis_ports_eth_addr[portid].addr_bytes[0],
				icis_ports_eth_addr[portid].addr_bytes[1],
				icis_ports_eth_addr[portid].addr_bytes[2],
				icis_ports_eth_addr[portid].addr_bytes[3],
				icis_ports_eth_addr[portid].addr_bytes[4],
				icis_ports_eth_addr[portid].addr_bytes[5]);

		/* initialize port stats */
		memset(&port_statistics, 0, sizeof(port_statistics));
	}

	if (!nb_ports_available) {
		rte_exit(EXIT_FAILURE,
			"All available ports are disabled. Please set portmask.\n");
	}

	check_all_ports_link_status(nb_ports, icis_enabled_port_mask);

	/* launch per-lcore init on every lcore */
	rte_eal_mp_remote_launch(icis_launch_one_lcore, NULL, SKIP_MASTER);
        packet_consumer(NULL);
	RTE_LCORE_FOREACH_SLAVE(lcore_id) {
		if (rte_eal_wait_lcore(lcore_id) < 0)
			return -1;
	}
//        packet_consumer(NULL);
	return 0;
}

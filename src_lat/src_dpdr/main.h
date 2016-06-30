
#ifndef _MAIN_H
#define _MAIN_H

/* Includes */
#define _GNU_SOURCE
#include <pcap/pcap.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>
#include <libgen.h>
#include <sys/queue.h>
#include <sys/syscall.h>
#include <math.h>
#include <sched.h>
#include <pthread.h>
#include <unistd.h>

#include <rte_common.h>
#include <rte_log.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_memzone.h>
#include <rte_tailq.h>
#include <rte_errno.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_launch.h>
#include <rte_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_interrupts.h>
#include <rte_pci.h>
#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_ring.h>
#include <rte_log.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_string_fns.h>
#include <rte_cycles.h>
#include <rte_atomic.h>
#include <rte_version.h>
#include <rte_eth_ring.h>
#include <rte_alarm.h>
#include <fcntl.h>

/* Useful macro for error handling */
#define RTE_LOGTYPE_FBM RTE_LOGTYPE_USER1
#define PRINT_INFO(fmt, args...)        RTE_LOG(INFO, FBM, fmt "\n", ##args)
#define FATAL_ERROR(fmt, args...)       rte_exit(EXIT_FAILURE, fmt "\n", ##args)
#define MAX_PKT_BURST 4096 
#define MAX_PORT 16
#define MAX_RX_QUEUE_PER_LCORE 16
#define MAX_TX_QUEUE_PER_PORT 16
#define RTE_TEST_RX_DESC_DEFAULT 4096
#define RTE_TEST_TX_DESC_DEFAULT 32
struct lcore_queue_conf {
        unsigned n_rx_port;
        unsigned rx_port_list[MAX_RX_QUEUE_PER_LCORE];
} __rte_cache_aligned;
struct lcore_queue_conf lcore_queue_conf[RTE_MAX_LCORE];
//#endif //_COMMON_H

#define INTERVAL_STATS 1 


//All funtions have to be declared here 
int packet_producer(__attribute__((unused)) void * arg);
inline int FlushToFile(__rte_unused void *param);
inline void createNewFile(char * filename, int snaplen);
int portinit(int portid);
void check_all_ports_link_status(uint8_t port_num, uint32_t port_mask);
int parse_args(int argc, char **argv);

int l2fwd_parse_portmask(const char *portmask);
unsigned int l2fwd_parse_nqueue(const char *q_args);
int isPowerOfTwo (unsigned int x);
void sig_handler(int signo);
//static void init_port(int i);
void print_stats (void);
//void alarm_routine (__attribute__((unused)) int unused);
int packet_consumer(__attribute__((unused)) void * arg);
int Statistics_lcore(__attribute__((unused)) void * arg);


#endif

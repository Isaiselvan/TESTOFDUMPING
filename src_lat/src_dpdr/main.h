/*
*
* Copyright (c) 2015
*      Politecnico di Torino.  All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* For bug report and other information please write to:
* martino.trevisan@studenti.polito.it
*
*
*/


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

/* Useful macro for error handling */
#define FATAL_ERROR(fmt, args...)       rte_exit(EXIT_FAILURE, fmt "\n", ##args)
#define INTERVAL_STATS 1 
/* Function prototypes */
static int packet_producer(__attribute__((unused)) void * arg);
static void sig_handler(int signo);
//static void init_port(int i);
static int parse_args(int argc, char **argv);
void print_stats (void);
void alarm_routine (__attribute__((unused)) int unused);
static int packet_consumer(__attribute__((unused)) void * arg);
static int Statistics_lcore(__attribute__((unused)) void * arg);
int isPowerOfTwo (unsigned int x);


// Dpdk driver init code

#define MAX_PKT_BURST 4096 
#define BURST_TX_DRAIN_US 100 /* TX drain every ~100us */

/*
 *  * Configurable number of RX/TX ring descriptors
 *   */
#define RTE_TEST_RX_DESC_DEFAULT 4096
#define RTE_TEST_TX_DESC_DEFAULT 32
static uint16_t nb_rxd = RTE_TEST_RX_DESC_DEFAULT;
static uint16_t nb_txd = RTE_TEST_TX_DESC_DEFAULT;
/* mask of enabled ports */
static uint32_t l2fwd_enabled_port_mask = 0;
/* ethernet addresses of ports */
static struct ether_addr l2fwd_ports_eth_addr[RTE_MAX_ETHPORTS];
static unsigned int l2fwd_rx_queue_per_lcore = 1;

static int l2fwd_parse_portmask(const char *portmask);
static unsigned int l2fwd_parse_nqueue(const char *q_args);
static void check_all_ports_link_status(uint8_t port_num, uint32_t port_mask);
/* RSS symmetrical 40 Byte seed, according to "Scalable TCP Session Monitoring with Symmetric Receive-side Scaling" (Shinae Woo, KyoungSoo Park from KAIST)  */
uint8_t rss_seed [] = { 0x6d, 0x5a, 0x6d, 0x5a, 0x6d, 0x5a, 0x6d, 0x5a,
                        0x6d, 0x5a, 0x6d, 0x5a, 0x6d, 0x5a, 0x6d, 0x5a,
                        0x6d, 0x5a, 0x6d, 0x5a, 0x6d, 0x5a, 0x6d, 0x5a,
                        0x6d, 0x5a, 0x6d, 0x5a, 0x6d, 0x5a, 0x6d, 0x5a,
                        0x6d, 0x5a, 0x6d, 0x5a, 0x6d, 0x5a, 0x6d, 0x5a
};

/* This seed is to load balance only respect source IP, according to me (Martino Trevisan, from nowhere particular) */
uint8_t rss_seed_src_ip [] = {
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
/* This seed is to load balance only destination source IP, according to me (Martino Trevisan, from nowhere particular) */
uint8_t rss_seed_dst_ip [] = {
                        0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


/* Struct for devices configuration for const defines see rte_ethdev.h */
static const struct rte_eth_conf port_conf = {
        .rxmode = {
                .mq_mode = ETH_MQ_RX_RSS,       /* Enable RSS */
                .split_hdr_size = 0,
                .header_split   = 0, /*< Header Split disabled */
               .hw_ip_checksum = 0, /**< IP checksum offload disabled */
              .hw_vlan_filter = 0, /**< VLAN filtering disabled */
            .jumbo_frame    = 0, /**< Jumbo Frame Support disabled */
          .hw_strip_crc   = 0, /**< CRC stripped by hardware */
        },
        .txmode = {
                .mq_mode = ETH_MQ_TX_NONE,
        },
        .rx_adv_conf = {
                .rss_conf = {
                        .rss_key = rss_seed_dst_ip,//rss_seed,                            /* Set the seed,                                        */
                        .rss_key_len = 40,                              /* and the seed length.                                 */
                        .rss_hf = (ETH_RSS_TCP | ETH_RSS_UDP) , /* Set the mask of protocols RSS will be applied to     */
                }
        }
};
/*
static const struct rte_eth_conf port_conf = {
        .rxmode = {
                .split_hdr_size = 0,
                .header_split   = 0, *< Header Split disabled */
 //               .hw_ip_checksum = 0, /**< IP checksum offload disabled */
   //             .hw_vlan_filter = 0, /**< VLAN filtering disabled */
     //           .jumbo_frame    = 0, /**< Jumbo Frame Support disabled */
       //         .hw_strip_crc   = 0, /**< CRC stripped by hardware */
       // },
        //.txmode = {
          //      .mq_mode = ETH_MQ_TX_NONE,
        //},
//};*/

/* Struct for configuring each rx queue. These are default values */
static const struct rte_eth_rxconf rx_conf = {
        .rx_thresh = {
                .pthresh = 8,   /* Ring prefetch threshold */
                .hthresh = 8,   /* Ring host threshold */
                .wthresh = 4,   /* Ring writeback threshold */
        },
        .rx_free_thresh = 32,    /* Immediately free RX descriptors */
};

#define MAX_RX_QUEUE_PER_LCORE 16
#define MAX_TX_QUEUE_PER_PORT 16
struct lcore_queue_conf {
        unsigned n_rx_port;
        unsigned rx_port_list[MAX_RX_QUEUE_PER_LCORE];
} __rte_cache_aligned;
struct lcore_queue_conf lcore_queue_conf[RTE_MAX_LCORE];

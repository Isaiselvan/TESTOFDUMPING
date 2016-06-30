#include "main.h"

extern uint32_t l2fwd_enabled_port_mask;
extern uint8_t nb_ports_available;
extern int readportid[MAX_PORT];
extern int en_sys_ports;
extern struct rte_ring    * intermediate_ring;
extern struct rte_mempool * pktmbuf_pool;
static struct ether_addr l2fwd_ports_eth_addr[RTE_MAX_ETHPORTS];
/*
 *  * Configurable number of RX/TX ring descriptors
 *   */
static uint16_t nb_rxd = RTE_TEST_RX_DESC_DEFAULT;
static uint16_t nb_txd = RTE_TEST_TX_DESC_DEFAULT;

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


static const struct rte_eth_conf port_conf = {
        .rxmode = {
                .mq_mode = ETH_MQ_RX_RSS,       /* Enable RSS */
                .split_hdr_size = 0,
                .header_split   = 0, /*< Header Split disabled */
               .hw_ip_checksum = 0, /**< IP checksum offload disabled */
              .hw_vlan_filter = 1, /**< VLAN filtering disabled */
            .jumbo_frame    = 0, /**< Jumbo Frame Support disabled */
          .hw_strip_crc   = 0, /**< CRC stripped by hardware */
        },
        .txmode = {
                .mq_mode = ETH_MQ_TX_NONE,
        },
        .rx_adv_conf = {
                .rss_conf = {

                        //.rss_key = rss_seed,//rss_seed,                            /* Set the seed,                                        */
                        .rss_key = NULL,
                        //.rss_key_len = 40,                              /* and the seed length.                                 */
                        .rss_hf = (ETH_RSS_TCP | ETH_RSS_UDP) , /* Set the mask of protocols RSS will be applied to     */
                }
        }
};
/* Struct for devices configuration for const defines see rte_ethdev.h */
#if 0
static const struct rte_eth_conf port_conf = {
               .rxmode = {
                         .mq_mode = ETH_MQ_RX_RSS,
                         },
               .rx_adv_conf = {
                         .rss_conf ={
                                       .rss_key = NULL,
                                       .rss_hf = (ETH_RSS_UDP | ETH_RSS_TCP),
                                    }
                              }
 };
#endif
 //               port_conf.rxmode.mq_mode = ETH_MQ_RX_RSS;
   //             port_conf.rx_adv_conf.rss_conf.rss_key = NULL;
     //           port_conf.rx_adv_conf.rss_conf.rss_hf = ETH_RSS_UDP | ETH_RSS_TCP;
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
        .rx_free_thresh = 3072,    /* Immediately free RX descriptors */
};


int portinit(int portid)
{
 int ret ;
 /* skip ports that are not enabled */
                if ((l2fwd_enabled_port_mask & (1 << portid)) == 0) {
                        printf("Skipping disabled port %u\n", (unsigned) portid);
                        nb_ports_available--;
                        return 0;
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

              return 0;
}

/* Check the link status of all ports in up to 9s, and print them finally */
void
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


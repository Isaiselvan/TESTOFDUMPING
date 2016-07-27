
#include <pcap.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "main.h"

/* Constants of the system */
#define MEMPOOL_NAME "cluster_mem_pool"                         // Name of the NICs' mem_pool
#define MEMPOOL_ELEM_SZ  256                                    // Power of two greater than 1500
#define MEMPOOL_CACHE_SZ 512 
#define INTERMEDIATERING_NAME "intermedate_ring"






//extern int parse_args(int argc, char **argv);
extern int nb_sys_ports;
/* Global vars */
//uint64_t sz_dumped_file = 0;
//static int test1=0, test2=0, test3=0, test4 =0;
/* Main function */
#define SNAP_LEN 512 // Full packet reading to get host info on Sandbox min is 512
int snaplen =  RTE_MBUF_DEFAULT_DATAROOM;/* amount of data per packet */
uint64_t buffer_size = 1048576 ; //Ring size
// Dpdk driver init code
struct rte_ring    * intermediate_ring;
struct rte_mempool * pktmbuf_pool;

/* mask of enabled ports */
extern uint32_t l2fwd_enabled_port_mask ;// = 0;
/* ethernet addresses of ports */
extern unsigned int l2fwd_rx_queue_per_lcore;// = 1;
extern int nb_Wlcore;
extern int Wlcore_list[RTE_MAX_LCORE]; 


uint8_t nb_ports_available;

int main(int argc, char **argv)
{
        int ret;
        uint8_t nb_ports, lcore_count;
        uint8_t portid ;
	//struct lcore_queue_conf *qconf;
	int lcore_id;// rx_lcore_id;
        //int i;


        /* Initialize DPDK enviroment with args, then shift argc and argv to get application parameters */
        ret = rte_eal_init(argc, argv);
        if (ret < 0) FATAL_ERROR("Cannot init EAL\n");
        argc -= ret;
        argv += ret;

        /* Check if this application can use three cores*/
        lcore_count = rte_lcore_count ();
        if (lcore_count < 3 ) FATAL_ERROR("This application needs minimum (3) cores.");

        /* Parse arguments */
        ret = parse_args(argc, argv);
        if (ret < 0) FATAL_ERROR("Wrong arguments\n");

        nb_ports = rte_eth_dev_count();
        nb_sys_ports = nb_ports;
        if (nb_ports == 0)
          rte_exit(EXIT_FAILURE, "No Ethernet ports - bye\n");          
 
        //store_ring = rte_ring_create ("Store_ring",  buffer_size ,rte_socket_id(), RING_F_SP_ENQ | RING_F_SC_DEQ );
          //if (store_ring == NULL ) FATAL_ERROR("Cannot create store ring ");
 
       
        /* Start consumer and producer routine on 2 different cores: consumer launched first... */
        //ret =  rte_eal_mp_remote_launch (packet_consumer, NULL, SKIP_MASTER);
        //if (ret != 0) FATAL_ERROR("Cannot start consumer thread\n");
        //rx_lcore_id = 0;
        //qconf = NULL;
        /* Initialize the port/queue configuration of each logical core */
#if 0
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
#endif 
       nb_ports_available = nb_ports;

        if (check_lcore_params() < 0)
                rte_exit(EXIT_FAILURE, "check_lcore_params failed\n");

       pktmbuf_pool = NULL;
       int msize = buffer_size;
       int Prooption = MEMPOOL_F_SP_PUT, Conoption = MEMPOOL_F_SC_GET;
       int rxlcorelt[lcore_count], rxlcore_count;
       rxlcore_count = get_nb_rx_lcores(&rxlcorelt[0]);
       if(rxlcore_count > 1)
             Prooption = 0;
         
       if( nb_Wlcore > 1)
             Conoption = 0;  
 
       while (pktmbuf_pool == NULL)
       { 
       // pktmbuf_pool = rte_mempool_create(MEMPOOL_NAME, msize /*(nb_ports * 1 * RTE_TEST_RX_DESC_DEFAULT)*/, snaplen /*RTE_MBUF_DEFAULT_DATAROOM */ /*(snaplen + 128 + RTE_PKTMBUF_HEADROOM)*/, MEMPOOL_CACHE_SZ, sizeof(struct rte_pktmbuf_pool_private), rte_pktmbuf_pool_init, NULL, rte_pktmbuf_init, NULL,rte_socket_id(), Prooption + Conoption);//MEMPOOL_F_SP_PUT | MEMPOOL_F_SC_GET);
        pktmbuf_pool = rte_pktmbuf_pool_create(MEMPOOL_NAME, msize, MEMPOOL_CACHE_SZ, 0, snaplen/*RTE_MBUF_DEFAULT_DATAROOM *//* RTE_PKTMBUF_HEADROOM MEMPOOL_ELEM_SZ*/, SOCKET_ID_ANY);
        //if (pktmbuf_pool == NULL) FATAL_ERROR("Cannot create cluster_mem_pool. Errno: %d [ENOMEM: %d, ENOSPC: %d, E_RTE_NO_TAILQ: %d, E_RTE_NO_CONFIG: %d, E_RTE_SECONDARY: %d, EINVAL: %d, EEXIST: %d]\n", rte_errno, ENOMEM, ENOSPC, RTE_MAX_TAILQ/*E_RTE_NO_TAILQ*/, E_RTE_NO_CONFIG, E_RTE_SECONDARY, EINVAL, EEXIST  );
        //
          msize = msize - 128;      
        }
         PRINT_INFO("MemPool Size allocated %d  size %d cachesize %d\n", (msize + 128), RTE_MBUF_DEFAULT_DATAROOM , MEMPOOL_CACHE_SZ);  
         PRINT_INFO("Test %d %d  %d\n", MEMPOOL_F_SP_PUT, MEMPOOL_F_SC_GET, (MEMPOOL_F_SP_PUT | MEMPOOL_F_SC_GET)); 
        Prooption = RING_F_SP_ENQ;
        Conoption = RING_F_SC_DEQ;

        if(rxlcore_count > 1)
             Prooption = 0;

        if( nb_Wlcore > 1)
             Conoption = 0;

        /* Init intermediate queue data structures: the ring. */
        intermediate_ring = NULL;
        msize = buffer_size;
        while (intermediate_ring == NULL)
        {
        intermediate_ring = rte_ring_create (INTERMEDIATERING_NAME, msize ,rte_socket_id(), Prooption + Conoption );
        msize = msize / 2 ;     
          //if (intermediate_ring == NULL ) FATAL_ERROR("Cannot create ring");
        }
         PRINT_INFO("RteRing Size allocated%d\n" , (msize * 2));


        ret = init_lcore_rx_queues();
        if (ret < 0)
                rte_exit(EXIT_FAILURE, "init_lcore_rx_queues failed\n");

        if (check_port_config(nb_ports) < 0)
                rte_exit(EXIT_FAILURE, "check_port_config failed\n"); 

/* Initialise each port */
        for (portid = 0; portid < nb_ports; portid++){
                  ret = portinit(portid);
                 if (ret != 0) FATAL_ERROR("portinit failed\n"); 
         }

        if (!nb_ports_available) {
                rte_exit(EXIT_FAILURE,
                        "All available ports are disabled. Please set portmask.\n");
        }

        check_all_ports_link_status(nb_ports, l2fwd_enabled_port_mask);
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);
     
//Get the receiver lcore config
             printf("lcore_count = %d, rxlcore_count = %d, nb_Wlcore = %d \n", lcore_count,rxlcore_count,nb_Wlcore);  
            if((lcore_count - (rxlcore_count + nb_Wlcore))  < 1)
                   FATAL_ERROR("FBM: Need more core allocation\n");   
// Start All lcores according to the configuration        
        lcore_id = 0;
        int masterid = rte_lcore_id();
        int masterlcore = 0, lid; // 1 - pro, 2- con, 0 - stat  
        bool statsStarted = false;      
        int Statcore1 = 0, Statcore2 = 0;
        RTE_LCORE_FOREACH_SLAVE(lid) {
  
         Statcore1 = -2;
         Statcore2 = -3;
        for (lcore_id = 0; lcore_id < rxlcore_count; lcore_id++ )
            {
                if(rxlcorelt[lcore_id] == lid)
                {
                 printf("TEST1 %d %d %d\n", masterid,Wlcore_list[lcore_id],lid);
                 Statcore1 = 1;
                }

                if(masterid == rxlcorelt[lcore_id])
                {
                 masterlcore = 1;
                }
                 
            }
                 //printf("T1 %d %d %d\n", masterid,Wlcore_list[lcore_id],lid);
 
                
        for (lcore_id = 0; lcore_id < nb_Wlcore; lcore_id++ )
            {
                if(Wlcore_list[lcore_id] == lid)
                 {
                 printf("TEST2 %d %d %d\n", masterid,Wlcore_list[lcore_id],lid);
                 Statcore2 = 1;
                 }

                 if(masterid == Wlcore_list[lcore_id])
                 {
                  masterlcore = 2;
                 }

            }
                if(Statcore1 == 1)
                {      
                        PRINT_INFO("Starting Producer on LcoreId --> %d\n" , lid);
                       ret = rte_eal_remote_launch(packet_producer, NULL,lid);
                       if (ret != 0) FATAL_ERROR("Cannot start Producer thread on %d\n", lid);
                }
                 //printf("T2 %d %d %d\n", masterid,Wlcore_list[lcore_id],lid);
                else if(Statcore2 == 1)
                {
                       PRINT_INFO("Starting Consumer on LcoreId --> %d\n" , lid);
                       ret = rte_eal_remote_launch(packet_consumer, NULL,lid);
                       if (ret != 0) FATAL_ERROR("Cannot start consumer thread on %d\n", lid);
                }
                else if(!statsStarted && (masterid != lid))
                 {
                    printf("TEST3 masterid = %d lid = %d\n", masterid,lid);
                     printf("TEST4 Statcore1 = %d Statcore2 = %d \n", Statcore1, Statcore2); 
                        PRINT_INFO("Starting Statistics_lcore on LcoreId --> %d\n" , lid);
                      ret = rte_eal_remote_launch(Statistics_lcore, NULL,lid);
                       if (ret != 0) FATAL_ERROR("Cannot start Statistics_lcore thread on %d\n", lid);
                   statsStarted = true;
                 }
        }         

		      if(masterlcore == 0 && !statsStarted) 
		      {
                              PRINT_INFO("Starting Statistics_lcore on LcoreId masterid--> %d\n" , masterid);
			      Statistics_lcore(NULL);
		      }
		      else if(masterlcore == 2)
		      {
                              PRINT_INFO("Starting Consumer on LcoreId masterid--> %d\n" , masterid);
			      packet_consumer(NULL);
		      }else if(masterlcore == 1) 
                      {
                              PRINT_INFO("Starting producer on LcoreId masterid--> %d\n" , masterid);
                             packet_producer( NULL);
                      }
        //Wait for all lcore to finish  
        rte_eal_mp_wait_lcore();
        return 0;
}




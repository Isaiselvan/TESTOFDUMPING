
#include "main.h"

extern struct rte_ring    * intermediate_ring;
extern struct rte_mempool * pktmbuf_pool;
extern struct lcore_conf lcore_conf[RTE_MAX_LCORE];

int en_sys_ports;
int readportid[MAX_PORT];
uint64_t nb_captured_packets[RTE_MAX_LCORE] = {0};
long long int m_numberofpackets[RTE_MAX_LCORE]  = {0};  /*for continous number of packets to capture */
long long int missedouts[RTE_MAX_LCORE] = {0};



int packet_producer(__attribute__((unused)) void * arg){
        struct rte_mbuf *pkts_burst[MAX_PKT_BURST * MAX_PORT];
        //struct rte_mbuf * m;
        unsigned lcore_id = rte_lcore_id();
        PRINT_INFO("Lcore id of producer %d\n", lcore_id);
        unsigned int i;
        int idx, portid, nb_rx1, nb_rx, ret, pidx =0, queueid;
        struct lcore_conf pconf;
        struct timeval t_pack;

        pconf = lcore_conf[lcore_id];

        //if (qconf->n_rx_port == 0) {
         //       PRINT_INFO("lcore %u has nothing to do\n", lcore_id);
           //     return -1;
       // }
        PRINT_INFO( "entering main loop on lcore %u\n", lcore_id);
        for (i = 0; i < pconf.n_rx_pqp; i++) {

                portid = pconf.rx_queue_list[i].port_id;
                queueid =  pconf.rx_queue_list[i].queue_id; 
                PRINT_INFO(" -- lcoreid=%u portid=%u queueid=%d\n", lcore_id,
                        portid, queueid);

        }
      // int debug =0 ;
        /* Infinite loop */
          if (en_sys_ports == 0 )
               FATAL_ERROR("No port is up for reading\n"); 

        while(1) {
                          //PRINT_INFO( "portid%d \n", readportid);
			/* Timestamp the packet */
			ret = gettimeofday(&t_pack, NULL);
			if (ret != 0) FATAL_ERROR("Error: gettimeofday failed. Quitting...\n");
                  nb_rx = 0;
                //while ((m = rte_pktmbuf_alloc(pktmbuf_pool) ) == NULL );   

		//status = pcap_next_ex (m_pcapHandle, &PcapHdr, &data);
            // for (i = 0; i < qconf->n_rx_port; i++) {

                        //portid = qconf->rx_port_list[i];
                      while(pidx < pconf.n_rx_pqp) 
                      {
                        nb_rx1 = rte_eth_rx_burst((uint8_t)pconf.rx_queue_list[pidx].port_id,(uint8_t)pconf.rx_queue_list[pidx].queue_id, &pkts_burst[nb_rx], MAX_PKT_BURST);
                          nb_rx += nb_rx1;    
                          ++pidx;
                  //Multi port read
                          if(pidx ==  pconf.n_rx_pqp || pidx == MAX_PORT)  
                            {
                              pidx = 0;
                              break;
                            }
                      }
                        if(unlikely(nb_rx <0))
                              continue ;

                           for (idx= 0;idx<nb_rx;idx++)
                           {
                             pkts_burst[idx]->tx_offload = t_pack.tv_sec;
                             pkts_burst[idx]->udata64 =  t_pack.tv_usec;
                           }
                           nb_captured_packets[lcore_id] += nb_rx;
                           ret = rte_ring_enqueue_burst(intermediate_ring, (void*)pkts_burst,nb_rx);
                                             
                            m_numberofpackets[lcore_id] += ret;
                            missedouts[lcore_id] += nb_rx -ret;
    
                         for(;ret<nb_rx;ret++) 
                            rte_pktmbuf_free((struct rte_mbuf *)pkts_burst[ret]);


                          
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


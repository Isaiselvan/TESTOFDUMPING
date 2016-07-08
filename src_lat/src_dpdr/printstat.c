
#include "main.h"

int do_shutdown = 0;
extern long long int m_numberofpackets[RTE_MAX_LCORE];  /*for continous number of packets to capture */
extern long long int missedouts[RTE_MAX_LCORE];
extern char file_name_moveG[1000];
extern char file_name_oldG[1000];
extern char file_name_rotated [1000];
extern int nb_sys_ports;
extern uint64_t start_secs;
extern FILE *FP;
extern inline int FlushToFile(__rte_unused void *param);
extern struct rte_ring    * intermediate_ring;
extern struct rte_mempool * pktmbuf_pool;

static void alarm_routine (__rte_unused void *param);

void print_stats (void){
        
    static time_t curT = 0, prevT = 0 ;//= last_rotation;
    struct timeval t_pack; 
    int ret;
    int lcore_id ;
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
        int lcore_count = rte_lcore_count ();
        int rxlcorelt[lcore_count], rxlcore_count;
        rxlcore_count = get_nb_rx_lcores(&rxlcorelt[0]); 
 

	struct rte_eth_stats stat; 
	int i; 
	long int  good_pkt = 0, miss_pkt = 0, st_pktproc =0, st_missout =0, timeInt = 0;
        for (lcore_id = 0; lcore_id < rxlcore_count; lcore_id++ )
        {
                //rxlcorelt[lcore_id] 
        st_pktproc += m_numberofpackets[rxlcorelt[lcore_id]];
        st_missout += missedouts[rxlcorelt[lcore_id]];
        missedouts[rxlcorelt[lcore_id]] = 0;
        }
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
        timeInt = curT - prevT;
        PRINT_INFO("Packet Capture Statistics:\n");
	printf("\n-------------------------------------------------"); 
	printf("\nTOT:     Rx: %ld Drp: %ld Tot: %ld Perc: %.3f%%", good_pkt, miss_pkt, good_pkt+miss_pkt, (float)miss_pkt/(good_pkt+miss_pkt)*100 ); 
	printf("\n"); 
        fprintf(f, "Splunk %s Appname=FBMDump pktrecv=%lld pktdrop=%lld  pktprocss=%lld  ", TimeBuf, 
                (good_pkt - prvrecevied)/timeInt, (miss_pkt - prvdrop)/timeInt, (st_pktproc - prvprocessed)/timeInt);           
        //fprintf(f, "Splunk %s Appname=FBMDump pktrecv=%ld pktdrop=%ld  pktprocss=%ld \n ", TimeBuf,
          //      good_pkt/INTERVAL_STATS, miss_pkt/INTERVAL_STATS, st_pktproc/INTERVAL_STATS);
        fprintf(f," Missedby enqueue=%ld\n",st_missout/timeInt); 
        prvrecevied = good_pkt;
        prvdrop = miss_pkt;
        prvprocessed = st_pktproc;
        prevT = curT;
        fprintf(f,"Ring free = %d, Ring used = %d\n", rte_ring_free_count(intermediate_ring), rte_ring_count(intermediate_ring)); 
        fprintf(f,"Mempool free %d, Mempool used = %d\n", rte_mempool_count(pktmbuf_pool), rte_mempool_free_count(pktmbuf_pool));  
        //rte_ring_dump(f, intermediate_ring);
        //rte_mempool_dump(f,pktmbuf_pool);
	fclose(f);
     
}

int Statistics_lcore(__attribute__((unused)) void * arg){
        /* Create handler for SIGINT for CTRL + C closing and SIGALRM to print stats*/
        //signal(SIGALRM, alarm_routine);
 PRINT_INFO("Lcore id of Statistics_lcore %d\n", rte_lcore_id());
        //alarm(1);
         rte_eal_alarm_set(INTERVAL_STATS * MS_PER_S, alarm_routine, NULL);
        while(1)
        {
                      /*if(file_name_moveG[0] != '\0')
                        {
                        if (rename (file_name_oldG, file_name_moveG))
                         PRINT_INFO("\n failed to rename file %s\n", file_name_rotated); 
                           file_name_moveG[0] = '\0';
                        }*/
         
                     if (unlikely(do_shutdown))
			break;
         usleep(100);
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
void sig_handler(int signo)
{
        uint64_t diff;
        int ret;
        struct timeval t_end;
        char file_name_move[1000];

        /* Catch just SIGINT */
        //if (signo == SIGINT){

                /* Signal the shutdown */
                do_shutdown=1;

                if(FP)
                {
                   FlushToFile(NULL);
                   fclose(FP);
                   FP = NULL;
                }
                /* Print the per port stats  */
                printf("\n\nQUITTING...\n");

                ret = gettimeofday(&t_end, NULL);
                if (ret != 0) FATAL_ERROR("Error: gettimeofday failed. Quitting...\n");
                diff = t_end.tv_sec - start_secs;
                PRINT_INFO("Received signal %d \n", signo);
                printf("The capture lasted %ld seconds.\n", diff);
                print_stats();

                /* Close the pcap file */
                //pcap_close(pd);
                //pcap_dump_close(pcap_file_p);
                snprintf(file_name_move,sizeof(file_name_move), "%s%s", file_name_rotated, "ready.pcap");
                        if (rename (file_name_rotated, file_name_move))
                printf("\n failed to rename file %s\n", file_name_rotated);
                exit(0);
        //}
}



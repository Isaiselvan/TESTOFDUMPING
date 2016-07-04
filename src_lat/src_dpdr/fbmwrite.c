
#include "main.h"

#define FILE_CHUNK_SIZE (RTE_TEST_RX_DESC_DEFAULT * 1500 ) 
extern void sig_handler(int signo);
extern int snaplen;
extern struct rte_ring    * intermediate_ring;
extern struct rte_mempool * pktmbuf_pool;

uint64_t seconds_rotation;
uint64_t max_packets ;
int64_t  max_rotations = -1 ;

uint64_t nb_dumped_packets = 0;
char * file_name = NULL;
char file_name_moveG[1000];
char file_name_oldG[1000];
char file_name_rotated [1000];
int nb_sys_ports;
uint64_t start_secs;
uint64_t last_rotation = 0;
int64_t  nb_rotations=0;
static int bufferidx = 0;
static char filechunk[FILE_CHUNK_SIZE];
FILE *FP;


struct pcap_timeval {
    bpf_int32 tv_sec;           /* seconds */
    bpf_int32 tv_usec;          /* microseconds */
};

struct pcap_sf_pkthdr {
    struct pcap_timeval ts;     /* time stamp */
    bpf_u_int32 caplen;         /* length of portion present */
    bpf_u_int32 len;            /* length this packet (off wire) */
};


int packet_consumer(__attribute__((unused)) void * arg){

        struct timeval t_pack;
        struct rte_mbuf * m[MAX_PKT_BURST];
        u_char * packet;
        int ret, idx;
        //struct pcap_pkthdr pcap_hdr;
        struct pcap_sf_pkthdr sf_hdr;
        PRINT_INFO("Lcore id of consumer %d\n", rte_lcore_id());
        /* Init first rotation */
        ret = gettimeofday(&t_pack, NULL);
        if (ret != 0) FATAL_ERROR("Error: gettimeofday failed. Quitting...\n");

        last_rotation = t_pack.tv_sec;
        start_secs = t_pack.tv_sec;

        /* Open pcap file for writing */
        snprintf(file_name_rotated, sizeof(file_name_rotated),"%s%ld",file_name,last_rotation);
        createNewFile(file_name_rotated, snaplen);
          PRINT_INFO("Opened file %s\n", file_name_rotated);
       
         /* Start stats */
        //alarm(1);


        /* Infinite loop for consumer thread */
        while(1){


                /* Dequeue packet */
                //ret = rte_ring_dequeue(intermediate_ring, (void**)&m);
                 if( unlikely(ret = rte_ring_dequeue_burst(intermediate_ring, (void **)m,
                                MAX_PKT_BURST)) <= 0)
                  continue;
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
                if (unlikely((seconds_rotation > 0 && t_pack.tv_sec - last_rotation > seconds_rotation) || (max_packets != 0  && nb_dumped_packets >= max_packets))){

                        last_rotation = t_pack.tv_sec;
                        nb_rotations ++;

                        /* Quit if the number of rotations is the max */
                        if (max_rotations != -1 && nb_rotations > max_rotations)
                                sig_handler(SIGINT);

                        
                        /* Close the pcap file */
			if(FP)
			{
			    FlushToFile(NULL);
			    fclose(FP);
			    FP = NULL;
			}
				
                        snprintf(file_name_moveG,sizeof(file_name_moveG), "%s%s", file_name_rotated, "ready.pcap");  
                        snprintf(file_name_oldG,sizeof(file_name_oldG) ,"%s", file_name_rotated);
                        /* Open pcap file for writing */
                        snprintf(file_name_rotated,sizeof(file_name_rotated), "%s%ld", file_name, last_rotation);
                         createNewFile(file_name_rotated, snaplen);
                        nb_dumped_packets = 0;
                                 
                     //   PRINT_INFO("Opened file %s\n", file_name_rotated);
                }

                /* Compile pcap header */
                //pcap_hdr.ts = t_pack;
                //pcap_hdr.caplen = (rte_pktmbuf_data_len(m[idx]) < snaplen) ? rte_pktmbuf_data_len(m[idx]): snaplen;
                //pcap_hdr.len = m[idx]->pkt_len;
                packet = rte_pktmbuf_mtod(m[idx], u_char *);
                sf_hdr.ts.tv_sec = t_pack.tv_sec;
                sf_hdr.ts.tv_usec = t_pack.tv_usec;
                sf_hdr.caplen = (rte_pktmbuf_data_len(m[idx]) < snaplen) ? rte_pktmbuf_data_len(m[idx]): snaplen;
                sf_hdr.len = m[idx]->pkt_len;
                
                /* Write on pcap */
                //pcap_dump ((u_char *)pcap_file_p, & pcap_hdr,  rte_pktmbuf_mtod(m[idx], u_char *));
                 
                rte_memcpy(&filechunk[bufferidx],( const char *)&sf_hdr,sizeof(sf_hdr));
                bufferidx += sizeof(sf_hdr);
                rte_memcpy(&filechunk[bufferidx],( const char *)packet,sf_hdr.caplen); 
                bufferidx += sf_hdr.caplen;
                 
                nb_dumped_packets++;
                //printf("BufferIdx = %d, pcap_hdr.caplen = %d, sizeofpcap_hdr = %d, pcap_hdr.len = %d\n ", bufferidx,pcap_hdr.caplen, sizeof(pcap_hdr),pcap_hdr.len );   
                /* Free the buffer */
                rte_pktmbuf_free( (struct rte_mbuf *)m[idx]);
                m[idx] = NULL;
                
               // Flush file
                 if( (bufferidx + sizeof(sf_hdr) + snaplen) >     FILE_CHUNK_SIZE )
                  FlushToFile(NULL); 
              }
                //pcap_dump_flush(pcap_file_p);
        }
}


inline int FlushToFile(__rte_unused void *param)
{

  int blockSize = 1;
 // int seekbefore =0;
       
  //if(blockSize > bufferidx )
    //blockSize = bufferidx;
   //PRINT_INFO("blockSize = %d\n", blockSize);
   if(FP)
    {
     int rc = fwrite(filechunk, blockSize, bufferidx, FP);
     if (rc < bufferidx)
         FATAL_ERROR("Error while writing to file \n") ;
     //seekbefore = bufferidx % blockSize;
     //fseek(FP, -seekbefore, SEEK_CUR); 
   //  PRINT_INFO("Writing to file \n");
     bufferidx = 0 ;
     memset(filechunk,0, sizeof(filechunk));
    }
   return 0;
}

inline void createNewFile(char * filename, int snaplen)
{
  static struct pcap_file_header fileheader;
  if(FP)
  {
    FlushToFile(NULL);
    fclose(FP);
    FP = NULL;
  }

  FP = fopen((const char *)filename, "wb");
  if(!FP)
    {
      FATAL_ERROR("Fatal error while creating the new file exiting..%s\n",strerror(errno));
    }
       if(fileheader.magic != 0xa1b2c3d4)   
       {
         char tmpstr[] = "/tmp/PCAPTMP_hdr.0000";
         FILE* pFile = fopen (tmpstr , "wb");
         int ret;
         pcap_t *pt;
          pt = pcap_open_dead(DLT_EN10MB, snaplen); 
          pcap_dumper_t *dump = pcap_dump_fopen(pt, pFile);
            if (dump) pcap_dump_close(dump);
          pcap_close(pt); 
        int tmpfd = open(tmpstr, O_RDONLY); /* get pcap to create a header */   
        if (tmpfd >= 0) 
             ret = read(tmpfd, (char *)&fileheader, sizeof(fileheader));
        if (tmpfd >= 0) close(tmpfd);
        //fclose(pFile);
           fileheader.snaplen = snaplen;
        //        
       }

       /* fileheader.magic = 0xa1b2c3d4;
        fileheader.version_major = 2;
        fileheader.version_minor = 4;
        fileheader.thiszone = 0;
        fileheader.sigfigs = 0;
        fileheader.snaplen = snaplen;
        fileheader.linktype = 1;*/
     fwrite((void *)&fileheader, 1, sizeof(fileheader), FP);
     bufferidx = 0;
}

#include "PacketReader.h"
#include <sys/types.h>
#include <unistd.h>
#include <syscall.h>

PacketReader* PacketReader::m_onlyreader = NULL;   
int PacketReader::countPkt = 0;

bool PacketReader::SetThreadAttributes() 
{
  if ( -1 == m_readeraffinity)
       return false; // Default thread behaviour 

    int readtid = syscall(__NR_gettid);//syscall.h to get the pid associated with thread
 pthread_t thread = pthread_self(); 
    std::cout << "DEV: threadID " << thread << std::endl ;
    cpu_set_t csmask;
    CPU_ZERO(&csmask);
    CPU_SET(m_readeraffinity, &csmask);
   /*if( sched_setaffinity(readtid, sizeof(cpu_set_t), &csmask) != 0 ) {
     std::cerr <<  "Reader could not set cpu affinity : " << m_readeraffinity << "\n" ; 
     return false;
   }*/

   if ( pthread_setaffinity_np(thread, sizeof(cpu_set_t), &csmask) != 0 ) {
     std::cerr << "Reader error on thread set cpu \n";
     return false;
   }
  
   // Set the priority of the read thread If priority is set  
   if(-1 == m_readerpriority)
      return true; // return true as affinity is changed

   if(setpriority(PRIO_PROCESS, readtid, m_readerpriority) != 0 ) {
     std::cerr << "Reader could not set scheduling priority: \n" ; 
     return false;
   }
 
}

void PacketReader::PcapStartUp()
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
	if (m_bflag != 0) {
	 	status = pcap_set_buffer_size(m_pcapHandle, m_buflen);
	if (status != 0)
	fprintf(stderr,"%s: Can't set buffer size: %s",
			    m_interfacename, pcap_statustostr(status));
		}
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

extern struct pcap_file_header fileheader; 
void PacketReader::createPcapFheader()
{
  char tmpstr[] = "/tmp/PCAPTMP_hdr.0000";
       FILE* pFile = fopen (tmpstr , "wb");
        pcap_dumper_t *dump = pcap_dump_fopen(m_pcapHandle, pFile);
        if (dump) pcap_dump_close(dump);
        int tmpfd = open(tmpstr, O_RDONLY); /* get pcap to create a header */    
        if (tmpfd >= 0) read(tmpfd, (char *)&fileheader, sizeof(fileheader));
        if (tmpfd >= 0) close(tmpfd);
        //fclose(pFile);
        fileheader.snaplen = snaplen;
        std::cout << "START READING THREAD" << std::endl; 

}

void PacketReader::cleanup()
{
 std::cout << "Total Packets actually queued in buffer for processing " << countPkt << std::endl;
 PrintStat();
}


void PacketReader::PrintStat()
{
       if (!(linktype = pcap_datalink(m_pcapHandle))) {
                fprintf(stderr,
                        "Error getting link layer type for interface %s",
                        m_interfacename);
                exit(9);
        }
        printf("The link layer type for packet capture device %s is: %d.\n",
                m_interfacename, linktype);


      if (pcap_stats(m_pcapHandle, &m_pcapstatus) != 0) {
                fprintf(stderr, "Error getting Packet Capture stats: %s\n",
                        pcap_geterr(m_pcapHandle));
                exit(10);
        }

        /* Print the statistics out */
        printf("Packet Capture Statistics:\n");
        printf("%d packets received by filter\n", m_pcapstatus.ps_recv);
        printf("%d packets dropped by kernel\n", m_pcapstatus.ps_drop); 
}

void PacketReader::Reader_Run()
{
 // 1st set the threadAtttributes and priority if needed
 SetThreadAttributes();
 PcapStartUp();
 createPcapFheader();

/*  if (pcap_loop(m_pcapHandle, m_numberofpackets, &PacketReader::WritePKTtoBuf,NULL) < 0) {
  sprintf(prestr,"Error reading packets from interface %s",
                        m_interfacename);
                pcap_perror(m_pcapHandle,prestr);
                exit(8);
        }*/

    register struct pcap_pkthdr *PcapHdr;
    register const u_char * data;
    int status;
    while (1)
    {
	    if(m_onlyreader->m_threadStop)
	    {
		    m_onlyreader->cleanup();
		    pthread_exit(NULL);
	    }

	    status = pcap_next_ex (m_pcapHandle, &PcapHdr, &data);
	    if(status == 1)
	    {
		    WritePKTtoBuf(NULL, PcapHdr, data);
	    }
	    else if (status == 0)
	    {
		    //  fprintf(stdout,"\nWarning: Timeout while reading packet\n");
	    }
	    else if (status == -1)
	    {
		    fprintf(stderr,"\nError while reading the packets\t from dev:%s %s ",m_interfacename,pcap_geterr(m_pcapHandle));
		    exit(-1);
	    }

    }

}

inline void PacketReader::WritePKTtoBuf(u_char *DummyFile, const struct pcap_pkthdr *PcapHdr, const u_char * data)
{
   countPkt++;
   struct pcktPLUSEpcaphd capture;
             capture.sf_hdr.ts.tv_sec = PcapHdr->ts.tv_sec;
             capture.sf_hdr.ts.tv_usec = PcapHdr->ts.tv_usec;
             capture.sf_hdr.caplen = PcapHdr->caplen;
             capture.sf_hdr.len = PcapHdr->len;
             
             memset(&capture.pktdata,0,sizeof(capture.pktdata));
             memcpy (&capture.pktdata, data, capture.sf_hdr.caplen);
   int  i = buffer_.size_approx();
   if (!buffer_.enqueue(capture))
   std::cout << "Enqueue failed loss of data" << std::endl;  
}


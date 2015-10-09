#include "PacketWriter.h"
#include "PacketReader.h"

#include <sys/types.h>
#include <unistd.h>
#include <syscall.h>
#include <sstream>


PacketWriter* PacketWriter::m_onlywriter = NULL;   
auto m_pretime =  std::chrono::high_resolution_clock::now();

bool PacketWriter::SetThreadAttributes()
{
  if ( -1 == m_writeraffinity)
       return false; // Default thread behaviour

    int writetid = syscall(__NR_gettid);//syscall.h to get the pid associated with thread
     pthread_t thread = pthread_self();
    std::cout << "DEV: threadID2 " << thread << std::endl ;
    cpu_set_t csmask;
    CPU_ZERO(&csmask);
    CPU_SET(m_writeraffinity, &csmask);
  /* if( sched_setaffinity(writetid, sizeof(cpu_set_t), &csmask) != 0 ) {
     std::cerr <<  "Reader could not set cpu affinity : " << m_writeraffinity << "\n" ;
     return false;
   }*/

   // Set the priority of the read thread If priority is set
       if(-1 == m_readerpriority)
             return true; // return true as affinity is changed
   
   if(setpriority(PRIO_PROCESS, writetid, m_readerpriority) != 0 ) {
                     std::cerr << "Reader could not set scheduling priority: \n" ;
        return false;
   }

}


void PacketWriter::cleanup()
{
  FlushToFile();
  if(FP)
  {
    std::fclose(FP);
    FP = NULL;
  }
  
  std::cout << "Totol Packets received from Buffer = " << noOfpkts << std::endl;
}

extern struct pcap_file_header fileheader;
inline void PacketWriter::createNewFile()
{
  if(FP)
  {
    //FlushToFile(); 
    std::fclose(FP);
    FP = NULL;
  }
  std::string  filename = m_filename;

  if (m_filerotate )
  { 
  m_pretime = std::chrono::high_resolution_clock::now();
  std::chrono::nanoseconds ns = std::chrono::duration_cast<std::chrono::nanoseconds>(m_pretime.time_since_epoch());

  std::time_t tmpInt = ns.count(); 
  std::stringstream ss;
  ss << tmpInt ;
  filename = filename + ss.str() + ".pcap";
  }
 
  FP = std::fopen((const char *)filename.c_str(), "wb");
  if(!FP)
    {
      fprintf(stderr,"Fatal error while creating the new file exiting..\n",strerror(errno));
      exit(errno);
    }

    if (fileheader.magic != 0xa1b2c3d4) {  /* if not fetched , do this */
        fprintf(stderr, "%s: using canned pcap header\n");
        fileheader.magic = 0xa1b2c3d4;
        fileheader.version_major = 2;
        fileheader.version_minor = 4;
        fileheader.thiszone = 0;
        fileheader.sigfigs = 0;
        fileheader.snaplen = PacketReader::GetInstance()->GetSnapLen(); 
        fileheader.linktype = 1;
        }
     std::fwrite((void *)&fileheader, 1, sizeof(fileheader), FP); 
}

inline void PacketWriter::FlushToFile()
{

  int blockSize = 64;

  if(blockSize > bufferLen )
    blockSize = bufferLen;
  
   if(FP)
    {
     int rc = std::fwrite(tochunk, 1, bufferLen, FP);
     if (rc < bufferLen)
         fprintf(stderr,"Error while writing to file \n" ) ;

     bufferLen = 0 ;
     memset(tochunk,0, sizeof(tochunk)); 
    }

   
}


void PacketWriter::Writer_Run ()
{

  std::cout << "STARTING THE WRITER THREAD" << std::endl;
  SetThreadAttributes();
  pcktPLUSEpcaphd buffered;
  createNewFile();

 while (1)
 {
 
  if (buffer_.try_dequeue(buffered))
   {
     noOfpkts++;
     memcpy(&tochunk[bufferLen],( const char *)&buffered.sf_hdr,sizeof(buffered.sf_hdr)) ;
     bufferLen += sizeof(buffered.sf_hdr);
     memcpy(&tochunk[bufferLen],(const char *) &buffered.pktdata, buffered.sf_hdr.caplen);
     bufferLen +=  buffered.sf_hdr.caplen;
 
     // Check for flushing out the buffer
     if( (bufferLen + sizeof (struct pcktPLUSEpcaphd) ) > (CHUNKSIZE ))
     FlushToFile();  

   }else if(m_onlywriter->m_threadStop)
   {
            cleanup();
            pthread_exit(NULL);
   }else {
        usleep(WAITUSEC);
   }
    
    if (m_filerotate) // Rotate file 
    {
      auto currentTime = std::chrono::high_resolution_clock::now();
      auto TIMEIN = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - m_pretime).count();

      if(TIMEIN > m_interval)
        {
          createNewFile(); 
        } 
    } 
  
 }
}




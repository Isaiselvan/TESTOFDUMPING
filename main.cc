#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include "readerwriterqueue.h" 
#include <pthread.h>
#include <signal.h>
#include <chrono>
#include "PacketReader.h"
#include "PacketWriter.h" 

int MAX_PKT_SIZE = 63500;

/*struct _pcap_File_Pckt{

  char [MAX_PKT_SIZE];
};*/
// 3rd party lib, for lock less cricular queue 
// Declared Globally to be accessed from both the reader and writer threads. 
using namespace moodycamel;
ReaderWriterQueue <struct pcktPLUSEpcaphd> buffer_(200); // Initial allocation of 200 pkt space which is 200MB of ram.  

struct pcap_file_header fileheader; 


void *Reader (void *args)
{
 // Read thread helper function     
 //Call the runner loop
      PacketReader::GetInstance()->Reader_Run();
}

void *Writer (void *args)
{
 // Write thread helper function 
 //Call the runner loop 
    PacketWriter::GetInstance()->Writer_Run();
}


void cleanup(int signo)
{
 std::cout << "\nSTOPPING THREADS" << std::endl;
 PacketReader::GetInstance()->StopThread();
 PacketWriter::GetInstance()->StopThread();
 // Clean reader
 
 //PacketReader::GetInstance()->cleanup();

 // Clean Writeer
 //PacketWriter::GetInstance()->cleanup();
 sleep (2);
 PacketReader::GetInstance()->cleanup();
 exit(2);
}

int main (int argc, char ** argv)
{
 
 pthread_t threads[2];
 int rc ;
 char *Ifs; 
  int opt;

  while ((opt = getopt(argc, argv, "i:c:f:B:s:w:R:n")) != EOF)
  {
    switch (opt) 
           {
     
       case 'i': 
           Ifs = optarg;
          
           PacketReader::GetInstance()->SetInterface(Ifs);
          break;

       case 's':
            PacketReader::GetInstance()->SetSnapLen(atoi(optarg));
           break; 

       case 'w':
            if(strlen(optarg) > 1 )
            PacketWriter::GetInstance()->Setfilename((std::string)optarg);
           break;

       case 'f':
            if(strlen(optarg) > 1)
            PacketReader::GetInstance()->SetFilterBSF((std::string)optarg);
           break; 
  
       case 'R':
            if(strlen(optarg) > 1)
            PacketWriter::GetInstance()->Setfilerotate(atoi(optarg));
            else
            PacketWriter::GetInstance()->Setfilerotate();
         
          break;

      /* case 'f':
           break;*/

      // default:
        ///      ;
      }

  }
 
//sleep (2); 
 PacketReader::GetInstance()->SetAffinityValue(READ_AFF); // TODO need to fetch from property file
 PacketWriter::GetInstance()->SetAffinityValue(WRITE_AFF); // TODO need to Fetch from property file
 //Set thread priority if needed. using setprio.. 
 PacketReader::GetInstance()->SetPriority(READ_PRIO);
 PacketWriter::GetInstance()->SetPriority(WRITE_PRIO);
 
 


rc = pthread_create(&threads[0], NULL, &Reader, NULL);
    if (rc){
        fprintf(stderr, "%s: pthread_create error\n", argv[0]);
        exit(1);
        }

sleep (1); 
rc = pthread_create(&threads[1], NULL, &Writer, NULL);
    if (rc){
        fprintf(stderr, "%s: pthread_create error\n", argv[0]);
        exit(1);
        }

#ifdef USE_SIGNAL
    signal(SIGINT, cleanup);
    signal(SIGPIPE, cleanup);
#else
    struct sigaction sa;
    sa.sa_handler = cleanup;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;                    /* allow signal to abort pcap read */

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGPIPE, &sa, NULL);
#endif /* USE_SIGNAL */



 int *status;
 pthread_join(threads[0], (void **) &status);
 pthread_join(threads[1], (void **) &status);

 return 0;
}





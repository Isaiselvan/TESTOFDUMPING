#include <iostream>
#include <pthread.h>
#include "readerwriterqueue.h"
#include <sys/resource.h>
#include "Common.h"
#include <map>
#include <sys/mman.h>
#include <chrono>

#define CHUNKSIZE 1024 * 100 // 1024 * 100 writing to the disk at 100 Mbps

#define SECINNANO  1000 * 1000 * 1000

using namespace moodycamel;
extern ReaderWriterQueue <struct pcktPLUSEpcaphd> buffer_;



class PacketWriter 
{
 public:

 public:
       static PacketWriter * GetInstance(){
          if( NULL == m_onlywriter )
           {
             m_onlywriter = new PacketWriter();
           }
           return m_onlywriter;
          }

        void Setfilename(std::string fstr) {m_filename = fstr; }
        void Writer_Run (); 
        bool SetAffinityValue(int aff){m_writeraffinity = aff;}
        void SetPriority(int prio) {m_readerpriority = prio;}
        void cleanup();
        void StopThread(){m_threadStop = true;}
        void Setfilerotate (unsigned long int inte = SECINNANO){
        m_filerotate = true;
        m_interval = inte; 
        }
 private:

       char tochunk[CHUNKSIZE];
       int bufferLen;
       static PacketWriter * m_onlywriter;
       std::string m_filename;
       int m_writeraffinity;
       int m_readerpriority;
       int snaplen;
       bool m_threadStop = false;
       FILE *FP; 
       std::map<std::string, int> totalFilesCreated; //print stats TODO
       int noOfpkts; // Per file basis
       bool m_filerotate;
       unsigned long int  m_interval ;
 private: 
        PacketWriter (){
          // Will not change the affinity if the value is -1
          m_writeraffinity = -1;
          m_readerpriority = -1;
          snaplen = 65535;
          bufferLen = 0;
          noOfpkts = 0; 
          FP = NULL;
          mlock(tochunk, CHUNKSIZE);
          m_filename = "./ICISPCAP.pcap";
          m_filerotate = false;
          m_interval = SECINNANO; 
        }
        
        bool SetThreadAttributes();
        void createNewFile();
        void FlushToFile();
        

   //TODO
   //Write filters and controls
};



#include <iostream>
#include <string>
#include <string.h>
#include <map>
#include <pthread.h>
//#include "protocol.h"
//#include "libtrace.h"
#include "libtrace_parallel.h"
#include "tcpProto.h"

#define TIMEINT 60
extern const char clr[]; // = { 27, '[', '2', 'J', '\0' };
extern const char topLeft[];// = { 27, '[', '1', ';', '1', 'H','\0' };

//Max = 20
extern std::string protcolname[]; 
// Singleton Displaystat only one display
class displayStats{
  
  private : 
  
  int curIntStarttime;
  int curIntEndtime;
  unsigned long int totalpkts;
  unsigned long int totaldatalen;
  libtrace_stat_t pcapStats; 
  static displayStats * displayBoard;  
  pthread_mutex_t disLock;  
     //Source ethernet address // Node
  std::map<std::string, std::map<libtrace_ipproto_t, protocolBase* > > dashboard; 
  
  

  private :
      
  displayStats (){
   curIntStarttime = 0;
   curIntEndtime = 0;
   totalpkts = 0;
   totaldatalen = 0;
   StatsAvailable = false;
   protcolname[TRACE_IPPROTO_TCP] = "TCP";
   std::cout << "protcolname[TRACE_IPPROTO_TCP] = " << protcolname[TRACE_IPPROTO_TCP] << std::endl; 
  }
  public : 
  bool StatsAvailable ; //1st time call to dashboard 
  static displayStats * getdashB(){
    
   if (displayBoard == NULL)
   {
      displayBoard = new displayStats();
   }
   
      return displayBoard;  
   }
  ~displayStats(){}

  int ParsePkt(libtrace_packet_t *pkt);
  template <typename T> int addPkt(T,libtrace_packet_t *, libtrace_ipproto_t, int);
  template <typename T> int getDataLen(T t){return t.getDataLen(); }

  protocolBase*  getProtoBase(std::string node, libtrace_ipproto_t);  
  int cleardashB(int newtsrtime, int newendtime);
  void printstats(); // Future will be sending to some other module or UI
  int setStats(libtrace_stat_t stat){
   //   pthread_mutex_lock(&disLock);
      pcapStats.accepted += stat.accepted;
      pcapStats.filtered += stat.filtered;
      pcapStats.received += stat.received;
      pcapStats.dropped += stat.dropped; 
      pcapStats.captured += stat.captured;
      pcapStats.errors += stat.errors;
      StatsAvailable = true;  
   //   pthread_mutex_unlock(&disLock);
   return 0;
   }
  int clearStats(){
    pcapStats.accepted = 0;
    pcapStats.filtered = 0;
    pcapStats.received = 0; 
    pcapStats.dropped = 0; 
    pcapStats.captured = 0; 
    pcapStats.errors = 0;
    StatsAvailable = false; 
   } 
};




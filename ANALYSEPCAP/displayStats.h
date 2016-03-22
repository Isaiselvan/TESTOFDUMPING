#ifndef DISPLAY_STATS_H
#define DISPLAY_STATS_H

#include <iostream>
#include <string>
#include <string.h>
#include <map>
#include <pthread.h>
#include "protocol.h"
//#include "libtrace.h"
#include "packetCmm.h"
#include "libtrace_parallel.h"

#define TIMEINT 30 //Sec 
extern const char clr[]; // = { 27, '[', '2', 'J', '\0' };
extern const char topLeft[];// = { 27, '[', '1', ';', '1', 'H','\0' };

//Max = 20
extern std::string protcolname[]; 
// Array of Singleton Displaystat only one display
class displayStats{
  
  private : 
  
  int curIntStarttime;
  int curIntEndtime;
  unsigned long int totalpkts;
  unsigned long int totaldatalen;
  libtrace_stat_t pcapStats; 
  static displayStats * displayBoard[MAX_LAYER];  
  pthread_mutex_t disLock;  
     //Source ethernet address // Node
  std::map<std::string, std::map<libtrace_ipproto_t, protocolBase* > > dashboard; 
  Displaylayer layer;  
  

  private :
      
  displayStats (){
   curIntStarttime = 0;
   curIntEndtime = 0;
   totalpkts = 0;
   totaldatalen = 0;
   StatsAvailable = false;
   protcolname[TRACE_IPPROTO_TCP] = "TCP";
   protcolname[TRACE_IPPROTO_UDP] = "UDP"; 
//   std::cout << "protcolname[TRACE_IPPROTO_TCP] = " << protcolname[TRACE_IPPROTO_TCP] << std::endl; 
  }
  public : 
  bool StatsAvailable ; //1st time call to dashboard 
  static displayStats * getdashB(Displaylayer lay = CORE_LAYER){
    
   if (displayBoard[lay] == NULL)
   {
      displayBoard[lay] = new displayStats();
      displayBoard[lay]->layer = lay; 
   }
      return displayBoard[lay];  
   }
  ~displayStats(){}

  int ParsePkt(libtrace_packet_t *pkt);
  int addPkt(m_Packet ,libtrace_packet_t *, libtrace_ipproto_t, int);
//  int getDataLen(m_Packet t){return t.getDataLen(); }

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


#endif // DISPLAY_STATS_H

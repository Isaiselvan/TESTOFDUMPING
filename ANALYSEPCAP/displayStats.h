#include <iostream>
#include <string>
#include <string.h>
#include <map>
#include <pthread.h>
//#include "protocol.h"
//#include "libtrace.h"
#include "libtrace_parallel.h"
#include "tcpProto.h"


// Singleton Displaystat only one display
class displayStats{
  
  private : 
  
  int curIntStarttime;
  int curIntEndtime;
  int totalpkts;
  int totaldatalen;
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
  }; 
 
  public : 
  bool StatsAvailable ; //1st time call to dashboard 
  static displayStats * getdashB(){
    
   if (displayBoard == NULL)
   {
      displayBoard = new displayStats();
      
   }
   
      return displayBoard;  
   };
  ~displayStats(){};

  int ParsePkt(libtrace_packet_t *pkt);
  template <typename T> int addPkt(T,libtrace_packet_t *, libtrace_ipproto_t, int);
  template <typename T> int getDataLen(T t){return t.getDataLen(); }

  protocolBase*  getProtoBase(std::string node, libtrace_ipproto_t);  
  int cleardashB(int newtsrtime, int newendtime);
  void printstats(); // Future will be sending to some other module or UI
  int setStats(libtrace_stat_t stat){
      pthread_mutex_lock(&disLock);
      pcapStats = stat;
      StatsAvailable = true;  
      pthread_mutex_unlock(&disLock);
   }
};




#include <iostream>
#include <string>
#include <string.h>
#include <map>
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
     //Source ethernet address // Node
  std::map<std::string, std::map<libtrace_ipproto_t, protocolBase > > dashboard; 
  
  

  private :
  displayStats (){
   curIntStarttime = 0;
   curIntEndtime = 0;
   totalpkts = 0;
   totaldatalen = 0;
  }; 
 
  public : 

  static displayStats * getdashB(){
    
   if (displayBoard = NULL)
      return (displayBoard = new displayStats());
  
      return displayBoard;  
   };
  ~displayStats(){};

  int ParsePkt(libtrace_packet_t *pkt);
  template <typename T> int addPkt(T);
  int cleardashB(int newtsrtime, int newendtime);
  void printstats(); // Future will be sending to some other module or UI

};

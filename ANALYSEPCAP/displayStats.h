#include <iostream>
#include <string>
#include <string.h>
#include "protocol.h"
#include "libtrace.h"


// Singleton Displaystat only one display
class displayStats{
  
  private : 
  
  int curIntStarttime;
  int curIntEndtime;
  int totalpkts;
  int totaldatalen;
  static displayStats * displayBoard;   
     //Source ethernet address // Node
  map<string, map<libtrace_ipproto_t, protocolBase > > dashboard; 
  
  

  private :
  displayStats (){}; 
 
  public : 

  static displayStats * getdashB(){
    
   if (displayBoard = NULL)
      return (displayBoard = new displayStats());
  
      return displayBoard;  
   };
  ~displayStats(){};

  bool addPkt(libtrace_packet_t *pkt);
  int cleardashB(int newtsrtime, int newendtime);
  void printstats(); // Future will be sending to some other module or UI

};

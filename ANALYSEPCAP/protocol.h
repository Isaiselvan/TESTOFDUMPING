#include <iostream>
#include <string>
#include <string.h>
//#include "libtrace.h"
#include "libtrace_parallel.h"



class protocolBase { 

protected: 
  int m_starttime;
  int m_endtime; 
  libtrace_ipproto_t m_protocoltyp;  
public :
   protocolBase(libtrace_ipproto_t type, int start, int end): m_starttime(start),m_endtime(end),m_protocoltyp(type){ };
   virtual  ~protocolBase(){};
   virtual int bandWidthCalc() {} ; // TO be pure , But can't use in Map 
   virtual void calculatemetrics() {} ; // "
   virtual void displaymetrics() {} ;  // "
};

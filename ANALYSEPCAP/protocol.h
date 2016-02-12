#include <iostream>
#include <string>
#include <string.h>
#include "libtrace.h"




class protocolBase { 
 
  int m_starttime;
  int m_endtime; 
  libtrace_ipproto_t m_protocoltyp;  
public :
   protocolBase(libtrace_ipproto_t type, int start, int end){ };
   virtual  ~protocolBase(){};
   int bandWidthCalc() = 0 {}; 
   void calculatemetrics() = 0 {}; 
   void displaymetrics() = 0 {}; 
};

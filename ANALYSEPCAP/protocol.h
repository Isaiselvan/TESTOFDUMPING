#include <iostream>

enum ether_type = {
  IEEE802_33 1,
  IEEE80ll ,
  
};

enum ip_type = {
  IPV4 1,
  IPV6
};


struct etherNet_hdr {
     ether_type;
};

struct ip_hdr {
     ip_type;
};

struct tcp_hdr{

};

struct upd_hdr{

}; 



class protocolBase { 

private:

   int m_totalNumPkts ; 
   int m_totalUplink ; 
   int m_totalDownlink;   

public :
  
   int bandWidthCalc(); 
   void calculatemetrics() = 0 {}; 
   void displaymetrics() = 0 {}; 
};



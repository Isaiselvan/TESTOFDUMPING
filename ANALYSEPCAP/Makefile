
APPNAME = PCAP_NEW

APP_OBJS = main.o\
	   PacketReader.o\
	   PacketWriter.o

#export LD_LIBRARY_PATH = $(LD_LIBRARY_PATH):/usr/local/lib/

CPPFLAGS = "-std=c++11" -lpthread -lrt
ifdef PF_RING
PCAPLIB = -L ./lib/ \
          -lpcap_pf_ring -lpfring 
else
PCAPLIB = -lpcap  
endif

INCLUDEPATH  =  -I ./readerwriterqueue/

.SUFFIXES:
	.h .cc 

%.o:%.cc
	$(CXX) $(CPPFLAGS) -g -c $^

.PHONY:
	all clean


all:
	g++ -ltrace -lpthread -o analyse.exe main.cpp	
	g++ -ltrace  -c tcpProto.cpp
	g++ -ltrace -c displayStats.cpp 
PCAP_NEW: $(APP_OBJS)
	$(CXX) $(CPPFLAGS) $(APP_OBJS) -g  -o $@ $(PCAPLIB) 
	
clean:
	-rm *.o $(APPNAME)







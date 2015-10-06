
APPNAME = PCAP_NEW

APP_OBJS = main.o\
	   PacketReader.o\
	   PacketWriter.o

CPPFLAGS = "-std=c++11" -lpthread -lrt
ifdef PF_RING
PCAPLIB = /home/sathyaisai/Sai_Proj/PF_RING/userland/libpcap/libpcap.a \
          /home/sathyaisai/Sai_Proj/PF_RING/userland/lib/libpfring.a
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
	$(MAKE) $(APPNAME)

PCAP_NEW: $(APP_OBJS)
	$(CXX) $(CPPFLAGS) $(APP_OBJS) -g  -o $@ $(PCAPLIB) 
	
clean:
	-rm *.o $(APPNAME)







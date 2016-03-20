
APPNAME = PCAP_ANALY 


APP_OBJS = tcpProto.o\
	   udpProto.o\
	   lteGtp.o\
	   displayStats.o\
	   hostDpi.o \
           main.o

#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/
CPPINC = -I /usr/local/include/libndpi-1.7.0/libndpi/
CPPFLAGS =  -lpthread -lrt -ltrace -DUSER_TCP_UDP

#INCLUDEPATH  =  -I ./readerwriterqueue/

.SUFFIXES:
	 .cpp

%.o:%.cpp
	$(CXX) $(CPPFLAGS) $(CPPINC) -g -c $^ 


.PHONY:
	all clean


all: $(APPNAME)
	
 
$(APPNAME): $(APP_OBJS) 
	$(CXX) $(CPPFLAGS) $(APP_OBJS) -g  -o $@  
	
clean:
	-rm -f *.o *.gch $(APPNAME)

#	g++ -ltrace -lpthread -o analyse.exe main.cpp	
#	g++ -ltrace  -c tcpProto.cpp
#	g++ -ltrace -c displayStats.cpp 






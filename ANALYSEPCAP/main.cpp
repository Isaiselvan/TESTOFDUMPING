#include <iostream>
#include <inttypes.h>
#include <err.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <dirent.h>
#include <pthread.h>
#include <list>
#include <string>
#include <string.h>
#include <unistd.h>
#include "displayStats.h"
#include "libtrace_parallel.h"
using namespace std;

#define DIRPATH "/apps/opt/LIBTRACE/test/"
const string pcapFileEndStr = "ready.pcap";


uint64_t count = 0;

static void per_packet(libtrace_packet_t *packet)
{
	assert(packet);
	/* This function turns out to be really simple, because we are just
	 * counting the number of packets in the trace */
	count += 1;
        displayStats::getdashB()->ParsePkt(packet);
}

/* Due to the amount of error checking required in our main function, it
 * is a lot simpler and tidier to place all the calls to various libtrace
 * destroy functions into a separate function.
 */
static void libtrace_cleanup(libtrace_t *trace, libtrace_packet_t *packet) {
	
	/* It's very important to ensure that we aren't trying to destroy
	 * a NULL structure, so each of the destroy calls will only occur
	 * if the structure exists */
	if (trace)
		trace_destroy(trace);
	
	if (packet)
		trace_destroy_packet(packet);

}

void* readPcapFile(void* fileName) 
{
	libtrace_t *trace = NULL;
	libtrace_packet_t *packet = NULL;
        string filePath = DIRPATH;
        filePath += (char *)fileName;
        delete[] (char *)fileName;
        time_t startTime, endTime;
        time(&startTime);

        cout << "Got file:" << (char *)fileName  << std::endl;
        cout << "reading Pcap file:" << filePath << std::endl;
	
	/* Creating and initialising a packet structure to store the packets
	 * that we're going to read from the trace */
	packet = trace_create_packet();

	if (packet == NULL)
        {
		/* Unfortunately, trace_create_packet doesn't use the libtrace
		 * error system. This is because libtrace errors are associated
		 * with the trace structure, not the packet. In our case, we
		 * haven't even created a trace at this point so we can't 
		 * really expect libtrace to set an error on it for us, can
		 * we?
		 */
		perror("Creating libtrace packet");
		libtrace_cleanup(trace, packet);
	}

	/* Opening and starting the input trace, as per createdemo.c */
	trace = trace_create(filePath.c_str());

	if (trace_is_err(trace))
        {
		trace_perror(trace,"Opening trace file");
		libtrace_cleanup(trace, packet);
	}

	if (trace_start(trace) == -1) {
		trace_perror(trace,"Starting trace");
		libtrace_cleanup(trace, packet);
	}

	/* This loop will read packets from the trace until either EOF is
	 * reached or an error occurs (hopefully the former!)
	 *
	 * Remember, EOF will return 0 so we only want to continue looping
	 * as long as the return value is greater than zero
	 */
        
	while (trace_read_packet(trace,packet)>0) {
		/* Call our per_packet function for every packet */
		per_packet(packet);
	}
        
        if(!displayStats::getdashB()->StatsAvailable)
          {
           libtrace_stat_t *stat =  trace_create_statistics();
            trace_get_statistics( trace, stat);
           displayStats::getdashB()->setStats(*stat);
          }
          

          //displayStats::getdashB()->printstats();
	/* If the trace is in an error state, then we know that we fell out of
	 * the above loop because an error occurred rather than EOF being
	 * reached. Therefore, we should probably tell the user that something
	 * went wrong
	 */
	if (trace_is_err(trace)) {
		trace_perror(trace,"Reading packets");
		libtrace_cleanup(trace, packet);
	}

	/* We've reached the end of our trace without an error so we can
	 * print our final count. Note the use of the PRIu64 format which is
	 * portable across 64 and 32 bit machines */
	//printf("Packet Count = %" PRIu64 "\n", count);
        cout << "Count:" << count << std::endl;
        time(&endTime);
        double seconds = difftime(endTime, startTime);
        //std::cout << clr <<  topLeft ;
        cout << "Completed file:" << filePath << " in:" << seconds << " seconds." << std::endl;

        /* Rename the file to indicate that it is completed*/
        string newFilePath = filePath + ".completed";
        if(rename(filePath.c_str(), newFilePath.c_str()))
            cout << "Error renaming the file:" << filePath << " to:" << newFilePath << std::endl;

        pthread_exit(NULL);
}

bool isPcapfileReady(string fileName)
{
     if(fileName.length() > pcapFileEndStr.length())
     {
         string endStr = fileName.substr(fileName.length()- pcapFileEndStr.length(), fileName.length());
         if(!endStr.compare(pcapFileEndStr))
             return true;
     }

     return false;
}

int main(int argc , char * argv []) 
{
    int fileCount = 0, rc = 0, i = 0;

    std::list<string> filesList;
    std::list<string>::iterator it;
  while(1) 
 {
    DIR *dirp = NULL;
    dirp = opendir(DIRPATH);
    struct dirent *dr;
    while ((dr = readdir(dirp)) != NULL)
    {
        if(dr)
        {
            string fileName = dr->d_name;
            if(isPcapfileReady(fileName))
                 filesList.push_back(fileName);
        }
    }
    
    pthread_t threads[filesList.size()];

    for(it = filesList.begin(); it != filesList.end(); it++)
    {
        char *pcapFile = new char[it->length()];
        strcpy(pcapFile, it->c_str());

        cout << "Starting Thread for file:" << pcapFile << std::endl;
        rc = pthread_create(&threads[i], NULL, 
                          readPcapFile, pcapFile);
        if(rc)
            cout << "Error:: Failed to read pcap file:" << pcapFile << std::endl;

        i++;
      //  delete[] pcapFile;
    }
    i = 0;
    for (int i = 0; i < filesList.size(); i++)
    pthread_join(threads[i],NULL);
    filesList.clear();  
    (void)closedir(dirp);
    sleep (1);
 }

}

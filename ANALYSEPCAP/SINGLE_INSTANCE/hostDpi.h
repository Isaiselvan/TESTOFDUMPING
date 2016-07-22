#ifndef HOST_DPI_H
#define HOST_DPI_H
#include <stdlib.h>
#include <string.h>
#include "ndpi_main.h"
#include "libtrace_parallel.h"
#include "packetCmm.h"


#define NUM_ROOTS                 512

static u_int32_t detection_tick_resolution = 1000;
static std::string _protoFilePath = "./protos.txt";
class appLayer 
{
  struct ndpi_detection_module_struct *ndpi_struct;
  long long unsigned int protocol_counter[NDPI_MAX_SUPPORTED_PROTOCOLS + NDPI_MAX_NUM_CUSTOM_PROTOCOLS + 1];
  long long unsigned int protocol_counter_bytes[NDPI_MAX_SUPPORTED_PROTOCOLS + NDPI_MAX_NUM_CUSTOM_PROTOCOLS + 1]; 

  u_int32_t size_id_struct ;
  u_int32_t size_flow_struct ;

  typedef struct ndpi_flow {
  u_int32_t lower_ip;
  u_int32_t upper_ip;
  u_int16_t lower_port;
  u_int16_t upper_port;
  u_int8_t detection_completed, protocol;
  u_int16_t vlan_id;
  struct ndpi_flow_struct *ndpi_flow;
  char lower_name[48], upper_name[48];
  u_int8_t ip_version;
  u_int64_t last_seen;
  u_int64_t bytes;
  u_int32_t packets;

  // result only, not used for flow identification
  ndpi_protocol detected_protocol;

  char host_server_name[192];
  char bittorent_hash[41];

  struct {
    char client_certificate[48], server_certificate[48];
  } ssl;

  ndpi_id_struct *src, *dst;
  } ndpi_flow_t;

static void *malloc_wrapper(unsigned long size) {

  return malloc(size);
}

static void free_wrapper(void *freeable) {
  free(freeable);
}

static void debug_printf(u_int32_t protocol, void *id_struct,
                         ndpi_log_level_t log_level,
                         const char *format, ...) {
}

public :

  appLayer(){
//      ndpi_struct = ndpi_init_detection_module(detection_tick_resolution,
//   malloc_wrapper, free_wrapper, debug_printf);
   ndpi_struct = ndpi_init_detection_module();

  if(ndpi_struct == NULL) {
    printf("ERROR: global structure initialization failed\n");
    exit(-1);
  }
  NDPI_PROTOCOL_BITMASK all;
  NDPI_BITMASK_SET_ALL(all);
  ndpi_set_protocol_detection_bitmask2(ndpi_struct, &all);
  // Get size of id and flow struct
  size_id_struct = ndpi_detection_get_sizeof_ndpi_id_struct();
  size_flow_struct = ndpi_detection_get_sizeof_ndpi_flow_struct();
  //
  memset(protocol_counter, 0, sizeof(protocol_counter));
  memset(protocol_counter_bytes, 0, sizeof(protocol_counter_bytes));

    if(_protoFilePath.length())
    ndpi_load_protocols_file(ndpi_struct,(char *) _protoFilePath.c_str());

  } 

  ~appLayer(){
    //ndpi_exit_detection_module(ndpi_struct, free_wrapper);    
     ndpi_exit_detection_module(ndpi_struct);
   }

  int processPkt(libtrace_packet_t *, m_Packet&);//ndpi_detection_process_packet
  void printStat(std::string &);
};
#endif //HOST_DPI_H

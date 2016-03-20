#ifndef HOST_DPI_H
#define HOST_DPI_H
#include <stdlib.h>
#include <ndpi_main.h>
#include "libtrace_parallel.h"
#include "packetCmm.h"

static u_int32_t detection_tick_resolution = 1000;
static char *_protoFilePath = "./protos.txt";
class appLayer 
{
  struct ndpi_detection_module_struct *ndpi_struct;
  u_int64_t protocol_counter[NDPI_MAX_SUPPORTED_PROTOCOLS + NDPI_MAX_NUM_CUSTOM_PROTOCOLS + 1];
  u_int64_t protocol_counter_bytes[NDPI_MAX_SUPPORTED_PROTOCOLS + NDPI_MAX_NUM_CUSTOM_PROTOCOLS + 1]; 

  u_int32_t size_id_struct ;
  u_int32_t size_flow_struct ;

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
      ndpi_struct = ndpi_init_detection_module(detection_tick_resolution,
   malloc_wrapper, free_wrapper, debug_printf);

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

    if(_protoFilePath != NULL)
    ndpi_load_protocols_file(ndpi_struct, _protoFilePath);

  } 

  ~appLayer(){
    ndpi_exit_detection_module(ndpi_struct, free_wrapper);    
   }

  int processPkt(libtrace_packet_t *, m_Packet&);//ndpi_detection_process_packet
  void printStat(std::string &);
};
#endif //HOST_DPI_H

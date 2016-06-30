
#include "main.h"

uint32_t l2fwd_enabled_port_mask = 0;
unsigned int l2fwd_rx_queue_per_lcore = 1 ;
uint64_t max_size;

extern char * file_name ;
extern char file_name_moveG[1000];
extern char file_name_oldG[1000];
extern char file_name_rotated [1000];
extern int snaplen;
extern uint64_t seconds_rotation;
extern uint64_t max_packets ;
extern int64_t  max_rotations ;
extern uint64_t max_packets ;
extern uint64_t buffer_size ; //Ring size


int parse_args(int argc, char **argv)
{
        int option;


        /* Retrive arguments */
        while ((option = getopt(argc, argv,"w:c:B:G:W:C:S:i:f:b:q:p:")) != -1) {
                switch (option) {
                        case 'w' : file_name = strdup(optarg); /* File name, mandatory */
                                break;
                        case 'c': max_packets = atol (optarg); /* Max number of packets to save; default is infinite */
                                break;
                        case 'B': buffer_size = atoi (optarg); /* Buffer size in packets. Must be a power of two . Default is 1048576 */
                                break;
                        case 'G': seconds_rotation = atoi (optarg); /* Rotation of output in seconds. A progressive number will be added to file name */
                                break;
                        case 'W': max_rotations = atoi (optarg); /* Max rotations done. In case of 0, the program quits after first rotation time */
                                break;
                        case 'C': max_size = atoi (optarg); /* Max file size in KB. When reached, the program quits */
                                break;
                        case 'S': snaplen = atoi (optarg); /* Snap lenght default 96  */
                                break;
                                        /* portmask */
			case 'p':
				  l2fwd_enabled_port_mask = l2fwd_parse_portmask(optarg);
				  if (l2fwd_enabled_port_mask == 0) {
					  printf("invalid portmask\n");
					  return -1;
				  }
				  break;

				  /* nqueue */
			case 'q':
				  l2fwd_rx_queue_per_lcore = l2fwd_parse_nqueue(optarg);
				  if (l2fwd_rx_queue_per_lcore == 0) {
					  printf("invalid queue number\n");
					  return -1;
				  }
				  break;
				  /* long options */
			case 0:
				  return -1;

			default: return -1;
                }
        }

        /* Returning bad value in case of wrong arguments */
        if(file_name == NULL || isPowerOfTwo (buffer_size)!=1 )
                return -1;

        return 0;

}

int isPowerOfTwo (unsigned int x)
{
  return ((x != 0) && !(x & (x - 1)));
}


int
l2fwd_parse_portmask(const char *portmask)
{
        char *end = NULL;
        unsigned long pm;

        /* parse hexadecimal string */
        pm = strtoul(portmask, &end, 16);
        if ((portmask[0] == '\0') || (end == NULL) || (*end != '\0'))
                return -1;

        if (pm == 0)
                return -1;

        return pm;
}

unsigned int
l2fwd_parse_nqueue(const char *q_arg)
{
        char *end = NULL;
        unsigned long n;

        /* parse hexadecimal string */
        n = strtoul(q_arg, &end, 10);
        if ((q_arg[0] == '\0') || (end == NULL) || (*end != '\0'))
                return 0;
        if (n == 0)
                return 0;
        if (n >= MAX_RX_QUEUE_PER_LCORE)
                return 0;

        return n;
}




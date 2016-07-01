
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

struct lcore_conf lcore_conf[RTE_MAX_LCORE];
int Wlcore_list[RTE_MAX_LCORE] = {-1};
int nb_Wlcore = 0;



struct lcore_params lcore_params_array[MAX_LCORE_PARAMS];
struct lcore_params lcore_params_array_default[] = {
        {0, 0, 2},
        {0, 1, 2},
        {0, 2, 2},
        {1, 0, 2},
        {1, 1, 2},
        {1, 2, 2},
        {2, 0, 2},
        {3, 0, 3},
        {3, 1, 3}
};

struct lcore_params * lcore_params = lcore_params_array_default;
uint16_t nb_lcore_params = sizeof(lcore_params_array_default) /
                                sizeof(lcore_params_array_default[0]);

int
parse_config(const char *q_arg)
{
        char s[256];
        const char *p, *p0 = q_arg;
        char *end;
        enum fieldnames {
                FLD_PORT = 0,
                FLD_QUEUE,
                FLD_LCORE,
                _NUM_FLD
        };
        unsigned long int_fld[_NUM_FLD];
        char *str_fld[_NUM_FLD];
        int i;
        unsigned size;

        nb_lcore_params = 0;

        while ((p = strchr(p0,'(')) != NULL) {
                ++p;
                if((p0 = strchr(p,')')) == NULL)
                        return -1;

                size = p0 - p;
                if(size >= sizeof(s))
                        return -1;

                snprintf(s, sizeof(s), "%.*s", size, p);
                if (rte_strsplit(s, sizeof(s), str_fld, _NUM_FLD, ',') != _NUM_FLD)
                        return -1;
                for (i = 0; i < _NUM_FLD; i++){
                        errno = 0;
                        int_fld[i] = strtoul(str_fld[i], &end, 0);
                        if (errno != 0 || end == str_fld[i] || int_fld[i] > 255)
                                return -1;
                }
                if (nb_lcore_params >= MAX_LCORE_PARAMS) {
                        printf("exceeded max number of lcore params: %hu\n",
                                nb_lcore_params);
                        return -1;
                                  
        }
                lcore_params_array[nb_lcore_params].port_id =
                        (uint8_t)int_fld[FLD_PORT];
                lcore_params_array[nb_lcore_params].queue_id =
                        (uint8_t)int_fld[FLD_QUEUE];
                lcore_params_array[nb_lcore_params].lcore_id =
                        (uint8_t)int_fld[FLD_LCORE];
                ++nb_lcore_params;
        }
        lcore_params = lcore_params_array;
        return 0;
}


int parse_args(int argc, char **argv)
{
        int option, ret;
        const char *str5 = "FBM: Invalid config";
        char **argvopt;
        int option_index;
        //char *prgname = argv[0];
        static struct option lgopts[] = {
                {CMD_LINE_OPT_CONFIG, 1, 0, 0},
                {CMD_LINE_OPT_WCORE, 1,0,0},
                {NULL, 0, 0, 0}
               };

        argvopt = argv;
 
         /* Retrive arguments */
        while ((option = getopt_long(argc, argvopt,"w:c:B:G:W:C:S:i:f:b:q:p:",lgopts,&option_index)) != EOF) {
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
				   if (!strncmp(lgopts[option_index].name,
                                        CMD_LINE_OPT_CONFIG,
                                        sizeof(CMD_LINE_OPT_CONFIG))) {

                                ret = parse_config(optarg);
                                if (ret) {
                                        printf("%s\n", str5);
                                        //print_usage(prgname);
                                        return -1;
                                     }
                                }else if(!strncmp(lgopts[option_index].name,
                                        CMD_LINE_OPT_WCORE,
                                        sizeof(CMD_LINE_OPT_WCORE)))
                                 {
                                   ret = parse_Wcore(optarg);
                                     if (ret < 0) {
                                        printf("%s\n", str5);
                                        //print_usage(prgname);
                                        return -1;
                                       } 
                                 }
                                else 
                                    return -1;

                                 break;   

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

int parse_Wcore(const char *optargs)
{
   char s[256];
   char *str_fld[RTE_MAX_LCORE];
   char *end;
   int i;
// Wlcore_list
   if(optargs == NULL)
       return -1;
   //p0 = strchr(p,'\0') 
   snprintf(s, sizeof(s), "%s",optargs);
   nb_Wlcore = rte_strsplit(s, sizeof(s), str_fld, RTE_MAX_LCORE, ',');
   if(nb_Wlcore < 1)
         return -1;     
     for (i = 0; i < nb_Wlcore; i++)
       {
          errno = 0;
          Wlcore_list[i] = strtoul(str_fld[i], &end, 0);
             printf("Test1%d size%d\n",Wlcore_list[i], nb_Wlcore);
          if (errno != 0 || end == str_fld[i] || Wlcore_list[i] > 255)
             return -1;
       }    
    
  return nb_Wlcore;   
}

ifeq ($(RTE_SDK),)
$(error "Please define RTE_SDK environment variable")
endif

# Default target, can be overriden by command line or environment
RTE_TARGET ?= x86_64-native-linuxapp-gcc

include $(RTE_SDK)/mk/rte.vars.mk

LDLIBS = -lpcap -lpthread -lm  
#-lrte_pmd_e1000
#LDLIBS = /home/fbmpoc/Downloads/PF_RING/userland/libpcap/libpcap.a /home/fbmpoc/Downloads/PF_RING/userland/lib/libpfring.a -lpthread -lm 
#LDFLAGS = -Wl,--rpath -Wl,/usr/local/lib

# binary name
APP = icis-dump

ifdef dpdr
#SRCS-y := rte_eth_pcap.c 
SRCS-y += src_dpdr/main_dump.c
else
# all source are stored in SRCS-y
SRCS-y += src/main_dump.c
#SRCS-y := src/main_copy.c
endif 

CFLAGS += -O3
#CFLAGS += -g -ggdb
CFLAGS += $(WERROR_FLAGS)

EXPORT_MAP := rte_pmd_pcap_version.map
include $(RTE_SDK)/mk/rte.extapp.mk

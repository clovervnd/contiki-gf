#define CC1200_LANADA_SNIFFER 1

#ifndef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC csma_driver
#endif
#ifndef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC contikimac_driver
#endif

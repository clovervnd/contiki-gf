#ifndef __PROJECT_CONF_H__
#define __PROJECT_CONF_H__
#define CC1200_LANADA_SNIFFER 1

#ifndef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC csma_driver
//#define NETSTACK_CONF_MAC nullmac_driver
#endif
#ifndef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC cxmac_driver
//#define NETSTACK_CONF_RDC nullrdc_driver
#endif

#ifndef NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 2
#endif /* NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE */


#endif /* __PROJECT_CCONF_H__ */

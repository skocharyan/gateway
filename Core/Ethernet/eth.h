#ifndef ETH_H
#define ETH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

    typedef enum {
        DHCP_STATIC,
        DHCP_DYNAMIC,
        ETH_OFF
    } DhcpMode_t;

    typedef struct {
        DhcpMode_t dhcpMode;
        uint8_t ipAddress[4];
        uint8_t subnetMask[4];
        uint8_t gatewayAddress[4];
        uint8_t dnsAddress[4];
        uint32_t portNumber;
    } EthernetConfig;


void ethInit(void);

#ifdef __cplusplus
}
#endif

#endif // ETH_H
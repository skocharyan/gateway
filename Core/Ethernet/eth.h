#ifndef ETH_H
#define ETH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

    /* Portable 4-byte alignment macros for different compilers.
        IAR (ICCARM) places the attribute after the type, ARM/Keil uses __align(n)
        and GCC/Clang use __attribute__((aligned(n))).
    */


    typedef enum {
        DHCP_STATIC,
        DHCP_DYNAMIC,
        ETH_OFF
    } DhcpMode_t;

    typedef  struct __attribute__((aligned(8))) {
        DhcpMode_t dhcpMode;
        uint8_t ipAddress[4];
        uint8_t subnetMask[4];
        uint8_t gatewayAddress[4];
        uint8_t dnsAddress[4];
        uint32_t portNumber;
    }  EthernetConfig;


    void ethInit(void);

#ifdef __cplusplus
}
#endif

#endif // ETH_H
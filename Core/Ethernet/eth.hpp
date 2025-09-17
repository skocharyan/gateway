#ifndef ETH_H
#define ETH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "FreeRTOS_Routing.h"
#include "stdint.h"
#include "task.h"

typedef enum { DHCP_STATIC, DHCP_DYNAMIC, ETH_OFF } DhcpMode_t;

typedef struct __attribute__((aligned(8))) {
  DhcpMode_t dhcpMode;
  uint8_t ipAddress[4];
  uint8_t subnetMask[4];
  uint8_t gatewayAddress[4];
  uint8_t dnsAddress[4];
  uint32_t portNumber;
  uint8_t hostIPAddress[4];
} EthernetConfig;

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus
class Ethernet {
private:
  EthernetConfig config;
  uint8_t ucMACAddress[6];
  NetworkInterface_t xInterfaces[1];
  NetworkEndPoint_t xEndPoints[1];

  void printConfig();

public:
  Ethernet(EthernetConfig config);
};
#endif // __cplusplus

#endif // ETH_H

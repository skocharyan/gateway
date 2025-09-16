#include "eth.hpp"
#include "FreeRTOS.h"
#include "FreeRTOS_Routing.h"
#include "stdio.h"
#include "task.h"

// Private function prototypes
extern "C" NetworkInterface_t *
pxSTM32Fxx_FillInterfaceDescriptor(BaseType_t xEMACIndex,
                                   NetworkInterface_t *pxInterface);

extern "C" void generate_mac_address(uint8_t *device_id, uint8_t *mac_buffer);
// EOF prototypes

Ethernet::Ethernet(EthernetConfig config) : config{config} {

  printConfig();
  // Ethernet constructor initalization
  pxSTM32Fxx_FillInterfaceDescriptor(0, &(xInterfaces[0]));

  if (memcmp(config.ipAddress, (uint8_t[]){0, 0, 0, 0}, 4) == 0 ||
      memcmp(config.subnetMask, (uint8_t[]){0, 0, 0, 0}, 4) == 0 ||
      memcmp(config.gatewayAddress, (uint8_t[]){0, 0, 0, 0}, 4) == 0 ||
      config.portNumber == 0) {
    return;
  }

  //   uint16_t port = FreeRTOS_htons(config.portNumber);

  // Convert IP parameters to network byte order using
  // FreeRTOS_inet_addr_quick
  uint32_t ip =
      FreeRTOS_inet_addr_quick(config.ipAddress[0], config.ipAddress[1],
                               config.ipAddress[2], config.ipAddress[3]);
  uint32_t subnet =
      FreeRTOS_inet_addr_quick(config.subnetMask[0], config.subnetMask[1],
                               config.subnetMask[2], config.subnetMask[3]);
  uint32_t gateway = FreeRTOS_inet_addr_quick(
      config.gatewayAddress[0], config.gatewayAddress[1],
      config.gatewayAddress[2], config.gatewayAddress[3]);
  uint32_t dns =
      FreeRTOS_inet_addr_quick(config.dnsAddress[0], config.dnsAddress[1],
                               config.dnsAddress[2], config.dnsAddress[3]);

  memcpy((uint8_t *)0x1FFF7A10, ucMACAddress, sizeof(ucMACAddress));

  FreeRTOS_FillEndPoint(&(xInterfaces[0]), &(xEndPoints[0]), (uint8_t *)&ip,
                        (uint8_t *)&subnet, (uint8_t *)&gateway,
                        (uint8_t *)&dns, ucMACAddress);

#if (ipconfigUSE_DHCP != 0)
  {
    /* End-point 0 wants to use DHCPv4. */
    xEndPoints[0].bits.bWantDHCP = pdTRUE;
  }
#endif /* ( ipconfigUSE_DHCP != 0 ) */

  FreeRTOS_IPInit_Multi();
  printf("Ethernet initialized \n");
}

void Ethernet::printConfig() {
  printf("IP Address: %d.%d.%d.%d\n", config.ipAddress[0], config.ipAddress[1],
         config.ipAddress[2], config.ipAddress[3]);
  printf("Subnet Mask: %d.%d.%d.%d\n", config.subnetMask[0],
         config.subnetMask[1], config.subnetMask[2], config.subnetMask[3]);
  printf("Gateway Address: %d.%d.%d.%d\n", config.gatewayAddress[0],
         config.gatewayAddress[1], config.gatewayAddress[2],
         config.gatewayAddress[3]);
  printf("DNS Address: %d.%d.%d.%d\n", config.dnsAddress[0],
         config.dnsAddress[1], config.dnsAddress[2], config.dnsAddress[3]);
  printf("Port Number: %d\n", config.portNumber);
}
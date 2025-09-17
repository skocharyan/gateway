#include "eth.hpp"
#include "FreeRTOS.h"
#include "FreeRTOS_IP.h"
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

int32_t Ethernet::xTCPSend(const char *pcTxBuffer, size_t txLen) {
  Socket_t xSocket = FREERTOS_INVALID_SOCKET;
  struct freertos_sockaddr xRemoteAddress;
  size_t alreadyTransmitted = 0;
  int32_t result = -1;

  if (pcTxBuffer == NULL || txLen == 0) {
    return -1;
  }

  /* Diagnostic: ensure network is up before attempting to connect */
  if (FreeRTOS_IsNetworkUp() == pdFALSE) {
    printf("xTCPSend: network is not up yet\n");
    return -1;
  }

  memset(&xRemoteAddress, 0, sizeof(xRemoteAddress));
  xRemoteAddress.sin_port = FreeRTOS_htons(config.portNumber);
  xRemoteAddress.sin_address.ulIP_IPv4 = FreeRTOS_inet_addr_quick(
      config.hostIPAddress[0], config.hostIPAddress[1], config.hostIPAddress[2],
      config.hostIPAddress[3]);
  /* Ensure family constant matches socket creation */
  xRemoteAddress.sin_family = FREERTOS_AF_INET;

  xSocket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM,
                            FREERTOS_IPPROTO_TCP);
  configASSERT(xSocket != FREERTOS_INVALID_SOCKET);
  if (xSocket == FREERTOS_INVALID_SOCKET) {
    return -1;
  }

  TickType_t xSendTimeout = pdMS_TO_TICKS(5000);

  FreeRTOS_setsockopt(xSocket, 0, FREERTOS_SO_SNDTIMEO, &xSendTimeout,
                      sizeof(xSendTimeout));

  if (FreeRTOS_connect(xSocket, &xRemoteAddress, sizeof(xRemoteAddress)) != 0) {
    FreeRTOS_closesocket(xSocket);
    return -1;
  }
  while (alreadyTransmitted < txLen) {
    BaseType_t xBytesSent =
        FreeRTOS_send(xSocket, &pcTxBuffer[alreadyTransmitted],
                      (size_t)(txLen - alreadyTransmitted), 0);
    if (xBytesSent > 0) {
      alreadyTransmitted += (size_t)xBytesSent;
    } else {
      /* send error or timeout */
      FreeRTOS_shutdown(xSocket, FREERTOS_SHUT_RDWR);
      FreeRTOS_closesocket(xSocket);
      return -1;
    }
  }

  /* Optional: tell peer no more data will be sent (depends on protocol). */
  (void)FreeRTOS_shutdown(xSocket, FREERTOS_SHUT_WR);

  /* Receive loop */
  FreeRTOS_closesocket(xSocket);

  /* On success, return the number of bytes transmitted. Previously 'result'
   * was never updated, causing the function to always return -1. */
  result = (int32_t)alreadyTransmitted;
  return result;
}
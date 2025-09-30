#include "eth.hpp"
#include "FreeRTOS.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Routing.h"
#include "cstring"
#include "gate.hpp"
#include "stdio.h"
#include "task.h"
// Define the global task handle here
TaskHandle_t xUdpTaskHandle = nullptr;

// Private function prototypes
extern "C" NetworkInterface_t *
pxSTM32Fxx_FillInterfaceDescriptor(BaseType_t xEMACIndex,
                                   NetworkInterface_t *pxInterface);

extern "C" void generate_mac_address(uint8_t *device_id, uint8_t *mac_buffer);
// EOF prototypes

Ethernet::Ethernet(Config_t config) : config{config} {

  xUdpTaskHandle =
      xTaskCreateStatic(xUdpTask, "UDPTask", UDP_TASK_STACK_SIZE, this,
                        tskIDLE_PRIORITY + 5, xUdpStack, &xUdpTaskBuffer);

  xUdpTxTaskHandle =
      xTaskCreateStatic(xUDPTxTask, "UDPTxTask", UDP_TASK_STACK_SIZE, this,
                        tskIDLE_PRIORITY + 5, xUdpTxStack, &xUdpTxTaskBuffer);

  configASSERT(xUdpTaskHandle != nullptr);

  printConfig();
  // Ethernet constructor initalization
  pxSTM32Fxx_FillInterfaceDescriptor(0, &(xInterfaces[0]));

  if (memcmp(config.ipAddress, (uint8_t[]){0, 0, 0, 0}, 4) == 0 ||
      memcmp(config.subnetMask, (uint8_t[]){0, 0, 0, 0}, 4) == 0 ||
      memcmp(config.gatewayAddress, (uint8_t[]){0, 0, 0, 0}, 4) == 0 ||
      config.portNumber == 0) {
    return;
  }
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
}

void Ethernet::xUdpTask(void *params) {
  Ethernet *eth = static_cast<Ethernet *>(params);
  long lBytes;
  uint8_t cReceivedString[60];
  struct freertos_sockaddr xClient, xBindAddress;
  uint32_t xClientLength = sizeof(xClient);
  Socket_t xListeningSocket;

  /* Attempt to open the socket. */
  xListeningSocket = FreeRTOS_socket(
      FREERTOS_AF_INET, FREERTOS_SOCK_DGRAM, /* FREERTOS_SOCK_DGRAM for UDP */
      FREERTOS_IPPROTO_UDP);

  /* Check the socket was created. */
  configASSERT(xListeningSocket != FREERTOS_INVALID_SOCKET);

  memset(&xBindAddress, 0, sizeof(xBindAddress));
  xBindAddress.sin_port = FreeRTOS_htons((uint16_t)eth->config.portNumber);
  xBindAddress.sin_family = FREERTOS_AF_INET;
  FreeRTOS_bind(xListeningSocket, &xBindAddress, sizeof(xBindAddress));

  for (;;) {
    lBytes =
        FreeRTOS_recvfrom(xListeningSocket, cReceivedString,
                          sizeof(cReceivedString), 0, &xClient, &xClientLength);

    if (lBytes > 0) {
      printf("Received %ld bytes: '%.*s'\n", (long)lBytes, (int)lBytes,
             cReceivedString);

      if (strstr((const char *)cReceivedString, "PAUSE") != NULL) {
        printf("QR Paused\n");
        Gate::getInstance().pause();
      } else if (strstr((const char *)cReceivedString, "RESUME") != NULL) {
        printf("QR Resumed\n");
        Gate::getInstance().resume();
      }
    }
  }
}

void Ethernet::xUDPTxTask(void *params) {

  Ethernet *eth = static_cast<Ethernet *>(params);

  Socket_t xUDPTxSocket;
  struct freertos_sockaddr xUdpTxAddress;

  xUdpTxAddress.sin_family = FREERTOS_AF_INET;
  // Set destination IP to 192.168.0.100
  xUdpTxAddress.sin_address.ulIP_IPv4 = FreeRTOS_inet_addr_quick(
      eth->config.ipAddress[0], eth->config.ipAddress[1],
      eth->config.ipAddress[2], 255);
  xUdpTxAddress.sin_port = FreeRTOS_htons(eth->config.portNumber);

  xUDPTxSocket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_DGRAM,
                                 FREERTOS_IPPROTO_UDP);

  uint32_t notValue;

  while (1) {
    if (xTaskNotifyWait(0, UINT32_MAX, &notValue, portMAX_DELAY) == pdTRUE) {

      if (xUDPTxSocket == FREERTOS_INVALID_SOCKET) {
        xUDPTxSocket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_DGRAM,
                                       FREERTOS_IPPROTO_UDP);
      } else {
        BaseType_t sent = FreeRTOS_sendto(
            xUDPTxSocket, eth->ucUDPTxBuffer, eth->ucUDPDataLength, 0,
            &xUdpTxAddress, sizeof(xUdpTxAddress));
        printf("Sent %ld bytes\n", (long)sent);
      }
    }
  }
}
void Ethernet::xUDPSendISR(const char *pcTxBuffer, size_t txLen) {
  memcpy(ucUDPTxBuffer, pcTxBuffer, txLen);
  ucUDPDataLength = txLen;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xTaskNotifyFromISR(xUdpTxTaskHandle, 0, eSetValueWithOverwrite,
                     &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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

  if (xSocket == FREERTOS_INVALID_SOCKET) {
    printf("Socket creation failed\n");
    return -1;
  }

  TickType_t xSendTimeout = pdMS_TO_TICKS(5000);

  FreeRTOS_setsockopt(xSocket, 0, FREERTOS_SO_SNDTIMEO, &xSendTimeout,
                      sizeof(xSendTimeout));

  if (FreeRTOS_connect(xSocket, &xRemoteAddress, sizeof(xRemoteAddress)) != 0) {
    FreeRTOS_closesocket(xSocket);
    return ERR_SOCKET_CREATE;
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

int32_t Ethernet::xTCPSendAndReceive(const char *pcTxBuffer, size_t txLen,
                                     TickType_t recvTimeoutMs) {

  struct freertos_sockaddr xRemoteAddress;
  enum { LOCAL_RX_BUF_SIZE = 256 };
  char localRx[LOCAL_RX_BUF_SIZE];

  if (pcTxBuffer == NULL || txLen == 0) {
    return ERR_PARAM;
  }

  const int maxAttempts = 2;
  // printf("xTCPSendAndReceive: starting, txLen=%lu, recvTimeoutMs=%lu\n",
  //        (unsigned long)txLen, (unsigned long)recvTimeoutMs);

  for (int attempt = 0; attempt < maxAttempts; ++attempt) {
    Socket_t xSocket = FREERTOS_INVALID_SOCKET;
    size_t alreadyTransmitted = 0;
    size_t alreadyReceived = 0;
    // printf("Attempt %d/%d\n", attempt + 1, maxAttempts);

    if (FreeRTOS_IsNetworkUp() == pdFALSE) {
      // printf("xTCPSendAndReceive: network is not up yet\n");
      return ERR_NET_DOWN;
    }

    memset(&xRemoteAddress, 0, sizeof(xRemoteAddress));
    xRemoteAddress.sin_port = FreeRTOS_htons(config.portNumber);
    xRemoteAddress.sin_address.ulIP_IPv4 = FreeRTOS_inet_addr_quick(
        config.hostIPAddress[0], config.hostIPAddress[1],
        config.hostIPAddress[2], config.hostIPAddress[3]);
    xRemoteAddress.sin_family = FREERTOS_AF_INET;

    xSocket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM,
                              FREERTOS_IPPROTO_TCP);
    configASSERT(xSocket != FREERTOS_INVALID_SOCKET);
    if (xSocket == FREERTOS_INVALID_SOCKET) {
      if (attempt + 1 < maxAttempts) {
        vTaskDelay(pdMS_TO_TICKS(100));
        continue;
      }
      return ERR_SOCKET_CREATE;
    }

    TickType_t xRecvTimeout = pdMS_TO_TICKS(recvTimeoutMs); /* param in ms */
    TickType_t xSendTimeout = pdMS_TO_TICKS(5000);
    FreeRTOS_setsockopt(xSocket, 0, FREERTOS_SO_RCVTIMEO, &xRecvTimeout,
                        sizeof(xRecvTimeout));
    FreeRTOS_setsockopt(xSocket, 0, FREERTOS_SO_SNDTIMEO, &xSendTimeout,
                        sizeof(xSendTimeout));

    if (FreeRTOS_connect(xSocket, &xRemoteAddress, sizeof(xRemoteAddress)) !=
        0) {
      // printf("Connect failed (attempt %d)\n", attempt + 1);
      FreeRTOS_closesocket(xSocket);
      if (attempt + 1 < maxAttempts) {
        vTaskDelay(pdMS_TO_TICKS(100));
        continue;
      }
      return ERR_CONNECT;
    }

    while (alreadyTransmitted < txLen) {
      BaseType_t xBytesSent =
          FreeRTOS_send(xSocket, &pcTxBuffer[alreadyTransmitted],
                        (size_t)(txLen - alreadyTransmitted), 0);
      if (xBytesSent > 0) {
        alreadyTransmitted += (size_t)xBytesSent;
      } else {
        FreeRTOS_shutdown(xSocket, FREERTOS_SHUT_RDWR);
        FreeRTOS_closesocket(xSocket);
        if (attempt + 1 < maxAttempts) {
          vTaskDelay(pdMS_TO_TICKS(100));
          goto attempt_end; /* break outer attempt loop */
        }
        return ERR_SEND;
      }
    }

    /* Receive loop */
    for (;;) {
      if (alreadyReceived >= LOCAL_RX_BUF_SIZE) {
        break;
      }
      BaseType_t xBytesReceived =
          FreeRTOS_recv(xSocket, &localRx[alreadyReceived],
                        (size_t)(LOCAL_RX_BUF_SIZE - alreadyReceived), 0);
      if (xBytesReceived > 0) {
        alreadyReceived += (size_t)xBytesReceived;
        if (alreadyReceived < LOCAL_RX_BUF_SIZE) {
          localRx[alreadyReceived] = '\0';
        }
        continue; /* try read more until timeout/close */
      } else if (xBytesReceived == 0) {
        break;
      } else {
        alreadyReceived = 0;
        FreeRTOS_shutdown(xSocket, FREERTOS_SHUT_RDWR);
        FreeRTOS_closesocket(xSocket);
        if (attempt + 1 < maxAttempts) {
          vTaskDelay(pdMS_TO_TICKS(100));
          goto attempt_end; /* try next attempt */
        }
        return ERR_RECV;
      }
    }

    (void)FreeRTOS_shutdown(xSocket, FREERTOS_SHUT_RDWR);
    FreeRTOS_closesocket(xSocket);
    if (alreadyReceived > 0) {
      if (strstr(localRx, "OK") != NULL) {
        // printf("Received Qr Confirmation \n");
        return 0;
      }
    }

  attempt_end:;
    if (attempt + 1 < maxAttempts) {
      continue; /* next attempt */
    }
  }
  return ERR_NO_OK;
}
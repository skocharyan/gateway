#ifndef ETH_H
#define ETH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "FreeRTOS_Routing.h"
#include "stdint.h"
#include "task.h"

// Use a C-compatible constant to support C and C++ compilers
#define UDP_TASK_STACK_SIZE 512U

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

enum ErrorCodes {
  ERR_PARAM = -10,
  ERR_NET_DOWN = -11,
  ERR_SOCKET_CREATE = -12,
  ERR_CONNECT = -13,
  ERR_SEND = -14,
  ERR_RECV = -15,
  ERR_NO_OK = -16
};

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus

// Provide extern declaration; define in eth.cpp
extern TaskHandle_t xUdpTaskHandle;

class Ethernet {
private:
  EthernetConfig config;
  uint8_t ucMACAddress[6];
  NetworkInterface_t xInterfaces[1];
  NetworkEndPoint_t xEndPoints[1];

  StaticTask_t xUdpTaskBuffer;
  StackType_t xUdpStack[UDP_TASK_STACK_SIZE];

  Socket_t xUDPTxSocket{nullptr};
  // Rename to match usage in eth.cpp
  struct freertos_sockaddr xUdpTxAddress;

  void printConfig();

  static void xUdpTask(void *params);

public:
  Ethernet(EthernetConfig config);
  int32_t xTCPSend(const char *pcTxBuffer, size_t txLen);
  /*
   * xTCPSendAndReceive
   * pcTxBuffer    : data to send (not null-terminated requirement, length in
   * txLen) txLen         : number of bytes to transmit recvTimeoutMs : receive
   * timeout in milliseconds (converted internally via pdMS_TO_TICKS) Returns 0
   * on success ("OK" substring received), -1 on failure.
   */
  int32_t xTCPSendAndReceive(const char *pcTxBuffer, size_t txLen,
                             TickType_t recvTimeoutMs);
  int32_t xUDPSend(const char *pcTxBuffer, size_t txLen);
};
#endif // __cplusplus

#endif // ETH_H

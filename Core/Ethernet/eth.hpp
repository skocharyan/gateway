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

class Ethernet {
public:
  TaskHandle_t xUdpTaskHandle;
  TaskHandle_t xUdpTxTaskHandle;

private:
  EthernetConfig config;
  uint8_t ucMACAddress[6];
  NetworkInterface_t xInterfaces[1];
  NetworkEndPoint_t xEndPoints[1];

  StaticTask_t xUdpTaskBuffer;
  StackType_t xUdpStack[UDP_TASK_STACK_SIZE];

  StaticTask_t xUdpTxTaskBuffer;
  StackType_t xUdpTxStack[UDP_TASK_STACK_SIZE];

  uint8_t ucUDPTxBuffer[256];
  uint8_t ucUDPDataLength;

  void printConfig();

  static void xUdpTask(void *params);
  static void xUDPTxTask(void *params);

public:
  Ethernet(EthernetConfig config);

  int32_t xTCPSend(const char *pcTxBuffer, size_t txLen);

  int32_t xTCPSendAndReceive(const char *pcTxBuffer, size_t txLen,
                             TickType_t recvTimeoutMs);

  void xUDPSendISR(const char *pcTxBuffer, size_t txLen);
};
#endif // __cplusplus

#endif // ETH_H

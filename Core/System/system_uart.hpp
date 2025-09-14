#ifndef SYSTEM_UART_H
#define SYSTEM_UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "AppConfig.h"
#include "FreeRTOS.h"
#include "stdint.h"
#include "task.h"

class SystemUart {
private:
  uint8_t dmaBuffer[RX_BUFFER_SIZE]{0};
  uint8_t dmaProcessBuffer[RX_BUFFER_SIZE]{0};

  volatile uint32_t qrLastPos{0};

  TaskHandle_t qrTaskHandle{NULL};

public:
  SystemUart(TaskHandle_t qrTaskHandle);
  void process(void);
};

#ifdef __cplusplus
}
#endif

#endif
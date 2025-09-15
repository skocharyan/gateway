#ifndef QR_H
#define QR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "AppConfig.h"
#include "FreeRTOS.h"
#include "stdint.h"
#include "task.h"

void System_Init();

class SystemThread {
private:
  TaskHandle_t qrTaskHandle;
  uint8_t dmaProcessBuffer[RX_BUFFER_SIZE]{0};

  StaticTask_t xTaskBuffer;
  StackType_t xTaskStack[QR_TAKS_STACK_SIZE];

  static void threadFunction(void *args);

public:
  SystemThread();
  TaskHandle_t getTaskHandle() const;
  uint8_t (&getDataBufferRef())[RX_BUFFER_SIZE];
};

#ifdef __cplusplus
}
#endif

#endif
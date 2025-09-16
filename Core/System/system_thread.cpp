#include "system_thread.hpp"
#include "FreeRTOS.h"
#include "cstring"
#include "stdio.h"
#include "task.h"

extern "C" int32_t xTCPSend(const char *pcTxBuffer, size_t txLen,
                            uint16_t port);

SystemThread::SystemThread() {
  qrTaskHandle = xTaskCreateStatic(SystemThread::threadFunction, "QR_Task",
                                   QR_TAKS_STACK_SIZE, this, QR_TASK_PRIORITY,
                                   xTaskStack, &xTaskBuffer);
  if (qrTaskHandle != NULL) {
    printf("task is created \n");
  } else {
    printf("task is not creted \n");
  }
}

TaskHandle_t SystemThread::getTaskHandle() const { return qrTaskHandle; }

uint8_t (&SystemThread::getDataBufferRef())[RX_BUFFER_SIZE] {
  return dmaProcessBuffer;
}

void SystemThread::threadFunction(void *params) {

  SystemThread *self = static_cast<SystemThread *>(params);
  (void)self;

  uint32_t notValue;

  // if (getGateActualState() == IDLE) {
  //     handleGate(CLOSE);  // close idle state;
  // }

  while (1) {
    if (xTaskNotifyWait(0, UINT32_MAX, &notValue, QR_WAITING_TIME_MS) ==
        pdTRUE) {
      printf("QR data: %s\n", self->dmaProcessBuffer);
      printf("Notified value: %lu\n", notValue);
      uint16_t port = 7788;
      int32_t sentBytes =
          xTCPSend((const char *)self->dmaProcessBuffer,
                   strlen((const char *)self->dmaProcessBuffer), port);

      printf("Sent bytes: %ld\n", sentBytes);
      // uint8_t qrRespBuffer[RX_BUFFER_SIZE];

      // int32_t xBytesReceived = xTCPSendAndReceive((const
      // char*)qrDataBuffer, (size_t)notValue, (char*)qrRespBuffer,
      // RX_BUFFER_SIZE, QR_WAIT_TIMEOUT); if (xBytesReceived <= 0) {
      //     continue; // failed to verify the QR code
      // }

      // if (strcmp((const char*)qrRespBuffer, QR_SUCCESS_RESPONSE) != 0)
      // {
      //     continue; // invalid response
      // }

      // handleGate(OPEN);
    }
  }
}
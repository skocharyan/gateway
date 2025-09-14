#include "system_thread.hpp"
#include "FreeRTOS.h"
#include "task.h"

#pragma location = ".ccmram"
StaticTask_t xTaskBuffer;
#pragma location = ".ccmram"
StackType_t xTaskStack[QR_TAKS_STACK_SIZE];

SystemThread::SystemThread() {
  qrTaskHandle = xTaskCreateStatic(SystemThread::threadFunction, "QR_Task",
                                   QR_TAKS_STACK_SIZE, this, QR_TASK_PRIORITY,
                                   xTaskStack, &xTaskBuffer);
}

TaskHandle_t SystemThread::getTaskHandle() const { return qrTaskHandle; }

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
      // printf("QR data: %s\n", qrDataBuffer);
      // printf("Notified value: %lu\n", notValue);

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
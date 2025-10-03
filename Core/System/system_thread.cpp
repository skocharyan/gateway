#include "system_thread.hpp"
#include "FreeRTOS.h"
#include "cstring"
#include "eth.hpp"
#include "gate.hpp"
#include "stdio.h"
#include "task.h"

extern "C" int32_t xTCPSend(const char *pcTxBuffer, size_t txLen,
                            uint16_t port);
extern Ethernet *ethernetInstance;

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

  Gate &gate = Gate::getInstance();

  uint32_t notValue;

  while (1) {
    if (xTaskNotifyWait(0, UINT32_MAX, &notValue, QR_WAITING_TIME_MS) ==
        pdTRUE) {
      if (self->state == SYSTEM_IDLE) {
        self->state = SYSTEM_BUSY;
        int32_t success = ethernetInstance->xTCPSendAndReceive(
            (const char *)self->dmaProcessBuffer,
            strlen((const char *)self->dmaProcessBuffer), 500);
        if (success == 0) {
          xTaskNotify(gate.gateTaskHandle, 1, eNoAction);
          self->state = SYSTEM_IDLE;
        }

      } else {
        printf("System is busy, ignoring new data\n");
      }
    }
  }
}
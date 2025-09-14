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

  static void threadFunction(void *args);

public:
  SystemThread();
  TaskHandle_t getTaskHandle() const;
};

#ifdef __cplusplus
}
#endif

#endif
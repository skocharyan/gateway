#ifndef GATE_H
#define GATE_H

#include "AppConfig.h"
#include "FreeRTOS.h"
#include "spi.h"
#include "system_thread.hpp"
#include "task.h"
#include "timers.h"

enum GateStatus { NONE, OPENING, CLOSING, WAITING };

enum GateState { OPENED, CLOSED, UNDEFINED };

enum GateAction { OPEN, CLOSE, IDLE };

enum SoftTimerStatus { STOPPED, RUNNING, EXPIRED };

extern SystemThread *systemThreadInstance;

class Gate {
private:
  TimerHandle_t softTimer;

  StaticTimer_t softTimerBuffer;

  GateStatus gateStatus{NONE};

  GateState gateState{UNDEFINED};

  StaticTask_t gateTaskBuffer;

  StackType_t gateTaskStack[GATE_TASK_STACK_SIZE];

  TaskHandle_t gateMonitorTaskHandle;

  StaticTask_t gateMonitorTaskBuffer;

  StackType_t gateMonitorTaskStack[GATE_TASK_STACK_SIZE];

  SoftTimerStatus timerStatus{RUNNING};

  volatile bool suspendMotorTask{false};

  volatile TickType_t scanTimeout = GATE_SCAN_INIT_TIMEOUT;

  TaskHandle_t gateIWDGTaskHandle;

  StaticTask_t gateIWDGTaskBuffer;

  StackType_t gateIWDGTaskStack[GATE_TASK_STACK_SIZE / 2];

  void restartTimerISR(void);

  void restartTimer(void);

  float getInstantCurrent(void);

  void controlGateMotor(GateAction);

  static void gateControlTask(void *params);

  static void gateMonitorTask(void *params);

  static void gateIWDGTask(void *params);

public:
  // Get the singleton instance. Creates it on first call.
  static Gate &getInstance();

private:
  // Constructor is private for singleton enforcement
  // Deleted copy/move to enforce singleton
  Gate(const Gate &) = delete;
  Gate &operator=(const Gate &) = delete;
  Gate(Gate &&) = delete;
  Gate &operator=(Gate &&) = delete;

  ~Gate() = default;

  Gate();

public:
  TaskHandle_t gateTaskHandle;

  GateState getGateActualState(void);

  void handleOpenedState(void);

  void handleClosedState(void);

  void timerCallback(TimerHandle_t xTimer);

  void gateHandler(GateAction action);

  void pause() {
    if (systemThreadInstance == nullptr) {
      /* System thread not initialized yet - nothing to pause. */
      return;
    }
    TaskHandle_t handle = systemThreadInstance->getTaskHandle();
    if (handle == NULL) {
      return;
    }
    /* Suspend the system thread task from task context. Do NOT call
       vTaskSuspendFromISR - suspending from ISRs isn't supported. */
    vTaskSuspend(handle);
    timerStatus = SoftTimerStatus::STOPPED;
  };
  void resume() {
    if (systemThreadInstance == nullptr) {
      /* System thread not initialized yet - nothing to resume. */
      return;
    }
    TaskHandle_t handle = systemThreadInstance->getTaskHandle();
    if (handle == NULL) {
      return;
    }
    vTaskResume(handle);
    gateHandler(GateAction::CLOSE); // automaticaly close after resume
    timerStatus = SoftTimerStatus::RUNNING;
  };
};

#endif
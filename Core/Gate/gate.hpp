#ifndef GATE_H
#define GATE_H

#include "AppConfig.h"
#include "FreeRTOS.h"
#include "spi.h"
#include "task.h"
#include "timers.h"

enum GateStatus { NONE, OPENING, CLOSING, WAITING };

enum GateState { OPENED, CLOSED, UNDEFINED };

enum GateAction { OPEN, CLOSE, IDLE };

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

  volatile bool suspendMotorTask{false};

  volatile TickType_t scanTimeout = GATE_SCAN_INIT_TIMEOUT;

  void restartTimerISR(void);

  void restartTimer(void);

  float getInstantCurrent(void);

  void controlGateMotor(GateAction);

  static void gateControlTask(void *params);

  static void gateMonitorTask(void *params);

public:
  // Get the singleton instance. Creates it on first call.
  static Gate &getInstance();

  // Deleted copy/move to enforce singleton
  Gate(const Gate &) = delete;
  Gate &operator=(const Gate &) = delete;
  Gate(Gate &&) = delete;
  Gate &operator=(Gate &&) = delete;

  ~Gate() = default;

private:
  // Constructor is private for singleton enforcement
  Gate();

public:
  TaskHandle_t gateTaskHandle;

  GateState getGateActualState(void);

  void handleOpenedState(void);

  void handleClosedState(void);

  void timerCallback(TimerHandle_t xTimer);

  void gateHandler(GateAction action);
};

#endif
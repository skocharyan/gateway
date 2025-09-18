#ifndef GATE_H
#define GATE_H

#include "AppConfig.h"
#include "FreeRTOS.h"
#include "spi.h"
#include "task.h"
#include "timers.h"

typedef enum { NONE, OPEN, CLOSE } GateAction;

typedef enum { IDLE, OPENING, CLOSING, OPENED, CLOSED } GateState;

class Gate {
private:
  TimerHandle_t softTimer;

  StaticTimer_t softTimerBuffer;

  GateAction gateAction = NONE;

  GateState gateState = IDLE;

  TaskHandle_t gateTaskHandle;

  StaticTask_t gateTaskBuffer;

  StackType_t gateTaskStack[GATE_TASK_STACK_SIZE];

  volatile TickType_t scanTimeout = GATE_SCAN_INIT_TIMEOUT;

  void gateHandler(GateAction action);

  void resetTimer(void);

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
  GateState getGateActualState(void);

  GateState getGateState(void);

  void handleOpenedState(void);

  void handleClosedState(void);

  void timerCallback(TimerHandle_t xTimer);
};

#endif
#ifndef GATE_H
#define GATE_H

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

typedef enum { NONE, OPEN, CLOSE } GateAction;

typedef enum { IDLE, OPENING, CLOSING, OPENED, CLOSED } GateState;

class Gate {
private:
  TimerHandle_t softTimer;
  GateAction gateAction = NONE;
  GateState gateState = IDLE;

  void timerCallback(TimerHandle_t xTimer);

  void gateHandler(GateAction action);

  void resetTimer(void);

public:
  GateState getGateActualState(void);

  GateState getGateState(void);

  void handleOpenedState(void);

  void handleClosedState(void);
};

#endif
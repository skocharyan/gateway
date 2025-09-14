
#include "gate.hpp"

#include "FreeRTOS.h"
#include "main.h"
#include "timers.h"

extern Gate *gateInstance;

void Gate::timerCallback(TimerHandle_t xTimer) {
  gateState = CLOSED; // start the closing process
  gateHandler(CLOSE);
}

void Gate::resetTimer(void) { xTimerReset(softTimer, 0); }

void Gate::handleOpenedState(void) {
  // start the timer
  // after timer is elapsed start closing the gate
  resetTimer();
  gateState = OPENED;
  gateHandler(NONE); // disable motors
}
void Gate::handleClosedState(void) {
  gateState = CLOSING;
  gateHandler(NONE); // disable motors
}

GateState Gate::getGateActualState(void) {
  if (LL_GPIO_IsInputPinSet(OPEN_SW_GPIO_Port, OPEN_SW_Pin)) {
    return OPENED;
  } else if (LL_GPIO_IsInputPinSet(CLOSE_SW_GPIO_Port, CLOSE_SW_Pin)) {
    return CLOSED;
  }
  return IDLE;
}

GateState Gate::getGateState(void) { return gateState; }

void Gate::gateHandler(GateAction action) { __NOP(); }

extern "C" {
void handleOpeneState(void) { gateInstance->handleOpenedState(); }
void handleCloseState(void) { gateInstance->handleClosedState(); }
}

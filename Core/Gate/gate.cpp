
#include "gate.hpp"
#include "FreeRTOS.h"
#include "main.h"
#include "spi.h"
#include "tim.h"

void PWM_SetDutyCycle(uint8_t dutyCycle);

// Singleton accessor implementation
Gate &Gate::getInstance() {
  static Gate instance;
  return instance;
}

extern "C" void gate_timer_callback_wrapper(TimerHandle_t xTimer) {
  // Prefer the timer's ID (we pass `this` as the ID when creating the timer)
  void *id = pvTimerGetTimerID(xTimer);
  Gate *g = static_cast<Gate *>(id);
  if (g) {
    g->timerCallback(xTimer);
    return;
  }
}

void Gate::gateMonitorTask(void *params) {

  Gate *g = static_cast<Gate *>(params);
  for (;;) {
    vTaskDelay(g->scanTimeout);
  }
}

Gate::Gate() {

  softTimer = xTimerCreateStatic("GateTimer", pdMS_TO_TICKS(GATE_WAIT_TIMEOUT),
                                 pdFALSE, (void *)this,
                                 gate_timer_callback_wrapper, &softTimerBuffer);

  gateTaskHandle =
      xTaskCreateStatic(gateMonitorTask, "GateTask", GATE_TASK_STACK_SIZE, this,
                        GATE_TASK_PRIORITY, gateTaskStack, &gateTaskBuffer);

  vTaskSuspend(gateTaskHandle);

  MX_TIM9_Init(); // Initialie the pwm timer

  MX_SPI1_Init(); // Initialize the SPI for current sensing
}

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
void handleOpenedState(void) { Gate::getInstance().handleOpenedState(); }

void handleOpeneState(void) { handleOpenedState(); }

void handleCloseState(void) { Gate::getInstance().handleClosedState(); }
}

#include "gate.hpp"
#include "FreeRTOS.h"
#include "main.h"
#include "spi.h"
#include "tim.h"

constexpr float MAX_INST_CURRENT =
    0; // Placeholder threshold; implement real value later

// Singleton accessor implementation
Gate &Gate::getInstance() {
  static Gate instance;
  return instance;
}

extern "C" void gate_timer_callback_wrapper(TimerHandle_t xTimer) {
  // Prefer the timer's ID (we pass `this` as the ID when creating the timer)
  void *id = pvTimerGetTimerID(xTimer);
  Gate *gateInstance = static_cast<Gate *>(id);
  if (gateInstance) {
    gateInstance->timerCallback(xTimer);
  }
}

void Gate::gateControlTask(void *params) {
  Gate *gate = static_cast<Gate *>(params);
  uint32_t notValue;
  for (;;) {
    if (xTaskNotifyWait(0, UINT16_MAX, &notValue, portMAX_DELAY) == pdTRUE) {
      GateState currentState = gate->getGateActualState();
      if (currentState == CLOSED || currentState == UNDEFINED) {
        gate->gateHandler(OPEN);
      } else if (currentState == OPENED) {
        gate->gateHandler(IDLE);
      }
    }
  }
}

void Gate::gateMonitorTask(void *params) {
  Gate *gate = static_cast<Gate *>(params);
  GateStatus preStopStatus = GateStatus::NONE; // last active (OPENING/CLOSING)
  bool motorStoppedByOverCurrent = false;
  for (;;) {
    float current = gate->getInstantCurrent();

    // Only monitor when gate is moving
    GateStatus gs = gate->gateStatus;
    if (gs == GateStatus::OPENING || gs == GateStatus::CLOSING) {
      if (current > MAX_INST_CURRENT) {
        // Capture state and stop
        preStopStatus = gs;
        gate->controlGateMotor(GateAction::IDLE);
        gate->gateStatus = GateStatus::WAITING; // mark not moving
        motorStoppedByOverCurrent = true;
      }
    } else if (motorStoppedByOverCurrent) {
      // Placeholder auto-restart policy (disabled until real thresholds
      // implemented) When real hysteresis exists, restart logic can go here.
      // For now we keep motor stopped; clear flag so we don't loop.
      motorStoppedByOverCurrent = false;
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

Gate::Gate() {

  softTimer = xTimerCreateStatic("GateTimer", pdMS_TO_TICKS(GATE_WAIT_TIMEOUT),
                                 pdFALSE, (void *)this,
                                 gate_timer_callback_wrapper, &softTimerBuffer);

  gateTaskHandle =
      xTaskCreateStatic(gateControlTask, "GateTask", GATE_TASK_STACK_SIZE, this,
                        GATE_TASK_PRIORITY, gateTaskStack, &gateTaskBuffer);

  gateMonitorTaskHandle = xTaskCreateStatic(
      gateMonitorTask, "Gate Monitor", GATE_TASK_STACK_SIZE, this,
      GATE_TASK_PRIORITY, gateMonitorTaskStack, &gateMonitorTaskBuffer);

  MX_TIM9_Init(); // Initialie the pwm timer

  MX_SPI1_Init(); // Initialize the SPI for current sensing

  vTaskSuspend(gateMonitorTaskHandle);
}

void Gate::timerCallback(TimerHandle_t xTimer) { gateHandler(CLOSE); }

void Gate::resetTimer(void) { xTimerReset(softTimer, 0); }

void Gate::handleOpenedState(void) {
  // This function is called by IQR \
  // when the gate is fully opened
  // 1. disable motors
  // 2. sart wait timer
  // 3. set staus waiting

  gateStatus = GateStatus::WAITING;
  resetTimer();
  controlGateMotor(GateAction::IDLE);
  vTaskSuspend(gateMonitorTaskHandle); // thisable the current mointoring
}

void Gate::handleClosedState(void) {
  // This function is called by IQR \
  // when the gate is fully closed
  // 1. disable motor
  // 2. set status waiting
  gateStatus = GateStatus::WAITING;
  controlGateMotor(GateAction::IDLE);
  vTaskSuspend(gateMonitorTaskHandle); // thisable the current mointoring
}

GateState Gate::getGateActualState(void) {
  if (LL_GPIO_IsInputPinSet(OPEN_SW_GPIO_Port, OPEN_SW_Pin)) {
    return OPENED;
  } else if (LL_GPIO_IsInputPinSet(CLOSE_SW_GPIO_Port, CLOSE_SW_Pin)) {
    return CLOSED;
  }
  return UNDEFINED;
}

void Gate::gateHandler(GateAction action) {
  GateState currentState = getGateActualState();

  if (action == GateAction::OPEN) {
    if (currentState != GateState::OPENED) {
      gateStatus = GateStatus::OPENING;
      controlGateMotor(action); // Start motor first, then enable monitoring
      vTaskResume(gateMonitorTaskHandle);
    } else {
      // Gate is already fully open
      gateStatus = GateStatus::WAITING;
      controlGateMotor(GateAction::IDLE);
    }
  } else if (action == GateAction::CLOSE) {
    if (currentState != GateState::CLOSED) {
      gateStatus = GateStatus::CLOSING;
      controlGateMotor(action); // Start motor first, then enable monitoring
      vTaskResume(gateMonitorTaskHandle);
    } else {
      // Gate is already fully closed
      gateStatus = GateStatus::WAITING;
      controlGateMotor(GateAction::IDLE);
    }
  } else if (action == GateAction::IDLE) {
    // Stop the motor and wait
    gateStatus = GateStatus::WAITING;
    controlGateMotor(action);
  }
}

void Gate::controlGateMotor(GateAction action) {
  if (action == GateAction::CLOSE) {
    LL_GPIO_SetOutputPin(MOTOR_DIR_GPIO_Port, MOTOR_DIR_Pin);
    LL_GPIO_ResetOutputPin(MOTOR_DIR_2_GPIO_Port, MOTOR_DIR_2_Pin);
    PWM_SetDutyCycle(100);
  } else if (action == GateAction::OPEN) {
    LL_GPIO_ResetOutputPin(MOTOR_DIR_GPIO_Port, MOTOR_DIR_Pin);
    LL_GPIO_SetOutputPin(MOTOR_DIR_2_GPIO_Port, MOTOR_DIR_2_Pin);
    PWM_SetDutyCycle(100);
  }
}

float Gate::getInstantCurrent(void) { return 11.0f; }

extern "C" {
void handleOpenedState(void) { Gate::getInstance().handleOpenedState(); }

void handleClosedState(void) { Gate::getInstance().handleClosedState(); }
}

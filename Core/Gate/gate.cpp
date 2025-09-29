#include "gate.hpp"
#include "FreeRTOS.h"
#include "eth.hpp"
#include "main.h"
#include "spi.h"
#include "stdio.h"
#include "tim.h"

extern Ethernet *ethernetInstance;

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
    if (xTaskNotifyWait(0, UINT16_MAX, &notValue, pdMS_TO_TICKS(5000)) ==
        pdTRUE) {
      GateState currentState = gate->getGateActualState();
      if (currentState == CLOSED || currentState == UNDEFINED) {
        gate->gateHandler(OPEN);
      } else if (currentState == OPENED) {
        gate->restartTimer();
        gate->gateHandler(IDLE);
      }
    } else if (gate->getGateActualState() == UNDEFINED &&
               (gate->gateStatus == NONE || gate->gateStatus == WAITING)) {
      gate->gateHandler(CLOSE);
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
    // if (gs == GateStatus::OPENING || gs == GateStatus::CLOSING) {
    //   if (current > MAX_INST_CURRENT) {
    //     // Capture state and stop
    //     preStopStatus = gs;
    //     gate->controlGateMotor(GateAction::IDLE);
    //     gate->gateStatus = GateStatus::WAITING; // mark not moving
    //     motorStoppedByOverCurrent = true;
    //   }
    // } else if (motorStoppedByOverCurrent) {
    //   // Placeholder auto-restart policy (disabled until real thresholds
    //   // implemented) When real hysteresis exists, restart logic can go
    //   here.
    //   // For now we keep motor stopped; clear flag so we don't loop.
    //   motorStoppedByOverCurrent = false;
    // } else
    if (gate->suspendMotorTask) {
      vTaskSuspend(NULL);
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

  // vTaskSuspend(gateMonitorTaskHandle);
  suspendMotorTask = true;
}

void Gate::timerCallback(TimerHandle_t xTimer) { gateHandler(CLOSE); }

void Gate::restartTimerISR(void) {
  // Don't use stdio in ISRs; also only call FreeRTOS FromISR APIs
  // after the scheduler (and timer daemon) are running.
  if (xTaskGetSchedulerState() != taskSCHEDULER_RUNNING ||
      timerStatus != SoftTimerStatus::RUNNING) {
    return;
  }

  if (softTimer == NULL) {
    return;
  }

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xTimerResetFromISR(softTimer, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void Gate::restartTimer(void) {
  if (xTimerIsTimerActive(softTimer) == pdFALSE) {
    xTimerStart(softTimer, 0);
  } else {
    xTimerReset(softTimer, 0);
  }
}

void Gate::handleOpenedState(void) {
  // This function is called by IQR \
  // when the gate is fully opened
  // 1. disable motors
  // 2. sart wait timer
  // 3. set staus waiting
  if (gateStatus == GateStatus::OPENING) {
    return;
  }
  gateStatus = GateStatus::WAITING;
  controlGateMotor(GateAction::IDLE);
  // vTaskSuspend(gateMonitorTaskHandle); // thisable the current mointoring
  suspendMotorTask = true;
  // send udp status
  ethernetInstance->xUDPSendISR("OPENED", 7);
  restartTimerISR();
}

void Gate::handleClosedState(void) {
  // This function is called by IQR \
  // when the gate is fully closed
  // 1. disable motor
  // 2. set status waiting
  if (gateStatus == GateStatus::CLOSING) {
    return;
  }
  gateStatus = GateStatus::WAITING;
  controlGateMotor(GateAction::IDLE);
  // vTaskSuspend(gateMonitorTaskHandle); // thisable the current mointoring
  suspendMotorTask = true;
  restartTimerISR();
  ethernetInstance->xUDPSendISR("CLOSED", 7);
}

GateState Gate::getGateActualState(void) {
  // Reads the state of the gate limit switches to determine the actual
  // position. Assumes active-high logic: pin set means switch released.
  bool closeSwReleased =
      LL_GPIO_IsInputPinSet(CLOSE_SW_GPIO_Port, CLOSE_SW_Pin);
  bool openSwReleased = LL_GPIO_IsInputPinSet(OPEN_SW_GPIO_Port, OPEN_SW_Pin);

  if (closeSwReleased && !openSwReleased) {
    return GateState::CLOSED;
  } else if (!closeSwReleased && openSwReleased) {
    return GateState::OPENED;
  }
  return GateState::UNDEFINED;
}

void Gate::gateHandler(GateAction action) {
  GateState currentState = getGateActualState();
  if (action == GateAction::OPEN) {
    if (currentState != GateState::OPENED) {
      gateStatus = GateStatus::OPENING;
      controlGateMotor(action); // Start motor first, then enable monitoring
      restartTimer();
      vTaskResume(gateMonitorTaskHandle);
    } else {
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
  } else {
    LL_GPIO_ResetOutputPin(MOTOR_DIR_GPIO_Port, MOTOR_DIR_Pin);
    LL_GPIO_ResetOutputPin(MOTOR_DIR_2_GPIO_Port, MOTOR_DIR_2_Pin);
    PWM_SetDutyCycle(0);
  }
}

float Gate::getInstantCurrent(void) { return 11.0f; }

extern "C" {

void handleOpenedState(void) { Gate::getInstance().handleOpenedState(); }

void handleClosedState(void) { Gate::getInstance().handleClosedState(); }
}

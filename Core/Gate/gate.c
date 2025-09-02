#include "gate.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "AppConfig.h"
#include "main.h"
#include "stm32f4xx_ll_gpio.h"

GateAction gateAction = NONE;
GateState gateState = IDLE;

TimerHandle_t gateTimerHandle;

static void gateHandler(GateAction action);

static void timerCallback(TimerHandle_t xTimer) {
    gateState = CLOSED; // start the closing process
    gateHandler(CLOSE);
}

void creteGateTimer(void) {
    gateTimerHandle = xTimerCreate(
        "GateTimer",
        pdMS_TO_TICKS(10000),
        pdFALSE,
        (void*)0,
        timerCallback);
}


void handleGate(GateAction action) {
    switch (action) {
    case OPENED:
    gateState = OPENING;
    gateHandler(OPEN);
    break;
    case CLOSED:
    gateState = CLOSING;
    break;
    case NONE:
    default:
    break;
    }

}





void handleOpenedState(void) {
    // start the timer 
    // after timer is elapsed start closing the gate
    xTimerReset(gateTimerHandle, 0);
    gateState = OPENED;
    gateHandler(NONE); // disable motors
}
void handleClosedState(void) {
    gateState = CLOSING;
    gateHandler(NONE); // disable motors
}

GateState getGateActualState(void) {
    if (LL_GPIO_IsInputPinSet(OPEN_SW_GPIO_Port, OPEN_SW_Pin)) {
        return OPENED;
    }
    else if (LL_GPIO_IsInputPinSet(CLOSE_SW_GPIO_Port, CLOSE_SW_Pin)) {
        return CLOSED;
    }
    return IDLE;
}

GateState getGateState(void) {
    return gateState;
}

static void gateHandler(GateAction action) {
    __NOP();
}


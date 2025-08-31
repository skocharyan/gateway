#ifndef APPCONFIG_H
#define APPCONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#define FLASH_CONFIG_ADDRESS 0x08010000

#define QR_TAKS_STACK_SIZE 512
#define QR_TASK_PRIORITY  (tskIDLE_PRIORITY + 10)

#define QR_WAITING_TIME_MS portMAX_DELAY

#define RX_BUFFER_SIZE 256


    typedef  enum {
        NONE,
        OPEN,
        CLOSE
    } GateAction;

    typedef enum {
        IDLE,
        OPENING,
g    }GateState;

#ifdef __cplusplus
}
#endif

#endif 
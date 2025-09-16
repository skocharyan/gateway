#ifndef APPCONFIG_H
#define APPCONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#define FLASH_CONFIG_ADDRESS 0x08020000

#define QR_TAKS_STACK_SIZE 512
#define QR_TASK_PRIORITY (tskIDLE_PRIORITY + 10)

#define QR_WAITING_TIME_MS portMAX_DELAY

#define RX_BUFFER_SIZE 256

#define QR_WAIT_TIMEOUT 100

#define QR_SUCCESS_RESPONSE "OK"

#define GATE_WAIT_TIMEOUT 10000

#ifdef __cplusplus
}
#endif

#endif
#include "qr.h"
#include "eth.h"
#include "gate.h"
#include "flash.h"
#include "AppConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
#include "stdio.h"

extern EthernetConfig ethConfig;

TaskHandle_t qrTaskHandle = NULL;


#pragma location = ".ccmram"
static StaticTask_t xQrTaskBuffer;
#pragma location=".ccmram"
static StackType_t xQrTaskStack[QR_TAKS_STACK_SIZE];

static void loadEthConfigs(void);
static void qrTask(void* params);
static void createQRTask(void);
void cliInit(void);

int32_t xTCPSendAndReceive(const char* pcTxBuffer,
    size_t txLen,
    char* pcRxBuffer,
    size_t rxBufLen,
    TickType_t recvTimeoutMs);


extern uint8_t qrDataBuffer[RX_BUFFER_SIZE];;


void QR_Init() {
    cliInit();
    loadEthConfigs();
    // ethInit();
    createQRTask();

    vTaskStartScheduler();
}

static void loadEthConfigs() {
    Flash_Read_Data(FLASH_CONFIG_ADDRESS, (uint32_t*)&ethConfig, sizeof(EthernetConfig) / 4);
}



static void createQRTask(void) {
    // xTaskCreate((TaskFunction_t)QR_Task, "QR_Task", QC_TAKS_STACK_SIZE, NULL, QC_TASK_PRIORITY, NULL);
    qrTaskHandle = xTaskCreateStatic((TaskFunction_t)qrTask,
        "QR_Task",
        QR_TAKS_STACK_SIZE,
        NULL,
        QR_TASK_PRIORITY,
        xQrTaskStack,
        &xQrTaskBuffer);
}

static void qrTask(void* params) {
    uint32_t notValue;


    // if (getGateActualState() == IDLE) {
    //     handleGate(CLOSE);  // close idle state;
    // }

    while (1) {

        if (xTaskNotifyWait(0, UINT32_MAX, &notValue, QR_WAITING_TIME_MS) == pdTRUE) {

            // printf("QR data: %s\n", qrDataBuffer);
            // printf("Notified value: %lu\n", notValue);

            // uint8_t qrRespBuffer[RX_BUFFER_SIZE];

            // int32_t xBytesReceived = xTCPSendAndReceive((const char*)qrDataBuffer, (size_t)notValue, (char*)qrRespBuffer, RX_BUFFER_SIZE, QR_WAIT_TIMEOUT);
            // if (xBytesReceived <= 0) {
            //     continue; // failed to verify the QR code
            // }

            // if (strcmp((const char*)qrRespBuffer, QR_SUCCESS_RESPONSE) != 0) {
            //     continue; // invalid response
            // }

            // handleGate(OPEN);

        }
    }
}
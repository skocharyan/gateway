#include "qr.h"
#include "eth.h"
#include "flash.h"
#include "AppConfig.h"
#include "FreeRTOS.h"
#include "task.h"

extern EthernetConfig ethConfig;

TaskHandle_t qrTaskHandle = NULL;


#pragma location = ".ccmram"
static StaticTask_t xQrTaskBuffer;
#pragma location=".ccmram"
static StackType_t xQrTaskStack[QR_TAKS_STACK_SIZE];

static void loadEthConfigs(void);
static void qrTask(void* params);
static void createQRTask(void);


extern uint8_t rxDataBuffer[RX_BUFFER_SIZE];;
extern uint32_t qrLength;

GateAction gateAction = NONE;
GateState gateState = IDLE;

void QR_Init() {
    loadEthConfigs();
    ethInit();
    createQRTask();
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
    while (1) {
        if (xTaskNotifyWait(0, UINT32_MAX, &notValue, QR_WAITING_TIME_MS) == pdTRUE) {
            // verifyThe qr
            // open the gage
        }
    }
}
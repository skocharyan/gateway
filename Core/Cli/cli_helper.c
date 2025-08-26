
#include "stdint.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cli.h"
#include "stdint.h"

static char inputBuffer[CLI_INPUT_BUFFER_SIZE];
static char outputBuffer[CLI_OUTPUT_BUFFER_SIZE];

static uint8_t inputIndex = 0;

extern TaskHandle_t cliTaskHandle;

void CLI_USB_Receive_Char(uint8_t c)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (c == '\r' || c == '\n') {
        inputBuffer[inputIndex] = '\0';
        inputIndex = 0;
        if (cliTaskHandle != NULL) {
            vTaskNotifyGiveFromISR(cliTaskHandle, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        else {
            printf("cliTaskHandle is NULL!\n");
        }
    }
    else if (inputIndex < CLI_INPUT_BUFFER_SIZE - 1) {
        inputBuffer[inputIndex++] = c;
    }
    else {
        inputIndex = 0;
        inputBuffer[inputIndex++] = c;

    }
}

void CDC_ReceiveData(uint8_t* data, const size_t* length)
{
    for (uint32_t i = 0; i < *length; i++) {
        CLI_USB_Receive_Char(data[i]);
    }
}

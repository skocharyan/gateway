#include "FreeRTOS.h"
#include "task.h"
#include "cli.h"
#include "FreeRTOS_CLI.h"
#include "usbd_cdc_if.h"
#include "usbd_def.h"

// commands 
const CLI_Command_Definition_t xSetCommand;

StaticTask_t cliTaskTCB;
StackType_t  cliTaskStack[CLI_TASK_STACK_SIZE];
TaskHandle_t cliTaskHandle = NULL;

char inputBuffer[CLI_INPUT_BUFFER_SIZE];
char outputBuffer[CLI_OUTPUT_BUFFER_SIZE];
// Private functions
static void vCliTask(void* pvParameters);
//EOF private functions


// Command handlers
BaseType_t prvSetCommand(char* pcWriteBuffer, size_t xWriteBufferLen, const char* pcCommandString);
BaseType_t prvGetCommand(char* pcWriteBuffer, size_t xWriteBufferLen, const char* pcCommandString);

// EOF command handlers


// Command definition
const CLI_Command_Definition_t xSetCommand = {
    .pcCommand = "set",
    .pcHelpString = "set property <value>: Set the ip/port/mas .... values\r\n",
    .pxCommandInterpreter = prvSetCommand,
    .cExpectedNumberOfParameters = 2
};

const CLI_Command_Definition_t xGetCommand = {
    .pcCommand = "get",
    .pcHelpString =
        "get [all|field]: Get one field or ‘all’.  Fields:\r\n"
        "   id, ip, port, mask, gateway, dns, max_rpm, pulses, start_volume, stop_volume\r\n",
    .pxCommandInterpreter = prvGetCommand,
    .cExpectedNumberOfParameters = -1   // allow zero or one parameter
};

// EOF command definitions


void cliInit(void) {
    cliTaskHandle = xTaskCreateStatic(
        vCliTask,                     // Task function
        CLI_TASK_NAME,                // Task name
        CLI_TASK_STACK_SIZE,          // Stack size
        NULL,                          // Task parameters
        CLI_TASK_PRIORITY,            // Task priority
        cliTaskStack,                 // Stack buffer
        &cliTaskTCB                   // TCB buffer
    );
    // Register CLI commands
    FreeRTOS_CLIRegisterCommand(&xSetCommand);
    FreeRTOS_CLIRegisterCommand(&xGetCommand);
}


void vCliTask(void* pvParameters)
{
    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        BaseType_t more;

        do {
            memset(outputBuffer, 0, sizeof(outputBuffer));
            more = FreeRTOS_CLIProcessCommand(inputBuffer, outputBuffer, sizeof(outputBuffer));
            size_t outLen = strlen(outputBuffer);

            if (outLen > 0) {
                // block until CDC is ready
                while (CDC_Transmit_FS((uint8_t*)outputBuffer, outLen) == USBD_BUSY) {
                    vTaskDelay(pdMS_TO_TICKS(5));
                }
            }
        } while (more != pdFALSE);

        // send prompt
        while (CDC_Transmit_FS((uint8_t*)"> ", 1) == USBD_BUSY) {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }

}
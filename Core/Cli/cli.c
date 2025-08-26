#include "FreeRTOS.h"
#include "task.h"
#include "cli.h"
#include "FreeRTOS_CLI.h"

// commands 
const CLI_Command_Definition_t xSetCommand;

StaticTask_t cliTaskTCB;
StackType_t  cliTaskStack[CLI_TASK_STACK_SIZE];
TaskHandle_t cliTaskHandle = NULL;

// Private functions
static void vCliTask(void* pvParameters);
//EOF private functions


// Command handlers
BaseType_t prvSetCommand(char* pcWriteBuffer, size_t xWriteBufferLen, const char* pcCommandString);
// EOF command handlers


// Command definition
const CLI_Command_Definition_t xSetCommand = {
    .pcCommand = "set",
    .pcHelpString = "set property <value>: Set the ip/port/mas .... values\r\n",
    .pxCommandInterpreter = prvSetCommand,
    .cExpectedNumberOfParameters = 2
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
}

static void vCliTask(void* pvParameters) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100)); // Avoid from blocking
    }
}

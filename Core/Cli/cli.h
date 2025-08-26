#ifndef CLI_H
#define CLI_H

#ifdef __cplusplus
extern "C" {
#endif
#include "FreeRTOS.h"

#define CLI_INPUT_BUFFER_SIZE 256
#define CLI_OUTPUT_BUFFER_SIZE 512


#define CLI_TASK_STACK_SIZE 384
#define CLI_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define CLI_TASK_NAME "CLI_Task"

#ifdef __cplusplus
}
#endif

#endif // CLI_H
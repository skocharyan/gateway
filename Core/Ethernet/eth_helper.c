#include "FreeRTOS.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "stdint.h"
#include "stdio.h"
#include "stm32f4xx_hal.h"
#include "string.h"
#include "task.h"

uint8_t ip_address[6] = {192, 168, 0, 122};
uint16_t port = 1500;

void generate_mac_address(uint8_t *device_id, uint8_t *mac_buffer) {
  // Set the first octet to 0x02 to indicate a locally administered MAC address
  mac_buffer[0] = 0x02;

  // Generate the remaining 5 octets by XOR-ing groups of the device_id bytes.
  // This loop ensures each of the five octets gets contributions from the 16
  // bytes.
  for (int i = 0; i < 5; i++) {
    uint8_t xor_val = 0;
    for (int j = i; j < 16; j += 5) {
      xor_val ^= device_id[j];
    }
    mac_buffer[i + 1] = xor_val;
  }
}

BaseType_t xApplicationGetRandomNumber(uint32_t *pulNumber) {

  *pulNumber = HAL_GetTick(); // Use the system tick as a random number source
  return pdTRUE;
}

uint32_t ulApplicationGetNextSequenceNumber(uint32_t ulSourceAddress,
                                            uint16_t usSourcePort,
                                            uint32_t ulDestinationAddress,
                                            uint16_t usDestinationPort) {
  uint32_t ticks = xTaskGetTickCount();

  // Simple pseudo-random generation
  uint32_t value = ulSourceAddress ^ ulDestinationAddress;
  value ^= ((uint32_t)usSourcePort << 16) | usDestinationPort;
  value ^= ticks * 1103515245UL + 12345UL; // LCG

  return value;
}

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize) {
  static StaticTask_t xIdleTaskTCB;
  static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
  *ppxIdleTaskStackBuffer = xIdleStack;
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize) {
  static StaticTask_t xTimerTaskTCB;
  static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

  *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
  *ppxTimerTaskStackBuffer = xTimerStack;
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
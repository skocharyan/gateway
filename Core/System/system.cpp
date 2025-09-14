#include "system_thread.hpp"

#include "AppConfig.h"
#include "FreeRTOS.h"
#include "eth.h"
#include "flash.h"
#include "gate.hpp"
#include "stdio.h"
#include "string.h"
#include "system_uart.hpp"
#include "task.h"

extern EthernetConfig ethConfig;

static void loadEthConfigs(void);
extern "C" void cliInit(void);

SystemUart *systemUartInstance = nullptr;
Gate *gateInstance = nullptr;

extern "C" void System_Init() {

  cliInit();
  loadEthConfigs();
  // ethInit();
  SystemThread systemThread; // The main worker task

  SystemUart systemUart(systemThread.getTaskHandle());

  systemUartInstance = &systemUart;

  Gate gate; // Gate control instance
  gateInstance = &gate;

  vTaskStartScheduler();
}

static void loadEthConfigs() {
  Flash_Read_Data(FLASH_CONFIG_ADDRESS, (uint32_t *)&ethConfig,
                  sizeof(EthernetConfig) / 4);
}

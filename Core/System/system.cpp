#include "system_thread.hpp"

#include "AppConfig.h"
#include "FreeRTOS.h"
#include "eth.hpp"
#include "flash.h"
#include "gate.hpp"
#include "stdio.h"
#include "string.h"
#include "system_uart.hpp"
#include "task.h"

extern Config_t ethConfig;

static void loadEthConfigs(Config_t &ethConfig) {
  Flash_Read_Data(FLASH_CONFIG_ADDRESS, (uint32_t *)&ethConfig,
                  sizeof(Config_t) / 4);

  printf("Port numner from flash: %d\n", ethConfig.portNumber);
}

extern "C" void cliInit(void);

SystemUart *systemUartInstance = nullptr;
Ethernet *ethernetInstance = nullptr;
SystemThread *systemThreadInstance = nullptr;
Config_t ethConfig;

extern "C" void System_Init() {

  cliInit();

  loadEthConfigs(ethConfig);

  static Ethernet ethernet(ethConfig);
  // ethInit();
  static SystemThread systemThread; // The main worker task

  systemThreadInstance = &systemThread;
  static SystemUart systemUart(systemThread.getTaskHandle(),
                               systemThread.getDataBufferRef());

  systemUartInstance = &systemUart;

  static Gate &gate = Gate::getInstance();

  ethernetInstance = &ethernet;

  vTaskStartScheduler();
}

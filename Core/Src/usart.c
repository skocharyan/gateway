/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#include "stdint.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "AppConfig.h"

  /* USER CODE END Header */
  /* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
uint8_t rxDataBuffer[RX_BUFFER_SIZE] = { 0 };
uint32_t qrLength = 0;

extern TaskHandle_t qrTaskHandle;

/* USER CODE END 0 */

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  LL_USART_InitTypeDef USART_InitStruct = { 0 };

  LL_GPIO_InitTypeDef GPIO_InitStruct = { 0 };

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**USART1 GPIO Configuration
  PA9   ------> USART1_TX
  PA10   ------> USART1_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_9 | LL_GPIO_PIN_10;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USART1 DMA Init */

  /* USART1_RX Init */
  LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_2, LL_DMA_CHANNEL_4);

  LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_2, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

  LL_DMA_SetStreamPriorityLevel(DMA2, LL_DMA_STREAM_2, LL_DMA_PRIORITY_LOW);

  LL_DMA_SetMode(DMA2, LL_DMA_STREAM_2, LL_DMA_MODE_NORMAL);

  LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_STREAM_2, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_STREAM_2, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA2, LL_DMA_STREAM_2, LL_DMA_PDATAALIGN_BYTE);

  LL_DMA_SetMemorySize(DMA2, LL_DMA_STREAM_2, LL_DMA_MDATAALIGN_BYTE);

  LL_DMA_DisableFifoMode(DMA2, LL_DMA_STREAM_2);

  /* USART1 interrupt Init */
  NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 7, 0));
  NVIC_EnableIRQ(USART1_IRQn);

  /* USER CODE BEGIN USART1_Init 1 */
  LL_USART_EnableIT_IDLE(USART1);
  /* USER CODE END USART1_Init 1 */
  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART1, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(USART1);
  LL_USART_Enable(USART1);
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/* USER CODE BEGIN 1 */

void U1_Init(void) {

  // // DMA RX Initialization
  LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_2);
  LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_2,
    LL_USART_DMA_GetRegAddr(USART1));

  LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_2, (uint32_t)rxDataBuffer);
  LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_2,
    LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

  LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_2, RX_BUFFER_SIZE);
  // LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_2);

  LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_2);
  // LL_USART_EnableDMAReq_RX(USART1);


}


void USART_Process(void) {
  // Handle USART1 IDLE line detection event
  if (LL_USART_IsActiveFlag_IDLE(USART1)) {
    LL_USART_ClearFlag_IDLE(USART1);
    // Add your code to handle the IDLE line detection event here

    qrLength = RX_BUFFER_SIZE - LL_DMA_GetDataLength(DMA2, LL_DMA_STREAM_2);

    // Process received data in rxDataBuffer[0..dataLength-1] here
    // Example: user_callback(rxDataBuffer, dataLength);

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR(qrTaskHandle, 0, eNoAction, NULL);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);


    // Reset DMA for next reception
    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_2);
    LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_2, (uint32_t)rxDataBuffer);
    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_2, RX_BUFFER_SIZE);
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_2);


    // Optionally clear buffer if needed
    // memset(rxDataBuffer, 0, RX_BUFFER_SIZE);
  }
}

/* USER CODE END 1 */

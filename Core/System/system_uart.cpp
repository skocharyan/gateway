#include "system_uart.hpp"
#include "AppConfig.h"
#include "FreeRTOS.h"
#include "main.h"
#include "stdio.h"
#include "string.h"

extern SystemUart *systemUartInstance;

SystemUart::SystemUart(TaskHandle_t qrTaskHandle,
                       uint8_t (&ref)[RX_BUFFER_SIZE])
    : msgRef{ref} {

  printf("SystemUart constructor called \n");

  this->qrTaskHandle = qrTaskHandle;

  LL_USART_InitTypeDef USART_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
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
  LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_2, LL_DMA_CHANNEL_4);
  LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_2,
                                  LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

  LL_DMA_SetStreamPriorityLevel(DMA2, LL_DMA_STREAM_2, LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(DMA2, LL_DMA_STREAM_2, LL_DMA_MODE_CIRCULAR);
  LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_STREAM_2, LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_STREAM_2, LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(DMA2, LL_DMA_STREAM_2, LL_DMA_PDATAALIGN_BYTE);
  LL_DMA_SetMemorySize(DMA2, LL_DMA_STREAM_2, LL_DMA_MDATAALIGN_BYTE);
  LL_DMA_DisableFifoMode(DMA2, LL_DMA_STREAM_2);

  NVIC_SetPriority(USART1_IRQn,
                   NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 7, 0));
  NVIC_EnableIRQ(USART1_IRQn);

  LL_USART_EnableIT_IDLE(USART1); // Enable IDLE line detection interrupt

  USART_InitStruct.BaudRate = 9600;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART1, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(USART1);
  LL_USART_Enable(USART1);

  LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_2);
  LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_2,
                          LL_USART_DMA_GetRegAddr(USART1));
  LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_2, (uint32_t)dmaBuffer);
  LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_2, RX_BUFFER_SIZE);
  LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_2);
  LL_USART_EnableDMAReq_RX(USART1);

  qrLastPos = 0; // start from beginning
}

void SystemUart::process(void) {
  if (LL_USART_IsActiveFlag_IDLE(USART1)) {
    // Clear IDLE flag
    LL_USART_ClearFlag_IDLE(USART1);

    uint8_t dmaProcessBuffer[RX_BUFFER_SIZE]{0};

    // How many bytes DMA has written
    uint32_t dma_remaining = LL_DMA_GetDataLength(DMA2, LL_DMA_STREAM_2);
    uint32_t dma_pos = RX_BUFFER_SIZE - dma_remaining; // current write position

    if (dma_pos != qrLastPos) {
      uint32_t newDataLen = 0;

      if (dma_pos > qrLastPos) {
        // Linear case: data is contiguous
        newDataLen = dma_pos - qrLastPos;
        memcpy(dmaProcessBuffer, &dmaBuffer[qrLastPos], newDataLen);
      } else {
        uint32_t tailLen = RX_BUFFER_SIZE - qrLastPos;
        uint32_t headLen = dma_pos;
        memcpy(dmaProcessBuffer, &dmaBuffer[qrLastPos], tailLen);
        if (headLen > 0) {
          memcpy(dmaProcessBuffer + tailLen, dmaBuffer, headLen);
        }
        newDataLen = tailLen + headLen;
      }

      // Notify QR task that new data is available

      memcpy(msgRef, dmaProcessBuffer, newDataLen);

      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      xTaskNotifyFromISR(qrTaskHandle, newDataLen, eSetValueWithOverwrite,
                         &xHigherPriorityTaskWoken);
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

      qrLastPos = dma_pos;
    }
  }
}

extern "C" void USART1_IRQHandler_cpp(void) { systemUartInstance->process(); }

void USART1_IRQHandler(void) {
  extern void USART1_IRQHandler_cpp(void);
  USART1_IRQHandler_cpp();
}
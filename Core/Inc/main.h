/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_exti.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_cortex.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_dma.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define K2_BTN_Pin LL_GPIO_PIN_2
#define K2_BTN_GPIO_Port GPIOE
#define K1_BTN_Pin LL_GPIO_PIN_3
#define K1_BTN_GPIO_Port GPIOE
#define NOZ_SWITCH_Pin LL_GPIO_PIN_4
#define NOZ_SWITCH_GPIO_Port GPIOE
#define LED2_Pin LL_GPIO_PIN_13
#define LED2_GPIO_Port GPIOC
#define PC14_OSC32_IN_Pin LL_GPIO_PIN_14
#define PC14_OSC32_IN_GPIO_Port GPIOC
#define PC15_OSC32_OUT_Pin LL_GPIO_PIN_15
#define PC15_OSC32_OUT_GPIO_Port GPIOC
#define PH0_OSC_IN_Pin LL_GPIO_PIN_0
#define PH0_OSC_IN_GPIO_Port GPIOH
#define PH1_OSC_OUT_Pin LL_GPIO_PIN_1
#define PH1_OSC_OUT_GPIO_Port GPIOH
#define KEY_COL2_Pin LL_GPIO_PIN_2
#define KEY_COL2_GPIO_Port GPIOC
#define KEY_COL2_EXTI_IRQn EXTI2_IRQn
#define LCD_MOSI_Pin LL_GPIO_PIN_3
#define LCD_MOSI_GPIO_Port GPIOC
#define WKUP_Pin LL_GPIO_PIN_0
#define WKUP_GPIO_Port GPIOA
#define FRAM_SCK_Pin LL_GPIO_PIN_5
#define FRAM_SCK_GPIO_Port GPIOA
#define FRAM_MISO_Pin LL_GPIO_PIN_6
#define FRAM_MISO_GPIO_Port GPIOA
#define LED1_Pin LL_GPIO_PIN_1
#define LED1_GPIO_Port GPIOB
#define BOOT1_Pin LL_GPIO_PIN_2
#define BOOT1_GPIO_Port GPIOB
#define R1_V_HIGH_Pin LL_GPIO_PIN_8
#define R1_V_HIGH_GPIO_Port GPIOE
#define R1_V_LOW_Pin LL_GPIO_PIN_9
#define R1_V_LOW_GPIO_Port GPIOE
#define R1_PUMP_EN_Pin LL_GPIO_PIN_10
#define R1_PUMP_EN_GPIO_Port GPIOE
#define LCD_RSE_Pin LL_GPIO_PIN_13
#define LCD_RSE_GPIO_Port GPIOE
#define LCD_RS_Pin LL_GPIO_PIN_14
#define LCD_RS_GPIO_Port GPIOE
#define LCD_CS_Pin LL_GPIO_PIN_15
#define LCD_CS_GPIO_Port GPIOE
#define LCD_SCK_Pin LL_GPIO_PIN_10
#define LCD_SCK_GPIO_Port GPIOB
#define OTG_FS_DM_Pin LL_GPIO_PIN_11
#define OTG_FS_DM_GPIO_Port GPIOA
#define OTG_FS_DP_Pin LL_GPIO_PIN_12
#define OTG_FS_DP_GPIO_Port GPIOA
#define SWDIO_Pin LL_GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin LL_GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define KEY_ROW1_Pin LL_GPIO_PIN_15
#define KEY_ROW1_GPIO_Port GPIOA
#define SRCLK_Pin LL_GPIO_PIN_10
#define SRCLK_GPIO_Port GPIOC
#define LATCH_Pin LL_GPIO_PIN_11
#define LATCH_GPIO_Port GPIOC
#define DATA_Pin LL_GPIO_PIN_12
#define DATA_GPIO_Port GPIOC
#define KEY_ROW5_Pin LL_GPIO_PIN_0
#define KEY_ROW5_GPIO_Port GPIOD
#define KEY_COL1_Pin LL_GPIO_PIN_1
#define KEY_COL1_GPIO_Port GPIOD
#define KEY_COL1_EXTI_IRQn EXTI1_IRQn
#define KEY_COL3_Pin LL_GPIO_PIN_3
#define KEY_COL3_GPIO_Port GPIOD
#define KEY_COL3_EXTI_IRQn EXTI3_IRQn
#define KEY_COL4_Pin LL_GPIO_PIN_4
#define KEY_COL4_GPIO_Port GPIOD
#define KEY_COL4_EXTI_IRQn EXTI4_IRQn
#define OTG_FS_OverCurrent_Pin LL_GPIO_PIN_5
#define OTG_FS_OverCurrent_GPIO_Port GPIOD
#define KEY_ROW3_Pin LL_GPIO_PIN_6
#define KEY_ROW3_GPIO_Port GPIOD
#define KEY_ROW4_Pin LL_GPIO_PIN_7
#define KEY_ROW4_GPIO_Port GPIOD
#define SWO_Pin LL_GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define FRAM_CS_Pin LL_GPIO_PIN_4
#define FRAM_CS_GPIO_Port GPIOB
#define FRAM_MOSI_Pin LL_GPIO_PIN_5
#define FRAM_MOSI_GPIO_Port GPIOB
#define FRAM_WP_Pin LL_GPIO_PIN_6
#define FRAM_WP_GPIO_Port GPIOB
#define KEY_ROW2_Pin LL_GPIO_PIN_8
#define KEY_ROW2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

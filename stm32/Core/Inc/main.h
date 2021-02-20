/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#define EP_CLK_Pin GPIO_PIN_5
#define EP_CLK_GPIO_Port GPIOA
#define EP_MOSI_Pin GPIO_PIN_7
#define EP_MOSI_GPIO_Port GPIOA
#define EP_DC_Pin GPIO_PIN_4
#define EP_DC_GPIO_Port GPIOC
#define EP_CS_Pin GPIO_PIN_5
#define EP_CS_GPIO_Port GPIOC
#define EP_RST_Pin GPIO_PIN_0
#define EP_RST_GPIO_Port GPIOB
#define EP_BUSY_Pin GPIO_PIN_1
#define EP_BUSY_GPIO_Port GPIOB
#define UART3_TX_Pin GPIO_PIN_10
#define UART3_TX_GPIO_Port GPIOB
#define UART3_RX_Pin GPIO_PIN_11
#define UART3_RX_GPIO_Port GPIOB
#define PWR_3V3_EN_Pin GPIO_PIN_12
#define PWR_3V3_EN_GPIO_Port GPIOB
#define PWR_SD_EN_Pin GPIO_PIN_15
#define PWR_SD_EN_GPIO_Port GPIOA
#define SD_DET_Pin GPIO_PIN_4
#define SD_DET_GPIO_Port GPIOB
#define LDR_SIG_Pin GPIO_PIN_8
#define LDR_SIG_GPIO_Port GPIOB
#define LDR_GND_Pin GPIO_PIN_9
#define LDR_GND_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

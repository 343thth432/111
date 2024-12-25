/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
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
uint8_t NAND_ReadData(void);
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define FLASH_CLE_Pin GPIO_PIN_5
#define FLASH_CLE_GPIO_Port GPIOA
#define FLASH_ALE_Pin GPIO_PIN_6
#define FLASH_ALE_GPIO_Port GPIOA
#define FLASH_RB_Pin GPIO_PIN_7
#define FLASH_RB_GPIO_Port GPIOA
#define D0_Pin GPIO_PIN_0
#define D0_GPIO_Port GPIOB
#define D1_Pin GPIO_PIN_1
#define D1_GPIO_Port GPIOB
#define D2_Pin GPIO_PIN_2
#define D2_GPIO_Port GPIOB
#define W5500_CS_Pin GPIO_PIN_6
#define W5500_CS_GPIO_Port GPIOC
#define W5500_RST_Pin GPIO_PIN_7
#define W5500_RST_GPIO_Port GPIOC
#define FLASH_RE_Pin GPIO_PIN_8
#define FLASH_RE_GPIO_Port GPIOA
#define FLASH_WE_Pin GPIO_PIN_9
#define FLASH_WE_GPIO_Port GPIOA
#define FLASH_CE_Pin GPIO_PIN_10
#define FLASH_CE_GPIO_Port GPIOA
#define D3_Pin GPIO_PIN_3
#define D3_GPIO_Port GPIOB
#define D4_Pin GPIO_PIN_4
#define D4_GPIO_Port GPIOB
#define D5_Pin GPIO_PIN_5
#define D5_GPIO_Port GPIOB
#define D6_Pin GPIO_PIN_6
#define D6_GPIO_Port GPIOB
#define D7_Pin GPIO_PIN_7
#define D7_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

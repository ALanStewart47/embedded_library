/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define sel3_Pin GPIO_PIN_13
#define sel3_GPIO_Port GPIOC
#define sel2_Pin GPIO_PIN_14
#define sel2_GPIO_Port GPIOC
#define Select1_Pin GPIO_PIN_15
#define Select1_GPIO_Port GPIOC
#define Select2_Pin GPIO_PIN_0
#define Select2_GPIO_Port GPIOD
#define HorL_Pin GPIO_PIN_1
#define HorL_GPIO_Port GPIOD
#define LED3_Pin GPIO_PIN_0
#define LED3_GPIO_Port GPIOC
#define S_L_R_Pin GPIO_PIN_1
#define S_L_R_GPIO_Port GPIOC
#define S_MODE_Pin GPIO_PIN_2
#define S_MODE_GPIO_Port GPIOC
#define OCP_RESET_Pin GPIO_PIN_3
#define OCP_RESET_GPIO_Port GPIOC
#define PWM4_Pin GPIO_PIN_0
#define PWM4_GPIO_Port GPIOA
#define Trigger4_Pin GPIO_PIN_1
#define Trigger4_GPIO_Port GPIOA
#define W_INTn_Pin GPIO_PIN_4
#define W_INTn_GPIO_Port GPIOA
#define W_SCLK_Pin GPIO_PIN_5
#define W_SCLK_GPIO_Port GPIOA
#define W_MISO_Pin GPIO_PIN_6
#define W_MISO_GPIO_Port GPIOA
#define W_MOSI_Pin GPIO_PIN_7
#define W_MOSI_GPIO_Port GPIOA
#define W_SCSn_Pin GPIO_PIN_4
#define W_SCSn_GPIO_Port GPIOC
#define W_RSTn_Pin GPIO_PIN_5
#define W_RSTn_GPIO_Port GPIOC
#define PowerDown_Pin GPIO_PIN_0
#define PowerDown_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_1
#define LED1_GPIO_Port GPIOB
#define DPorSP_Pin GPIO_PIN_2
#define DPorSP_GPIO_Port GPIOB
#define URAT_232_TX_Pin GPIO_PIN_10
#define URAT_232_TX_GPIO_Port GPIOB
#define UART_232_RX_Pin GPIO_PIN_11
#define UART_232_RX_GPIO_Port GPIOB
#define Key1_Pin GPIO_PIN_12
#define Key1_GPIO_Port GPIOB
#define Key2_Pin GPIO_PIN_13
#define Key2_GPIO_Port GPIOB
#define PWM6_Pin GPIO_PIN_14
#define PWM6_GPIO_Port GPIOB
#define Trigger6_Pin GPIO_PIN_15
#define Trigger6_GPIO_Port GPIOB
#define PWM7_Pin GPIO_PIN_6
#define PWM7_GPIO_Port GPIOC
#define Trigger7_Pin GPIO_PIN_7
#define Trigger7_GPIO_Port GPIOC
#define Key3_Pin GPIO_PIN_8
#define Key3_GPIO_Port GPIOC
#define LED2_Pin GPIO_PIN_9
#define LED2_GPIO_Port GPIOC
#define PWM8_Pin GPIO_PIN_8
#define PWM8_GPIO_Port GPIOA
#define ST_Pin GPIO_PIN_11
#define ST_GPIO_Port GPIOA
#define DA_Pin GPIO_PIN_12
#define DA_GPIO_Port GPIOA
#define PWM3_Pin GPIO_PIN_15
#define PWM3_GPIO_Port GPIOA
#define UART_LAN_TX_Pin GPIO_PIN_10
#define UART_LAN_TX_GPIO_Port GPIOC
#define UART_LAN_RX_Pin GPIO_PIN_11
#define UART_LAN_RX_GPIO_Port GPIOC
#define sel4_Pin GPIO_PIN_12
#define sel4_GPIO_Port GPIOC
#define sel1_Pin GPIO_PIN_2
#define sel1_GPIO_Port GPIOD
#define Trigger3_Pin GPIO_PIN_3
#define Trigger3_GPIO_Port GPIOB
#define PWM2_Pin GPIO_PIN_4
#define PWM2_GPIO_Port GPIOB
#define Trigger2_Pin GPIO_PIN_5
#define Trigger2_GPIO_Port GPIOB
#define OCP_Check_Pin GPIO_PIN_9
#define OCP_Check_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

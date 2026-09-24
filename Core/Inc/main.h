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
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

    /* Private includes ----------------------------------------------------------*/
    /* USER CODE BEGIN Includes */

    /* USER CODE END Includes */

    /* Exported types ------------------------------------------------------------*/
    /* USER CODE BEGIN ET */
    typedef enum
    {
        _24_HOUR_MODE = 0,
        _12_HOUR_MODE = 1,
        _NONE = 2
    } TimeFormat_t;

    extern TimeFormat_t time_format; // Global variable to hold the current time format

    /* USER CODE END ET */

    /* Exported constants --------------------------------------------------------*/
    /* USER CODE BEGIN EC */

    /* USER CODE END EC */

    /* Exported macro ------------------------------------------------------------*/
    /* USER CODE BEGIN EM */

    // lcd pins
#define RS_PIN GPIO_PIN_0
#define RS_PORT GPIOA

#define ENABLE_PIN GPIO_PIN_1
#define ENABLE_PORT GPIOA

#define D4_PIN GPIO_PIN_2
#define D4_PORT GPIOA

#define D5_PIN GPIO_PIN_3
#define D5_PORT GPIOA

#define D6_PIN GPIO_PIN_4
#define D6_PORT GPIOA

#define D7_PIN GPIO_PIN_5
#define D7_PORT GPIOA

#define BACKLIGHT_PIN GPIO_PIN_9
#define BACKLIGHT_PORT GPIOB

    // buttons
    // #define UP_Pin GPIO_PIN_6
    // #define UP_GPIO_Port GPIOB

    // #define DOWN_Pin GPIO_PIN_8
    // #define DOWN_GPIO_Port GPIOB

    // #define ENTER_Pin GPIO_PIN_7
    // #define ENTER_GPIO_Port GPIOB

    // #define BACK_Pin GPIO_PIN_5
    // #define BACK_GPIO_Port GPIOB

#define UP_Pin GPIO_PIN_5
#define UP_GPIO_Port GPIOB

#define DOWN_Pin GPIO_PIN_7
#define DOWN_GPIO_Port GPIOB

#define ENTER_Pin GPIO_PIN_8
#define ENTER_GPIO_Port GPIOB

#define BACK_Pin GPIO_PIN_6
#define BACK_GPIO_Port GPIOB

    /* USER CODE END EM */

    /* Exported functions prototypes ---------------------------------------------*/
    void Error_Handler(void);

    /* USER CODE BEGIN EFP */

    /* USER CODE END EFP */

    /* Private defines -----------------------------------------------------------*/

    /* USER CODE BEGIN Private defines */

    void set_time_format(TimeFormat_t format);
    TimeFormat_t get_time_format(void);

    /* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "inputs.h"
#include "ui.h"

#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

/* USER CODE BEGIN PV */

Lcd_HandleTypeDef lcd;
TimeFormat_t time_format = _NONE; // Default to 24-hour mode

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void Enter_Stop_Mode_Until_Next_Minute(void)
{
  RTC_TimeTypeDef time;
  HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);

  uint32_t seconds_to_next_minute = (time.Seconds == 0) ? 60 : (60 - time.Seconds);
  uint32_t reload = seconds_to_next_minute - 1;

  HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
  HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, reload, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);

  // Only ENTER is allowed to wake the MCU from STOP mode.
  EXTI->IMR &= ~(UP_Pin | DOWN_Pin | BACK_Pin);
  __HAL_GPIO_EXTI_CLEAR_IT(UP_Pin);
  __HAL_GPIO_EXTI_CLEAR_IT(DOWN_Pin);
  __HAL_GPIO_EXTI_CLEAR_IT(BACK_Pin);
  __HAL_GPIO_EXTI_CLEAR_IT(ENTER_Pin);

  HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);

  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

  HAL_SuspendTick();
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

  // Woke up — restore the system clock and button interrupts.
  HAL_ResumeTick();
  SystemClock_Config();
  EXTI->IMR |= UP_Pin | DOWN_Pin | BACK_Pin;

  HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_RTC_Init();
  HAL_NVIC_SetPriority(RTC_WKUP_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);
  /* USER CODE BEGIN 2 */

  static Lcd_PortType ports[] = {D4_PORT, D5_PORT, D6_PORT, D7_PORT};
  static Lcd_PinType pins[] = {D4_PIN, D5_PIN, D6_PIN, D7_PIN};
  lcd = Lcd_create(ports, pins, RS_PORT, RS_PIN, ENABLE_PORT, ENABLE_PIN, LCD_4_BIT_MODE);

  // Display Initialized
  Lcd_init(&lcd);

  // UI Initialized
  ui_init(&lcd, &hrtc);

  // Initialize inputs
  inputs_init();

  // The UI enables the backlight only while a menu screen is active.
  HAL_GPIO_WritePin(BACKLIGHT_PORT, BACKLIGHT_PIN, 0);

  // get the time format from backup register
  time_format = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1);

  // Validate the time format value
  if (time_format != _12_HOUR_MODE && time_format != _24_HOUR_MODE)
  {
    time_format = _24_HOUR_MODE; // Default to 24-hour mode if invalid value
  }

  set_time_format(time_format); // Apply the time format to the RTC

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    ui_task();

    if (!ui_is_home())
    {
      continue;
    }

    // Sleep until next minute while the home screen is displayed.
    Enter_Stop_Mode_Until_Next_Minute();

    // Give a held ENTER button time to become a long press after wake-up.
    if (HAL_GPIO_ReadPin(ENTER_GPIO_Port, ENTER_Pin) == GPIO_PIN_RESET)
    {
      HAL_Delay(800);
    }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief RTC Initialization Function
 * @param None
 * @retval None
 */
static void MX_RTC_Init(void)
{
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /*
   * Check whether RTC has already been initialized.
   */
  if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != 0x1234)
  {
    /*
     * RTC has never been initialized.
     * Set initial date/time only once.
     */

    sTime.Hours = 12;
    sTime.Minutes = 12;
    sTime.Seconds = 12;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
    {
      Error_Handler();
    }

    sDate.WeekDay = RTC_WEEKDAY_SATURDAY;
    sDate.Month = RTC_MONTH_JUNE;
    sDate.Date = 7;
    sDate.Year = 25;

    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
    {
      Error_Handler();
    }

    /*
     * Mark RTC as initialized.
     */
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0x1234);
  }

  /*
   * DO NOT call SetTime/SetDate here when backup marker is valid.
   */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Pin = BACKLIGHT_PIN;

  // init he backlight pin
  HAL_GPIO_Init(BACKLIGHT_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(BACKLIGHT_PORT, BACKLIGHT_PIN, 1);

  /* Initalize all display pins as output */
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  GPIO_InitStruct.Pin = D4_PIN;
  HAL_GPIO_Init(D4_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = D5_PIN;
  HAL_GPIO_Init(D5_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = D6_PIN;
  HAL_GPIO_Init(D6_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = D7_PIN;
  HAL_GPIO_Init(D7_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = RS_PIN;
  HAL_GPIO_Init(RS_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = ENABLE_PIN;
  HAL_GPIO_Init(ENABLE_PORT, &GPIO_InitStruct);

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void set_time_format(TimeFormat_t format)
{
  // Write the time format to the backup register so it persists across resets
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, format); // Store the time format in backup register

  // change the time format and update the RTC hour format accordingly
  time_format = format;
  if (format == _24_HOUR_MODE)
  {
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  }
  else
  {
    hrtc.Init.HourFormat = RTC_HOURFORMAT_12;
  }

  // re-initialize the RTC with the new hour format
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
}

TimeFormat_t get_time_format(void)
{
  return time_format;
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

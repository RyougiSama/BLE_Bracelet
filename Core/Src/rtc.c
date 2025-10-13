/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   This file provides code for the configuration
  *          of the RTC instances.
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
#include "rtc.h"

/* USER CODE BEGIN 0 */
void read_bkup(RTC_HandleTypeDef* hrtc)
{
  if (HAL_RTCEx_BKUPRead(hrtc, RTC_BKP_DR1) == 0) {
      // 判断寄存器是否存储日期，相当于标志位
      return;
  } else {
      // 依次将年月日从备份寄存器读出
      hrtc->DateToUpdate.Year = HAL_RTCEx_BKUPRead(hrtc, RTC_BKP_DR2);
      hrtc->DateToUpdate.Month = HAL_RTCEx_BKUPRead(hrtc, RTC_BKP_DR3);
      hrtc->DateToUpdate.Date = HAL_RTCEx_BKUPRead(hrtc, RTC_BKP_DR4);
  }
}

void write_bkup(RTC_HandleTypeDef* hrtc)
{
  HAL_RTCEx_BKUPWrite(hrtc, RTC_BKP_DR1, 1);  // 每次写入置标志位
  // 依次写入年月日到备份寄存器
  HAL_RTCEx_BKUPWrite(hrtc, RTC_BKP_DR2, hrtc->DateToUpdate.Year);
  HAL_RTCEx_BKUPWrite(hrtc, RTC_BKP_DR3, hrtc->DateToUpdate.Month);
  HAL_RTCEx_BKUPWrite(hrtc, RTC_BKP_DR4, hrtc->DateToUpdate.Date);

}
/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */
  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};

  /* USER CODE BEGIN RTC_Init 1 */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
  if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) == 1) {
      return;
  }
  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x9;
  sTime.Minutes = 0x12;
  sTime.Seconds = 0x0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_OCTOBER;
  DateToUpdate.Date = 0x13;
  DateToUpdate.Year = 0x25;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */
  /* USER CODE END RTC_Init 2 */

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */
    HAL_PWR_EnableBkUpAccess();
    /* Enable BKP CLK enable for backup registers */
    __HAL_RCC_BKP_CLK_ENABLE();
    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

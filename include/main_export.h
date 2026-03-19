/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main_export.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
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
#ifndef MAIN_EXPORT_H
#define MAIN_EXPORT_H

#ifdef __cplusplus
extern "C"
{
#endif

  /* Includes ------------------------------------------------------------------*/
#include <stdint.h>

  /* Private includes ----------------------------------------------------------*/

  /* Exported types ------------------------------------------------------------*/

  /* Exported constants --------------------------------------------------------*/

  /* Exported macro ------------------------------------------------------------*/
#define NUM_PI_DMA_BANKS 2
#define NUM_PI_CHANNELS 4
#define SAMPLES_PER_PI_CHANNEL 2
#define PI_CHANNEL_1 0
#define PI_CHANNEL_2 1
#define PI_CHANNEL_3 2
#define PI_CHANNEL_4 3
#define PI_BANK_1 0
#define PI_BANK_2 1

  /* Exported functions prototypes ---------------------------------------------*/

  /* Private defines -----------------------------------------------------------*/

  extern uint16_t dma_capture_buffer[NUM_PI_DMA_BANKS][NUM_PI_CHANNELS][SAMPLES_PER_PI_CHANNEL];
  extern volatile uint8_t data_ready_[NUM_PI_DMA_BANKS][NUM_PI_CHANNELS];
  extern uint16_t periods_us[NUM_PI_DMA_BANKS][NUM_PI_CHANNELS];
  extern uint16_t freqs_hz[NUM_PI_DMA_BANKS][NUM_PI_CHANNELS];

#ifdef __cplusplus
}
#endif

#endif /* MAIN_EXPORT_H */

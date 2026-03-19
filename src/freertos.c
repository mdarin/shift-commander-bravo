/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <uart_printf.h>
#include "usart.h"
#include "tim.h"
#include <local_misc.h>
#include <main_export.h>
#include <appxx_TODO.hpp>
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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
    .name = "defaultTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for PIMeasTask */
osThreadId_t PIMeasTaskHandle;
const osThreadAttr_t PIMeasTask_attributes = {
    .name = "PIMeasTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for PMonTask */
osThreadId_t PMonTaskHandle;
const osThreadAttr_t PMonTask_attributes = {
    .name = "PMonTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for rampCtrlTask */
osThreadId_t rampCtrlTaskHandle;
const osThreadAttr_t rampCtrlTask_attributes = {
    .name = "rampCtrlTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void process_pi_capture_data(void);
void process_pi_channel_data(TIM_HandleTypeDef *htim, uint32_t clk, uint8_t bank, uint8_t channel);
void print_pi_measurements(void);
void display_pi_measurements(void);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartPIMeasTask(void *argument);
void StartParamMonTask(void *argument);
void StartRamCtrlTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
  /* USER CODE BEGIN Init */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);
  PIMeasTaskHandle = osThreadNew(StartPIMeasTask, NULL, &PIMeasTask_attributes);
  PMonTaskHandle = osThreadNew(StartParamMonTask, NULL, &PMonTask_attributes);
  /* USER CODE END Init */
  /* USER CODE BEGIN Header */
  /**
   ******************************************************************************
   * File Name          : freertos.c
   * Description        : Code for freertos applications
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

  /**
   * @}
   */

  /**
   * @}
   */
}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  set_PWM_duty16(&htim1, TIM_CHANNEL_1, 15);
  set_PWM_duty16(&htim1, TIM_CHANNEL_2, 15);
  set_PWM_duty16(&htim1, TIM_CHANNEL_3, 15);
  set_PWM_duty16(&htim1, TIM_CHANNEL_4, 15);

  set_PWM_duty32(&htim2, TIM_CHANNEL_1, 15);
  set_PWM_duty32(&htim2, TIM_CHANNEL_2, 15);
  set_PWM_duty32(&htim2, TIM_CHANNEL_3, 15);
  set_PWM_duty32(&htim2, TIM_CHANNEL_4, 15);

  set_PWM_duty16(&htim4, TIM_CHANNEL_1, 15);
  set_PWM_duty16(&htim4, TIM_CHANNEL_2, 15);
  set_PWM_duty16(&htim4, TIM_CHANNEL_3, 15);
  set_PWM_duty16(&htim4, TIM_CHANNEL_4, 15);

  set_PWM_duty16(&htim9, TIM_CHANNEL_1, 15);
  set_PWM_duty16(&htim9, TIM_CHANNEL_2, 15);

  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);

  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);

  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);

  HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_2);

  /* Infinite loop */
  for (;;)
  {
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    uart_printf(&huart2, "\r\n== Hello DMA ==\r\n");

    osDelay(1000);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartPIMeasTask */
/**
 * @brief Function implementing the PIMeasTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartPIMeasTask */
void StartPIMeasTask(void *argument)
{
  /* USER CODE BEGIN StartPIMeasTask */
  HAL_TIM_IC_Start_DMA(&htim3, TIM_CHANNEL_1, (uint32_t *)dma_capture_buffer[PI_BANK_1][PI_CHANNEL_1], SAMPLES_PER_PI_CHANNEL);
  HAL_TIM_IC_Start_DMA(&htim3, TIM_CHANNEL_2, (uint32_t *)dma_capture_buffer[PI_BANK_1][PI_CHANNEL_2], SAMPLES_PER_PI_CHANNEL);
  HAL_TIM_IC_Start_DMA(&htim3, TIM_CHANNEL_3, (uint32_t *)dma_capture_buffer[PI_BANK_1][PI_CHANNEL_3], SAMPLES_PER_PI_CHANNEL);
  HAL_TIM_IC_Start_DMA(&htim3, TIM_CHANNEL_4, (uint32_t *)dma_capture_buffer[PI_BANK_1][PI_CHANNEL_4], SAMPLES_PER_PI_CHANNEL);

  HAL_TIM_IC_Start_DMA(&htim8, TIM_CHANNEL_1, (uint32_t *)dma_capture_buffer[PI_BANK_2][PI_CHANNEL_1], SAMPLES_PER_PI_CHANNEL);
  HAL_TIM_IC_Start_DMA(&htim8, TIM_CHANNEL_2, (uint32_t *)dma_capture_buffer[PI_BANK_2][PI_CHANNEL_2], SAMPLES_PER_PI_CHANNEL);
  HAL_TIM_IC_Start_DMA(&htim8, TIM_CHANNEL_3, (uint32_t *)dma_capture_buffer[PI_BANK_2][PI_CHANNEL_3], SAMPLES_PER_PI_CHANNEL);
  HAL_TIM_IC_Start_DMA(&htim8, TIM_CHANNEL_4, (uint32_t *)dma_capture_buffer[PI_BANK_2][PI_CHANNEL_4], SAMPLES_PER_PI_CHANNEL);

  /* Infinite loop */
  for (;;)
  {
    // Обработка захваченных данных
    process_pi_capture_data();

    // Вывод результатов измеренеий
    print_pi_measurements();

    osDelay(10);
  }
  /* USER CODE END StartPIMeasTask */
}

/* USER CODE BEGIN Header_StartParamMonTask */
/**
 * @brief Function implementing the PMonTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartParamMonTask */
void StartParamMonTask(void *argument)
{
  /* USER CODE BEGIN StartParamMonTask */
  /* Infinite loop */
  for (;;)
  {
    // Запускаем рампу на одноразовый проход
    appxx_TODO();

    osDelay(100);
  }
  /* USER CODE END StartParamMonTask */
}

/* USER CODE BEGIN Header_StartRamCtrlTask */
/**
 * @brief Function implementing the rampCtrlTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartRamCtrlTask */
void StartRamCtrlTask(void *argument)
{
  /* USER CODE BEGIN StartRamCtrlTask */
  /* Infinite loop */
  for (;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartRamCtrlTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/**
 * @brief
 * @retval None
 */
// Обработка захваченных данных
void process_pi_capture_data(void)
{
  // Вычисление периода между двумя фронтами
  // DMA записывает значения счетчика в массив dma_capture_buffer

  // Обработка данных для каждого банка
  {
    // Обработка данных для каждого канала
    for (uint8_t channel = 0; channel < NUM_PI_CHANNELS; channel++)
    {
      if (data_ready_[PI_BANK_1][channel])
      {
        // Сброс флага канала
        data_ready_[PI_BANK_1][channel] = 0;

        // Получить частоту тактирования таймера
        uint32_t htim_clk = HAL_RCC_GetPCLK1Freq() * 2;

        // Обработка захваченных данных канала
        process_pi_channel_data(&htim3, htim_clk, PI_BANK_1, channel);
      }
    }

    for (uint8_t channel = 0; channel < NUM_PI_CHANNELS; channel++)
    {
      if (data_ready_[PI_BANK_2][channel])
      {
        // Сброс флага канала
        data_ready_[PI_BANK_2][channel] = 0;

        uint32_t htim_clk = HAL_RCC_GetPCLK2Freq() * 2;

        // Обработка захваченных данных канала
        process_pi_channel_data(&htim8, htim_clk, PI_BANK_2, channel);
      }
    }
  }
}

/**
 * @brief
 * @retval None
 */
// Обработка данных конкретного канала
void process_pi_channel_data(TIM_HandleTypeDef *htim, uint32_t htim_clk, uint8_t bank, uint8_t channel)
{
  uint16_t capture1 = dma_capture_buffer[bank][channel][0];
  uint16_t capture2 = dma_capture_buffer[bank][channel][1];

  // // выбираем два разных значения чтобы вычисления были корректны
  // do
  // {
  //   array_min_safe16(dma_capture_buffer[bank][channel], 2, &capture1);
  // } while (capture1 == 0);
  // do
  // {
  //   array_max_safe16(dma_capture_buffer[bank][channel], 2, &capture2);
  // } while (capture2 == capture1 || capture2 == 0);

  uint32_t period_us = 0;

  if (capture2 > capture1)
  {
    // Нормальный случай (без переполнения)
    period_us = capture2 - capture1;
  }
  else
  {
    // Случай с переполнением счетчика
    period_us = (0xFFFF - capture1) + capture2 + 1; // +1 защита от 0 заначения
  }

  // Получить частоту тактирования
  float clk = htim_clk / (htim->Instance->PSC + 1);

  // Вычисление частоты (1 МГц = 1 тик/мкс)
  uint32_t frequency_hz = (uint32_t)(clk / (float)period_us);

  // Проверка на выбросы (например, шум)
  if (period_us < 10 || period_us > 100000)
  {
    // Отбрасываем некорректное измерение
    period_us = 0;
    frequency_hz = 0;
  }

  periods_us[bank][channel] = period_us;
  freqs_hz[bank][channel] = frequency_hz;
}

/**
 * @brief
 * @retval None
 */
// Вывод измерений PI
void print_pi_measurements(void)
{
  static uint32_t print_counter = 0;

  // Выводим не каждое измерение, а например, каждое 10-е
  if (++print_counter >= 1000)
  {
    print_counter = 0;
    display_pi_measurements();
  }
}

/**
 * @brief
 * @retval None
 */
// Отображение измерений
void display_pi_measurements(void)
{
  char msg[128] = {0};

  sprintf(msg, "\r\n=== PI Measurements ===\r\n");
  HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);

  for (uint8_t bank = 0; bank < NUM_PI_DMA_BANKS; bank++)
  {
    for (uint8_t channel = 0; channel < NUM_PI_CHANNELS; channel++)
    {
      sprintf(msg, "BK%u CH%u: ", bank + 1, channel + 1);
      HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);

      if (freqs_hz[bank][channel] > 0)
      {
        sprintf(msg, "Freq: %5u Hz, Period: %5u us", freqs_hz[bank][channel], periods_us[bank][channel]);
        HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
      }
      else
      {
        sprintf(msg, "No signal");
        HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
      }

      sprintf(msg, "\r\n");
      HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
    }
  }

  sprintf(msg, "=====================\r\n");
  HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

/* USER CODE END Application */

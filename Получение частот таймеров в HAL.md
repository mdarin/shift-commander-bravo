## Получение частот таймеров в HAL

В STM32 HAL есть несколько способов получить частоты тактирования таймеров:

### 1. Использование готовых макросов HAL

```c
// Получение частоты APB1 (периферия)
uint32_t apb1_freq = HAL_RCC_GetPCLK1Freq();    // Частота шины APB1

// Получение частоты APB2 (периферия)
uint32_t apb2_freq = HAL_RCC_GetPCLK2Freq();    // Частота шины APB2

// Получение частоты системной шины
uint32_t hclk_freq = HAL_RCC_GetHCLKFreq();      // Частота AHB
```

### 2. Расчет частоты таймеров APB1 и APB2

Так как таймеры на APB1 и APB2 могут иметь удвоитель частоты, нужно учитывать предделители шин:

```c
// Функция для получения реальной частоты таймера на APB1
uint32_t HAL_GetTimerClkFreq_APB1(void) {
    uint32_t apb1_freq = HAL_RCC_GetPCLK1Freq();
    uint32_t hclk_freq = HAL_RCC_GetHCLKFreq();
    
    // Если APB1 prescaler не равен 1, то частота таймера удваивается
    if (apb1_freq != hclk_freq) {
        return apb1_freq * 2;
    } else {
        return apb1_freq;
    }
}

// Функция для получения реальной частоты таймера на APB2
uint32_t HAL_GetTimerClkFreq_APB2(void) {
    uint32_t apb2_freq = HAL_RCC_GetPCLK2Freq();
    uint32_t hclk_freq = HAL_RCC_GetHCLKFreq();
    
    // Если APB2 prescaler не равен 1, то частота таймера удваивается
    if (apb2_freq != hclk_freq) {
        return apb2_freq * 2;
    } else {
        return apb2_freq;
    }
}
```

### 3. Получение частоты конкретного таймера через его экземпляр

```c
// Более универсальная функция для любого таймера
uint32_t HAL_GetTimerClockFreq(TIM_HandleTypeDef *htim) {
    uint32_t timer_instance = (uint32_t)htim->Instance;
    
    // Определяем к какой шине подключен таймер
    if ((timer_instance == (uint32_t)TIM1) || 
        (timer_instance == (uint32_t)TIM8) || 
        (timer_instance == (uint32_t)TIM9) || 
        (timer_instance == (uint32_t)TIM10) || 
        (timer_instance == (uint32_t)TIM11)) {
        // Таймеры на APB2
        return HAL_GetTimerClkFreq_APB2();
    } 
    else if ((timer_instance == (uint32_t)TIM2) || 
             (timer_instance == (uint32_t)TIM3) || 
             (timer_instance == (uint32_t)TIM4) || 
             (timer_instance == (uint32_t)TIM5) || 
             (timer_instance == (uint32_t)TIM6) || 
             (timer_instance == (uint32_t)TIM7) || 
             (timer_instance == (uint32_t)TIM12) || 
             (timer_instance == (uint32_t)TIM13) || 
             (timer_instance == (uint32_t)TIM14)) {
        // Таймеры на APB1
        return HAL_GetTimerClkFreq_APB1();
    }
    else {
        return 0; // Неизвестный таймер
    }
}
```

### 4. Пример использования для расчета PSC

```c
void Calculate_PSC_For_TIM(TIM_HandleTypeDef *htim, uint32_t desired_freq_hz) {
    // Получаем реальную частоту таймера
    uint32_t timer_clk = HAL_GetTimerClockFreq(htim);
    
    // Рассчитываем предделитель
    uint32_t psc = (timer_clk / desired_freq_hz) - 1;
    
    printf("Timer clock: %lu Hz\n", timer_clk);
    printf("Desired frequency: %lu Hz\n", desired_freq_hz);
    printf("Calculated PSC: %lu\n", psc);
    printf("Resulting frequency: %lu Hz\n", timer_clk / (psc + 1));
}

// Пример вызова
TIM_HandleTypeDef htim8;
Calculate_PSC_For_TIM(&htim8, 1000000); // Для 1 МГц
```

### 5. Компактные макросы

```c
// Компактные макросы для прямого использования
#define TIM_APB1_CLK (HAL_RCC_GetPCLK1Freq() * (HAL_RCC_GetHCLKFreq() == HAL_RCC_GetPCLK1Freq() ? 1 : 2))
#define TIM_APB2_CLK (HAL_RCC_GetPCLK2Freq() * (HAL_RCC_GetHCLKFreq() == HAL_RCC_GetPCLK2Freq() ? 1 : 2))

// Использование
uint32_t tim8_clk = TIM_APB2_CLK;  // Частота для TIM8, TIM1 и т.д.
uint32_t tim3_clk = TIM_APB1_CLK;  // Частота для TIM3, TIM2, TIM4 и т.д.
```

### 6. Полный пример для вашего случая

```c
void Check_Timer_Frequencies(void) {
    // Получаем частоты шин
    uint32_t hclk = HAL_RCC_GetHCLKFreq();
    uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
    uint32_t pclk2 = HAL_RCC_GetPCLK2Freq();
    
    // Рассчитываем частоты таймеров
    uint32_t tim_apb1_clk = pclk1 * (hclk == pclk1 ? 1 : 2);
    uint32_t tim_apb2_clk = pclk2 * (hclk == pclk2 ? 1 : 2);
    
    printf("System Clock (HCLK): %lu Hz\n", hclk);
    printf("APB1 Peripheral Clock (PCLK1): %lu Hz\n", pclk1);
    printf("APB2 Peripheral Clock (PCLK2): %lu Hz\n", pclk2);
    printf("TIMERs on APB1 Clock: %lu Hz\n", tim_apb1_clk);
    printf("TIMERs on APB2 Clock: %lu Hz\n", tim_apb2_clk);
    
    // Проверка для ваших таймеров
    printf("\nFor your configuration:\n");
    printf("TIM8 (advanced) clock: %lu Hz\n", tim_apb2_clk);
    printf("TIM3 (general) clock: %lu Hz\n", tim_apb1_clk);
    printf("TIM1 (advanced) clock: %lu Hz\n", tim_apb2_clk);
    printf("TIM4 (general) clock: %lu Hz\n", tim_apb1_clk);
    
    // Расчет для 1 МГц на TIM8
    uint32_t desired = 1000000; // 1 МГц
    uint32_t psc_tim8 = (tim_apb2_clk / desired) - 1;
    printf("\nFor TIM8 at 1 MHz:\n");
    printf("PSC = %lu (timer clock: %lu Hz)\n", psc_tim8, tim_apb2_clk / (psc_tim8 + 1));
}
```

Эти функции и макросы позволят вам всегда точно знать частоты таймеров независимо от настроек тактирования в вашем проекте.

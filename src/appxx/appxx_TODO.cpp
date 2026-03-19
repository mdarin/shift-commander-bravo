/* Includes ------------------------------------------------------------------*/
#include <Ramp.hpp>
#include <uart_printf.h>
#include <usart.h>

// Это пространство или слой программы, в котором уже можно и нужно использовать возможности С++.
// Здесь будут только вставки на С

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
// Создаем объекты
STM32Ramp<uint8_t> myRamp(0); // ramp object instantiate with a value of O
STM32Ramp<uint8_t> downRamp(255);
STM32Ramp<uint8_t> backandforthRamp(0,
                                    [](void *arg)
                                    {
                                        // Извлекаем значение обратно
                                        // uint8_t val = static_cast<uint8_t>(reinterpret_cast<uintptr_t>(arg));

                                        // uart_printf(&huart3, ">callback_val:%u\r\n", val);
                                    });
STM32Ramp<uint8_t> bounceRamp(0);
uint8_t targetValue = 255; // value to go to
uint8_t targetValueDown = 0;
unsigned long duration_ms = 5000; // duration in ms
bool started = false;
auto lambda = [](void *)
{
    // uart_printf(&huart2, "Hello from lambda!\r\n");
};
auto sum = [](int a, int b) -> int
{
    return a + b;
};

/* Private function prototypes -----------------------------------------------*/
// Здесь код на С не стоит уже активно использовать, следут писать на С++

extern "C"
{
    /**
     * @brief
     * @retval None
     */
    // Точка входа, в этой функции исполняется код зазачи в потоке операционной системы
    void appxx_TODO(void) // (можно передать аргументы, это данные которые можно получать от других потоков ОС)
    {
        // TODO: здесь код на С++ который реализует логику данной задачи выполняемой в потоке исполнеия RTOS

        if (!started)
        {
            started = true;
            myRamp.go(targetValue, duration_ms, SINUSOIDAL_OUT); // start interpolation (value to go to, duration in ms)
            downRamp.go(targetValueDown, duration_ms, LINEAR, BACKWARD);
            backandforthRamp.go(targetValue, duration_ms, LINEAR, FORWARD, FORTHANDBACK);
            bounceRamp.go(targetValue, duration_ms, BOUNCE_IN, FORWARD, LOOPFORWARD);
            // myRamp.setAutomation(false); // Disable autoupdate
            // myRamp.setGrain(10);
            uart_printf(&huart2, "RAMP | Started\r\n");
        }

        // uint8_t actual_interpolation_value = myRamp.update(); // update() return the actual interpolation value
        // uint8_t actual_interpolation_value = myRamp.getValue();

        if (myRamp.isActive())
        {
            uart_printf(&huart2, "RAMP | IsActive\r\n");
        }

        if (myRamp.isFinished())
        {
            uart_printf(&huart2, "RAMP FINISHED | ramp value: %u orig: %u targ: %u\r\n",
                        myRamp.getValue(), myRamp.getOrigin(), myRamp.getTarget());
            // started = false;

            lambda(nullptr);
            int result = sum(5, 3);
            uart_printf(&huart2, "lambda sum result: %d\r\n", result);
        }
        else
        {
            uart_printf(&huart2, "RAMP | ramp value: %u orig: %u targ: %u pos: %u dur: %u\r\n",
                        myRamp.getValue(), myRamp.getOrigin(), myRamp.getTarget(), myRamp.getPosition(), myRamp.getDuration()); // show result
        }

        // Every serial message formated >varName:1234\n will be ploted in teleplot.Other messages will be printed in the teleplot console.
        uart_printf(&huart3, ">ramp_up_val:%u\r\n", myRamp.getValue());
        uart_printf(&huart3, ">ramp_down_val:%u\r\n", downRamp.getValue());
        uart_printf(&huart3, ">ramp_ backandforth_val:%u\r\n", backandforthRamp.getValue());
        uart_printf(&huart3, ">ramp_bounce_val:%u\r\n", bounceRamp.getValue());
    }
} // end extern "C"

/* Task code -----------------------------------------------------------------*/

// TODO:

/* Private function  ---------------------------------------------------------*/

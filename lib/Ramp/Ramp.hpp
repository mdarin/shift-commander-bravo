// Ramp_STM32.h
#ifndef RAMP_HPP
#define RAMP_HPP

/*
    Как сгенерировать документацию:
    Установите Doxygen.
    Создайте файл Doxyfile командой doxygen -g.

    Настройте параметры:
    PROJECT_NAME = "STM32Ramp Library"
    INPUT = ./Ramp_STM32.h
    RECURSIVE = NO
    GENERATE_HTML = YES
    GENERATE_LATEX = NO
    HAVE_DOT = YES          # Для диаграмм классов
    CALL_GRAPH = YES        # Для графов вызовов

    Запустите генерацию: doxygen Doxyfile.

    Откройте html/index.html в браузере.
*/

#include <stdint.h>
#include <math.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

// Для STM32 используем CMSIS для математики
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// Используем аппаратный FPU для float операций
#ifdef __FPU_PRESENT
#define USE_FPU 1
#endif

/*-----------------------------
 MODES ENUM
 -----------------------------*/

/**
 * @brief Типы функций интерполяции (easing functions)
 * @details Каждый режим определяет математическую функцию f(t),
 *          где t ∈ [0;1] — нормированное время/прогресс,
 *          а результат — коэффициент интерполяции u ∈ [0;1].
 *          Финальное значение вычисляется как: result = ramp_interp(A, B, u).
 *
 * @par Категории режимов:
 * - **Polynomial**: Quadratic, Cubic, Quartic, Quintic — степенные функции
 * - **Trigonometric**: Sinusoidal, Circular — на основе тригонометрии
 * - **Exponential**: Exponential — экспоненциальный рост/спад
 * - **Physical**: Elastic, Bounce, Back — имитация физических эффектов
 *
 * @par Суффиксы:
 * - `_IN`: ускорение в начале, плавный старт
 * - `_OUT`: замедление в конце, плавная остановка
 * - `_INOUT`: комбинация: плавный старт и остановка
 *
 * @see ramp_calc(), ramp_interp()
 */
enum ramp_mode
{
    NONE = 0x00, ///< @brief Без интерполяции. @details Мгновенный переход: u = (t >= 1) ? 1 : 0. Используется для телепортации к целевому значению.

    // === QUADRATIC (степень 2) ===
    LINEAR = 0x01,          ///< @brief Линейная интерполяция. @details f(t) = t. Равномерное движение с постоянной скоростью.
    QUADRATIC_IN = 0x02,    ///< @brief Квадратичное ускорение. @details f(t) = t². Медленный старт, постепенное ускорение к концу.
    QUADRATIC_OUT = 0x03,   ///< @brief Квадратичное замедление. @details f(t) = 1 - (1-t)². Быстрый старт, плавная остановка.
    QUADRATIC_INOUT = 0x04, ///< @brief Квадратичное ускорение и замедление. @details Симметричная кривая: медленный старт → ускорение → замедление → остановка.

    // === CUBIC (степень 3) ===
    CUBIC_IN = 0x05,    ///< @brief Кубическое ускорение. @details f(t) = t³. Более выраженное замедление в начале, чем у Quadratic.
    CUBIC_OUT = 0x06,   ///< @brief Кубическое замедление. @details f(t) = 1 - (1-t)³. Более резкий старт и плавное торможение.
    CUBIC_INOUT = 0x07, ///< @brief Кубическое ускорение и замедление. @details Плавная S-образная кривая с симметричным профилем скорости.

    // === QUARTIC (степень 4) ===
    QUARTIC_IN = 0x08,    ///< @brief Квартическое ускорение. @details f(t) = t⁴. Очень медленный старт, резкое ускорение в конце.
    QUARTIC_OUT = 0x09,   ///< @brief Квартическое замедление. @details f(t) = 1 - (1-t)⁴. Быстрый старт, очень плавное "мягкое" торможение.
    QUARTIC_INOUT = 0x0A, ///< @brief Квартическое ускорение и замедление. @details Выраженная S-кривая с долгими фазами разгона и торможения.

    // === QUINTIC (степень 5) ===
    QUINTIC_IN = 0x0B,    ///< @brief Квинтическое ускорение. @details f(t) = t⁵. Экстремально медленный старт, почти мгновенный рывок к финишу.
    QUINTIC_OUT = 0x0C,   ///< @brief Квинтическое замедление. @details f(t) = 1 - (1-t)⁵. Резкий старт, экстремально плавное затухание скорости.
    QUINTIC_INOUT = 0x0D, ///< @brief Квинтическое ускорение и замедление. @details Максимально плавная S-кривая для "премиальных" анимаций.

    // === SINUSOIDAL (тригонометрические) ===
    SINUSOIDAL_IN = 0x0E,    ///< @brief Синусоидальное ускорение. @details f(t) = 1 - cos(t·π/2). Плавный гармонический разгон.
    SINUSOIDAL_OUT = 0x0F,   ///< @brief Синусоидальное замедление. @details f(t) = sin(t·π/2). Плавное гармоническое торможение.
    SINUSOIDAL_INOUT = 0x10, ///< @brief Синусоидальное ускорение и замедление. @details f(t) = (1 - cos(t·π)) / 2. Идеально плавная синусоидальная анимация.

    // === EXPONENTIAL (экспоненциальные) ===
    EXPONENTIAL_IN = 0x11,    ///< @brief Экспоненциальное ускорение. @details f(t) = 2^(10·(t-1)). Почти незаметный старт, затем взрывной рост.
    EXPONENTIAL_OUT = 0x12,   ///< @brief Экспоненциальное замедление. @details f(t) = 1 - 2^(-10·t). Мгновенный рывок, затем экспоненциальное затухание.
    EXPONENTIAL_INOUT = 0x13, ///< @brief Экспоненциальное ускорение и замедление. @details Комбинация: медленный старт → резкий скачок в середине → плавное затухание.

    // === CIRCULAR (круговые) ===
    CIRCULAR_IN = 0x14,    ///< @brief Круговое ускорение. @details f(t) = 1 - √(1 - t²). Движение по дуге: медленный старт с постепенным разгоном.
    CIRCULAR_OUT = 0x15,   ///< @brief Круговое замедление. @details f(t) = √(1 - (t-1)²). Быстрый старт с плавным "закруглённым" торможением.
    CIRCULAR_INOUT = 0x16, ///< @brief Круговое ускорение и замедление. @details Симметричная кривая на основе полуокружности.

    // === ELASTIC (эластичные/пружинные) ===
    ELASTIC_IN = 0x17,    ///< @brief Эластичное ускорение. @details Имитация растянутой пружины: отскок назад в начале, затем колебания с затуханием при разгоне.
    ELASTIC_OUT = 0x18,   ///< @brief Эластичное замедление. @details Пружинный эффект в конце: объект "пролетает" целевую точку и колеблется вокруг неё с затуханием.
    ELASTIC_INOUT = 0x19, ///< @brief Эластичное ускорение и замедление. @details Комбинация: отскок в начале + колебания при остановке.

    // === BACK (эффект "оттягивания") ===
    BACK_IN = 0x1A,    ///< @brief Эффект оттягивания в начале. @details Объект слегка движется назад (overshoot < 0), затем резко ускоряется вперёд.
    BACK_OUT = 0x1B,   ///< @brief Эффект оттягивания в конце. @details Объект "пролетает" целевую точку (overshoot > 1), затем возвращается назад.
    BACK_INOUT = 0x1C, ///< @brief Эффект оттягивания в начале и конце. @details Комбинация: небольшой откат в старте + перелёт и возврат на финише.

    // === BOUNCE (эффект "подпрыгивания") ===
    BOUNCE_IN = 0x1D,   ///< @brief Эффект подпрыгивания в начале. @details Имитация мяча: серия затухающих "отскоков" в стартовой позиции перед основным движением.
    BOUNCE_OUT = 0x1E,  ///< @brief Эффект подпрыгивания в конце. @details Мяч "приземляется" в целевой точке и подпрыгивает с затухающей амплитудой.
    BOUNCE_INOUT = 0x1F ///< @brief Эффект подпрыгивания в начале и конце. @details Комбинация: отскоки на старте + отскоки на финише.
};

/**
 * @brief Направление движения рампы
 */
enum speed_mode
{
    FORWARD = 0x00, ///< Движение от начального значения A к целевому B
    BACKWARD = 0x01 ///< Движение от целевого значения B к начальному A
};

/**
 * @brief Режимы зацикливания анимации/интерполяции
 *
 * Определяет поведение рампы после достижения конечной точки (A или B).
 * Используется совместно с @ref speed_mode для управления направлением движения.
 *
 * @note Режимы с суффиксом "BACK" предполагают начальное направление BACKWARD
 *       и стартовую позицию pos = dur.
 *
 * @par Пример использования:
 * @code
 * // Бесконечное плавное колебание яркости 0↔255 за 2 секунды
 * ramp.go(255, 2000, SINUSOIDAL_INOUT, FORWARD, BACKANDFORTH);
 * @endcode
 *
 * @see speed_mode, ramp_mode, STM32Ramp::go()
 */
enum loop_mode
{
    ONCEFORWARD = 0x00,  ///< Пройти путь A→B один раз и остановиться в точке B
    LOOPFORWARD = 0x01,  ///< Бесконечный цикл: при достижении B мгновенный сброс в A и повтор A→B
    FORTHANDBACK = 0x02, ///< Пройти путь A→B, затем B→A и остановиться в точке A (один полный цикл туда-обратно)
    ONCEBACKWARD = 0x03, ///< Пройти путь B→A один раз (в обратном направлении) и остановиться в точке A
    LOOPBACKWARD = 0x04, ///< Бесконечный цикл в обратном направлении: при достижении A сброс в B и повтор B→A
    BACKANDFORTH = 0x05  ///< Бесконечное колебание (ping-pong): A→B→A→B... с автоматической сменой направления
};

/*-----------------------------
 COLOR STRUCTURE
 -----------------------------*/

union rgbColor
{
    uint32_t value;
    struct
    {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t _;
    };
    rgbColor(uint32_t v = 0) : value(v & 0x00FFFFFF) {}
    operator uint32_t() const { return value; }
};

/*-----------------------------
 GENERIC FUNCTIONS
 -----------------------------*/

// Вспомогательные функции
float powin(float k, uint8_t p);
float powout(float k, uint8_t p);
float powinout(float k, uint8_t p);
float ramp_calc(float k, ramp_mode m);

// Интерполяция
template <typename T>
T ramp_interp(T A, T B, float u);

// Специализации для STM32 (оптимизированные)
template <>
uint8_t ramp_interp<uint8_t>(uint8_t A, uint8_t B, float u);
template <>
uint16_t ramp_interp<uint16_t>(uint16_t A, uint16_t B, float u);
template <>
uint32_t ramp_interp<uint32_t>(uint32_t A, uint32_t B, float u);

// Для RGB
rgbColor ramp_interp(rgbColor A, rgbColor B, float u);

/*-----------------------------
 CLASS TEMPLATE
 -----------------------------*/

/**
 * @brief Основной шаблонный класс для плавной интерполяции значений с поддержкой FreeRTOS
 *
 * @tparam T Тип интерполируемого значения (uint8_t, uint16_t, float, rgbColor и т.д.)
 *
 * @details Класс предоставляет:
 * - Потокобезопасный доступ через мьютекс (xMutex)
 * - Автоматическое обновление через аппаратный/программный таймер FreeRTOS
 * - Поддержку 32 видов интерполяции (линейная, квадратичная, синусоидальная и др.)
 * - Различные режимы зацикливания (одиночный, повтор, туда-обратно)
 * - Оптимизацию под STM32 с использованием FPU при наличии
 *
 * @par Пример базового использования:
 * @code
 * STM32Ramp<uint16_t> pwm_ramp(0);           // Создаём рампу для PWM (0-65535)
 * pwm_ramp.go(32768, 1000, SINUSOIDAL_INOUT); // Плавный переход за 1 сек
 * while(pwm_ramp.isRunning()) {
 *     set_pwm(pwm_ramp.getValue());           // Применяем значение
 *     vTaskDelay(10);
 * }
 * @endcode
 *
 * @note Для типов, отличных от арифметических, необходимо реализовать
 *       специализацию функции ramp_interp<T>().
 *
 * @see ramp_interp(), ramp_calc(), loop_mode, speed_mode
 */
template <class T = uint8_t>
class STM32Ramp
{
private:
    // TODO:
    // ? может стоит создавать таймер во вне и передавать как параметр конструктора
    // предусмотреть два варианта аппаратный таймер и программный таймер
    // передаются хендлы

    // Основные данные
    TickType_t pos, dur, t; // ramp position, duration and previous time
    TickType_t grain;       // period
    T initVal, val, A, B;   // initial, actual, origin and destination values
    bool automated;
    bool paused;
    enum loop_mode loop;      // loop mode
    enum ramp_mode mode;      // ramp mode
    enum speed_mode speed;    // speed mode
    void (*callback)(void *); // callback on timer fired

    // FreeRTOS специфичные поля
    xTimerHandle xTimer; // Interval timer
    xSemaphoreHandle xMutex;
    StaticTimer_t xTimerBuffer;     // Статический буфер для таймера
    StaticSemaphore_t xMutexBuffer; // Статический буфер для мьютекса

    // Callback для таймера
    static void timerCallback(TimerHandle_t xTimer)
    {
        STM32Ramp *rampObj = (STM32Ramp *)pvTimerGetTimerID(xTimer);
        if (rampObj != NULL && rampObj->automated)
        {
            rampObj->updateFromTimer();
            // T v = rampObj->getValue();
            // rampObj->callback();
        }
    }

    // Функция обновления, вызываемая из таймера
    void updateFromTimer()
    {
        if (xMutex != NULL && xSemaphoreTake(xMutex, 0) == pdTRUE)
        {
            updateInternal();
            xSemaphoreGive(xMutex);
        }
    }

    // Внутренняя логика обновления
    void updateInternal()
    {
        // if (mode != NONE)
        {
            TickType_t delta = grain;

            if (isFinished())
            {
                handleLoopMode();
            }

            if (!paused)
            {
                updatePosition(delta);
                updateValue();
            }
        }
    }

    void handleLoopMode()
    {
        switch (loop)
        {
        case LOOPFORWARD:
            pos = 0;
            break;
        case LOOPBACKWARD:
            pos = dur;
            break;
        case FORTHANDBACK:
        case BACKANDFORTH:
            speed = (speed == FORWARD) ? BACKWARD : FORWARD;
            break;
        default:
            break;
        }
    }

    void updatePosition(TickType_t delta)
    {
        switch (speed)
        {
        case FORWARD:
            if (pos + delta < dur)
                pos += delta;
            else
                pos = dur;
            break;
        case BACKWARD:
            if ((int64_t)pos - delta > 0)
                pos -= delta;
            else
                pos = 0;
            break;
        }
    }

    void updateValue()
    {
        // if (mode != NONE && dur > 0 && A != B)
        {
            float u = ramp_calc((float)pos / (float)dur, mode);
            val = ramp_interp(A, B, u);
        }
        // else
        // {
        //     val = B;
        // }
    }

public:
    // Конструктор
    STM32Ramp(T initialValue = 0, void (*callbackAction)(void *) = nullptr)
    {
        init(initialValue, callbackAction);
    }

    // TODO: продумать конструктор и настройки класса
    void init(T val_, void (*func_)(void *))
    {
        A = val_;
        B = val_;
        val = val_;
        initVal = val_;
        dur = 0;
        pos = 0;
        grain = pdMS_TO_TICKS(10); // 10 мс по умолчанию
        mode = NONE;
        loop = ONCEFORWARD;
        speed = FORWARD;
        paused = false;
        automated = true;
        callback = func_;

        // Создаем мьютекс статически
        xMutex = xSemaphoreCreateMutexStatic(&xMutexBuffer);

        // Создаем таймер статически
        xTimer = xTimerCreateStatic(
            "RampTmr",      // Наименование объекта таймер
            grain,          // Период (pdMS_TO_TICKS(1000) конвертация мс в тики)
            pdTRUE,         // pdTRUE - периодический, pdFALSE - одноразовый
            this,           // Идентификатор таймера
            timerCallback,  // Callback функция
            &xTimerBuffer); // Статический буфер

        // Таймер создан успешно?
        if (xTimer != NULL)
        {
            // Запуск таймера
            xTimerStart(xTimer, 0);
        }
        // ? надо продумать как работает этот механизм ещё
    }

    // Деструктор
    ~STM32Ramp()
    {
        if (xTimer != NULL)
        {
            xTimerStop(xTimer, 0);
            xTimerDelete(xTimer, 0);
        }
        if (xMutex != NULL)
        {
            vSemaphoreDelete(xMutex);
        }
    }

    // Запуск программного таймера RTOS
    // pdFAIL will be returned if the start command could not be sent to the timer command queue even after xTicksToWait ticks had passed.
    // pdPASS will be returned if the command was successfully sent to the timer command queue.
    bool start()
    {
        if (xTimer != NULL)
        {
            if (xTimerStart(xTimer, 0) == pdPASS)
            {
                return true;
            }
        }

        return false;
    }

    // Остановка таймера
    bool stop()
    {
        if (xTimer != NULL)
        {
            if (xTimerStop(xTimer, 0) == pdPASS)
            {
                return true;
            }

            return false;
        }

        return true;
    }

    // Проверка активен ли таймер
    bool isActive()
    {
        if (xTimerIsTimerActive(xTimer) == pdFALSE)
        {
            return false;
        }

        return true;
    }

    // Основной метод update - теперь потокобезопасный
    //? Как-будто не нужный
    T update()
    {
        if (!automated)
        {
            // Если автоматизация выключена, обновляем принудительно
            TickType_t now = xTaskGetTickCount();

            if (xMutex != NULL && xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
            {
                if ((now - t) >= grain)
                {
                    TickType_t delta = now - t;
                    t = now;
                    updatePosition(delta);
                    updateValue();
                }
                xSemaphoreGive(xMutex);
            }
        }
        return getValue();
    }

    // Returns the actual interpolation value (Потокобезопасный getValue)
    T getValue()
    {
        T result;
        if (xMutex != NULL && xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
        {
            result = val;
            xSemaphoreGive(xMutex);
        }
        else
        {
            result = val; // Fallback
        }
        return result;
    }

    // Go directly to a new value (метод с защитой)
    T go(T targetVal, unsigned long duration_ms = 0, ramp_mode mode_ = NONE, speed_mode speed_ = FORWARD, loop_mode loop_ = ONCEFORWARD)
    {
        TickType_t newDur = pdMS_TO_TICKS(duration_ms);

        if (xMutex != NULL && xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
        {
            setup(targetVal, newDur, mode_, speed_, loop_);
            xSemaphoreGive(xMutex);

            // Сбрасываем таймер для немедленного начала (перезапуск с начала периода)
            if (xTimer != NULL)
            {
                xTimerReset(xTimer, 0);
            }
        }

        return val;
    }

    // Set up RAMP params before start the timer
    void setup(T targetVal, unsigned long dur_ = 0, ramp_mode mode_ = NONE, speed_mode speed_ = FORWARD, loop_mode loop_ = ONCEFORWARD)
    {
        A = initVal;
        B = targetVal;
        mode = mode_;
        dur = dur_;

        if (dur_ == 0)
        {
            val = B;
        }

        if (loop_ < ONCEBACKWARD)
        {
            pos = 0;
            speed = FORWARD;
        }
        else
        {
            pos = dur;
            speed = BACKWARD;
        }
        loop = loop_;
        paused = false;
    }

    // Управление
    void pause()
    {
        if (xMutex != NULL && xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
        {
            paused = true;
            xSemaphoreGive(xMutex);
        }
    }

    void resume()
    {
        if (xMutex != NULL && xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
        {
            paused = false;
            xSemaphoreGive(xMutex);

            // Сбрасываем таймер для немедленного начала (перезапуск с начала периода)
            if (xTimer != NULL)
            {
                xTimerReset(xTimer, 0);
            }
        }
    }

    // Состояния
    bool isFinished()
    {
        if (speed == FORWARD)
            return (pos == dur);
        if (speed == BACKWARD)
            return (pos == 0);
        return false;
    }

    bool isRunning()
    {
        return (!isFinished() && !paused);
    }

    bool isPaused() { return paused; }

    // Геттеры и сеттеры

    // set the grain (interval beetween updates)
    void setGrain(unsigned long interval_ms)
    {
        if (xMutex != NULL && xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
        {
            grain = pdMS_TO_TICKS(interval_ms);
            xSemaphoreGive(xMutex);
        }

        // Обновляем период таймера
        if (xTimer != NULL)
        {
            xTimerChangePeriod(xTimer, grain, 0);
        }
    }

    void setAutomation(bool enable)
    {
        if (xMutex != NULL && xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
        {
            automated = enable;
            xSemaphoreGive(xMutex);
        }

        if (enable)
        {
            // Сброс таймера (перезапуск с начала периода)
            xTimerReset(xTimer, 0);
        }
    }

    float getCompletion()
    {
        if (dur == 0)
            return 100.0f;

        float result;
        if (xMutex != NULL && xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
        {
            result = (pos * 10000.0f / dur) / 100.0f;
            xSemaphoreGive(xMutex);
        }
        else
        {
            result = 0;
        }
        return result;
    }

    unsigned long getDuration() { return dur * portTICK_PERIOD_MS; }
    unsigned long getPosition() { return pos * portTICK_PERIOD_MS; }
    T getOrigin() { return A; }
    T getTarget() { return B; }

    // Получить хендл таймера для отладки
    TimerHandle_t getTimerHandle() { return xTimer; }
};

// TODO:
/*-----------------------------
 CLASS ALIAS
 -----------------------------*/

// typedef _ramp<> ramp;
// typedef _ramp<> rampByte;
// typedef _ramp<> rampUnsignedChar;
// typedef _ramp<char> rampChar;
// typedef _ramp<int> rampInt;
// typedef _ramp<unsigned int> rampUnsignedInt;
// typedef _ramp<long> rampLong;
// typedef _ramp<unsigned long> rampUnsignedLong;
// typedef _ramp<float> rampFloat;
// typedef _ramp<double> rampDouble;
// typedef _ramp<rgbColor> rampColor;

#endif // RAMP_HPP

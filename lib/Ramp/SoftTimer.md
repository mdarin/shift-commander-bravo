# Software Timer

## Основные понятия

Программные таймеры FreeRTOS позволяют выполнять функции через заданные интервалы времени. Они работают на аппаратном таймере, но управляются программно.

Важные концепции:

    - Таймеры выполняются в контексте специальной задачи (Timer Service Task)
    - Бывают одноразовые (one-shot) и периодические (auto-reload)
    -Функции callback выполняются из задачи таймера, а не из прерывания

Создание таймера

```c
// Создание таймера (динамическое выделение памяти)
TimerHandle_t xTimerCreate(
    const char * const pcTimerName,     // Имя таймера
    const TickType_t xTimerPeriod,      // Период в тиках
    const UBaseType_t uxAutoReload,     // pdTRUE - периодический, pdFALSE - одноразовый
    void * const pvTimerID,              // Идентификатор таймера
    TimerCallbackFunction_t pxCallback   // Функция callback
);

// Создание таймера со статическим выделением памяти
TimerHandle_t xTimerCreateStatic(
    const char * const pcTimerName,
    TickType_t xTimerPeriod,
    UBaseType_t uxAutoReload,
    void * pvTimerID,
    TimerCallbackFunction_t pxCallback,
    StaticTimer_t * pxTimerBuffer       // Статический буфер
);
```

Управление таймером

```c
// Запуск таймера (из задачи)
BaseType_t xResult = xTimerStart(xTimer, 0);
if (xResult == pdPASS) {
    // Таймер успешно запущен
}

// Запуск таймера (из прерывания)
BaseType_t xHigherPriorityTaskWoken = pdFALSE;
xResult = xTimerStartFromISR(xTimer, &xHigherPriorityTaskWoken);

// Остановка таймера
xResult = xTimerStop(xTimer, portMAX_DELAY);

// Сброс таймера (перезапуск с начала периода)
xResult = xTimerReset(xTimer, 100);

// Изменение периода
xResult = xTimerChangePeriod(xTimer, pdMS_TO_TICKS(2000), 100);
```

Проверка состояния

```c
// Проверка активен ли таймер
if (xTimerIsTimerActive(xTimer) == pdFALSE) {
    printf("Timer is not active\n");
}

// Получение имени таймера
const char *pcName = pcTimerGetName(xTimer);

// Получение/установка ID
void *pvID = pvTimerGetTimerID(xTimer);
vTimerSetTimerID(xTimer, (void *) newID);
```

Отладка

```c
// Получение информации о таймерах
void vPrintTimerInfo(TimerHandle_t xTimer) {
    printf("Timer: %s\n", pcTimerGetName(xTimer));
    printf("Active: %d\n", xTimerIsTimerActive(xTimer));
    
    // Доп. информация через ID
    uint32_t *pID = (uint32_t *) pvTimerGetTimerID(xTimer);
    if (pID) {
        printf("ID value: %lu\n", *pID);
    }
}
```

#include "Ramp.hpp"

// Оптимизированные математические функции для STM32

// Используем FPU для ускорения вычислений
#if defined(__FPU_PRESENT) && __FPU_PRESENT == 1
__attribute__((always_inline)) inline float fast_pow(float x, uint8_t n)
{
    float result = 1.0f;
    while (n--)
        result *= x;
    return result;
}
#else
#define fast_pow powf
#endif

float powin(float k, uint8_t p)
{
    return fast_pow(k, p);
}

float powout(float k, uint8_t p)
{
    return 1.0f - fast_pow(1.0f - k, p);
}

float powinout(float k, uint8_t p)
{
    k *= 2.0f;
    if (k < 1.0f)
        return 0.5f * fast_pow(k, p);
    return 1.0f - 0.5f * fabsf(fast_pow(2.0f - k, p));
}

float ramp_calc(float k, ramp_mode m)
{
    if (k == 0 || k == 1)
        return k;

    float a, p, s;

    switch (m)
    {
    case QUADRATIC_IN:
        return powin(k, 2);

    case QUADRATIC_OUT:
        return powout(k, 2);

    case QUADRATIC_INOUT:
        return powinout(k, 2);

    case CUBIC_IN:
        return powin(k, 3);

    case CUBIC_OUT:
        return powout(k, 3);

    case CUBIC_INOUT:
        return powinout(k, 3);

    case QUARTIC_IN:
        return powin(k, 4);

    case QUARTIC_OUT:
        return powout(k, 4);

    case QUARTIC_INOUT:
        return powinout(k, 4);

    case QUINTIC_IN:
        return powin(k, 5);

    case QUINTIC_OUT:
        return powout(k, 5);

    case QUINTIC_INOUT:
        return powinout(k, 5);

    case SINUSOIDAL_IN:
        return 1 - cos(k * (M_PI / 2));

    case SINUSOIDAL_OUT:
        return sin(k * (M_PI / 2));

    case SINUSOIDAL_INOUT:
        return -0.5 * (cos(M_PI * k) - 1);

    case EXPONENTIAL_IN:
        return pow(2, 10 * (k - 1));

    case EXPONENTIAL_OUT:
        return (1 - pow(2, -10 * k));

    case EXPONENTIAL_INOUT:
        k *= 2.;
        if (k < 1)
            return 0.5 * pow(2, 10 * (k - 1));
        k--;
        return 0.5 * (2 - pow(2, -10 * k));

    case CIRCULAR_IN:
        return -(sqrt(1 - k * k) - 1);

    case CIRCULAR_OUT:
        k--;
        return sqrt(1 - k * k);

    case CIRCULAR_INOUT:
        k *= 2;
        if (k < 1)
            return -0.5 * (sqrt(1 - k * k) - 1);
        k -= 2;
        return 0.5 * (sqrt(1 - k * k) + 1);

    case ELASTIC_IN:
        k -= 1;
        a = 1;
        p = 0.3 * 1.5;
        s = p * asin(1 / a) / (2 * M_PI);
        return -a * pow(2, 10 * k) * sin((k - s) * (2 * M_PI) / p);

    case ELASTIC_OUT:
        a = 1;
        p = 0.3;
        s = p * asin(1 / a) / (2 * M_PI);
        return (a * pow(2, -10 * k) * sin((k - s) * (2 * M_PI) / p) + 1);

    case ELASTIC_INOUT:
        k = k * 2 - 1;
        a = 1;
        p = 0.3 * 1.5;
        s = p * asin(1 / a) / (2 * M_PI);
        if ((k + 1) < 1)
            return -0.5 * a * pow(2, 10 * k) * sin((k - s) * (2 * M_PI) / p);
        return a * pow(2, -10 * k) * sin((k - s) * (2 * M_PI) / p) * 0.5 + 1;

    case BACK_IN:
        s = 1.70158;
        return k * k * ((s + 1) * k - s);

    case BACK_OUT:
        k--;
        s = 1.70158;
        return k * k * ((s + 1) * k + s) + 1;

    case BACK_INOUT:
        k *= 2;
        s = 1.70158;
        s *= 1.525;
        if (k < 1)
            return 0.5 * k * k * ((s + 1) * k - s);
        k -= 2;
        return 0.5 * k * k * ((s + 1) * k + s) + 1;

    case BOUNCE_IN:
        return 1 - ramp_calc(1 - k, BOUNCE_OUT);

    case BOUNCE_OUT:
        if (k < (1 / 2.75))
            return 7.5625 * k * k;
        if (k < (2 / 2.75))
        {
            k -= 1.5 / 2.75;
            return 7.5625 * k * k + 0.75;
        }
        if (k < (2.5 / 2.75))
        {
            k -= (2.25 / 2.75);
            return 7.5625 * k * k + 0.9375;
        }
        k -= (2.625 / 2.75);
        return 7.5625 * k * k + 0.984375;

    case BOUNCE_INOUT:
        if (k < 0.5)
        {
            return ramp_calc(k * 2, BOUNCE_IN) * 0.5;
        }
        return ramp_calc(k * 2 - 1, BOUNCE_OUT) * 0.5 + 0.5;
        break;

    case LINEAR:
    default:
        return k;
    }
}

/*-----------------------------
 TYPE ABSTRACTION
 -----------------------------*/

template <typename T>
T ramp_interp(T A, T B, float u)
{
    if (B >= A)
        return (A + (B - A) * u);
    else
        return (A - (A - B) * u);
}

// Оптимизированная интерполяция для целых типов

template <>
uint32_t ramp_interp<uint32_t>(uint32_t A, uint32_t B, float u)
{
    int32_t diff = B - A;
    int32_t result = A + (diff * u);
    if (result < 0)
        result = 0;
    if ((uint32_t)result > UINT32_MAX)
        result = UINT32_MAX;
    return (uint32_t)result;
}

template <>
uint16_t ramp_interp<uint16_t>(uint16_t A, uint16_t B, float u)
{
    int32_t diff = (int32_t)B - (int32_t)A;
    int32_t result = A + (int32_t)(diff * u);
    if (result < 0)
        result = 0;
    if ((uint16_t)result > UINT16_MAX)
        result = UINT16_MAX;
    return (uint16_t)result;
}

template <>
uint8_t ramp_interp<uint8_t>(uint8_t A, uint8_t B, float u)
{
    int32_t diff = (int32_t)B - (int32_t)A;
    int32_t result = A + (int32_t)(diff * u);
    if (result < 0)
        result = 0;
    if ((uint8_t)result > UINT8_MAX)
        result = UINT8_MAX;
    return (uint8_t)result;
}

rgbColor ramp_interp(rgbColor A, rgbColor B, float u)
{
    rgbColor output = 0;
    output.r = ramp_interp(A.r, B.r, u);
    output.g = ramp_interp(A.g, B.g, u);
    output.b = ramp_interp(A.b, B.b, u);
    return output;
}

// Явная инстанциация шаблонов
template class STM32Ramp<uint8_t>;
template class STM32Ramp<int8_t>;
template class STM32Ramp<uint16_t>;
template class STM32Ramp<int16_t>;
template class STM32Ramp<uint32_t>;
template class STM32Ramp<int32_t>;
template class STM32Ramp<float>;
template class STM32Ramp<double>;
template class STM32Ramp<rgbColor>;

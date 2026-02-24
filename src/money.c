#include "../include/money.h"

#include <math.h>

const Money MONEY_ZERO = {0};

Money money_from_major(const long double major_amount)
{
    Money result;
    result.value = llroundl(major_amount * CURRENCY_SCALE);
    return result;
}

long double money_to_major(const Money amount)
{
    return (long double)amount.value / CURRENCY_SCALE;
}

Money money_add(const Money a, const Money b)
{
    Money result;
    result.value = a.value + b.value;
    return result;
}

Money money_sub(const Money a, const Money b)
{
    Money result;
    result.value = a.value - b.value;
    return result;
}

Money money_mul(const Money base, const long double factor)
{
    Money result;
    result.value = llroundl(base.value * factor);
    return result;
}

Money money_div(const Money base, const int divisor)
{
    Money result = {0};
    if (divisor != 0) {
        result.value = base.value / divisor;
    }
    return result;
}

bool money_eq(const Money a, const Money b)
{
    return a.value == b.value;
}

bool money_gt(const Money a, const Money b)
{
    return a.value > b.value;
}

bool money_lt(const Money a, const Money b)
{
    return a.value < b.value;
}

bool money_gte(const Money a, const Money b)
{
    return a.value >= b.value;
}

bool money_lte(const Money a, const Money b)
{
    return a.value <= b.value;
}

bool money_is_zero(const Money a)
{
    return a.value == 0;
}

bool money_is_positive(const Money a)
{
    return a.value > 0;
}

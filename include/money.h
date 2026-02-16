#ifndef STATISTICALDATAPROCESSOR_MONEY_H
#define STATISTICALDATAPROCESSOR_MONEY_H

#include <stdbool.h>

#define CURRENCY_SCALE 100

typedef struct {
    long long value;
} Money;

extern const Money MONEY_ZERO;

Money money_from_major(long double major_amount);
long double money_to_major(Money amount);

Money money_add(Money a, Money b);
Money money_sub(Money a, Money b);
Money money_mul(Money base, long double factor);
Money money_div(Money base, int divisor);

bool money_eq(Money a, Money b);
bool money_gt(Money a, Money b);
bool money_lt(Money a, Money b);
bool money_gte(Money a, Money b);
bool money_lte(Money a, Money b);

bool money_is_zero(Money a);
bool money_is_positive(Money a);

#endif // STATISTICALDATAPROCESSOR_MONEY_H
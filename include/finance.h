#ifndef STATISTICALDATAPROCESSOR_FINANCE_H
#define STATISTICALDATAPROCESSOR_FINANCE_H

#include <math.h>

#define CURRENCY_SCALE 100

#define TO_MINOR_UNIT(major) ((Money)llroundl((long double)(major) * CURRENCY_SCALE))
#define TO_MAJOR_UNIT(minor) ((long double)(minor) / (long double)CURRENCY_SCALE)

typedef long long Money;

typedef struct {
    long double value;
} Rate;

typedef enum {
    LOAN_EQUAL_INSTALLMENTS,
    LOAN_DECREASING_INSTALLMENTS
} LoanType;

typedef enum {
    STRATEGY_REDUCE_TERM,
    STRATEGY_REDUCE_INSTALLMENT
} OverpaymentStrategy;

typedef struct {
    Money capital;
    Money interest;
    Money payment;
    Money balance;
} Installment;

typedef struct {
    Installment *items;
    int count;
    Money total_interest;
    Money total_paid;
} LoanSchedule;

void free_schedule(LoanSchedule *schedule);

Rate create_rate(long double value);

LoanSchedule calculate_schedule(Money principal, Rate annual_rate, int months, LoanType type);

LoanSchedule calculate_dynamic_schedule(Money principal, Rate annual_rate, int months, LoanType type, const Money *custom_payments, OverpaymentStrategy strategy);

#endif //STATISTICALDATAPROCESSOR_FINANCE_H
#ifndef STATISTICALDATAPROCESSOR_FINANCE_H
#define STATISTICALDATAPROCESSOR_FINANCE_H

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

typedef enum {
    FINANCE_SUCCESS = 0,
    FINANCE_ERR_INVALID_PRINCIPAL,
    FINANCE_ERR_INVALID_MONTHS,
    FINANCE_ERR_INVALID_ARGUMENT,
    FINANCE_ERR_NULL_RATES,
    FINANCE_ERR_INVALID_RATE,
    FINANCE_ERR_ALLOCATION_FAILED,
    FINANCE_ERR_NEGATIVE_AMORTIZATION,
    FINANCE_ERR_PAYMENT_TOO_LARGE,
    FINANCE_ERR_NUMERIC_OVERFLOW,
} FinanceErrorCode;

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

int calculate_schedule(Money principal, Rate annual_rate, int months, LoanType type, LoanSchedule *out_schedule);

int calculate_dynamic_schedule(Money principal, const Rate *annual_rates, int months, LoanType type, const Money *custom_payments, OverpaymentStrategy strategy, LoanSchedule *out_schedule);

#endif //STATISTICALDATAPROCESSOR_FINANCE_H
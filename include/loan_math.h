#ifndef STATISTICALDATAPROCESSOR_FINANCE_H
#define STATISTICALDATAPROCESSOR_FINANCE_H

#include "money.h"

typedef struct {
    long double value;
} Rate;

typedef enum { LOAN_EQUAL_INSTALLMENTS, LOAN_DECREASING_INSTALLMENTS } LoanType;

typedef enum { STRATEGY_REDUCE_TERM, STRATEGY_REDUCE_INSTALLMENT } OverpaymentStrategy;

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
    Money principal;
    int term_months;
    LoanType type;
} LoanDefinition;

typedef struct {
    const Rate *annual_rates;
} MarketScenario;

typedef struct {
    OverpaymentStrategy strategy;
    const Money *custom_payments;
} SimulationConfig;

typedef struct {
    Installment *items;
    int count;
    Money total_interest;
    Money total_paid;
} LoanSchedule;

typedef struct {
    Money current_balance;
    Money last_total_payment;
    int current_month;
} SimulationState;

void free_schedule(LoanSchedule *schedule);
Rate create_rate(long double value);
const char *finance_error_string(FinanceErrorCode code);

Money calculate_monthly_interest(Money balance, Rate current_rate);
FinanceErrorCode calculate_baseline_payment(const LoanDefinition *loan,
                                            const MarketScenario *market,
                                            const SimulationState *state,
                                            Money interest,
                                            Money *out_payment);

#endif // STATISTICALDATAPROCESSOR_FINANCE_H

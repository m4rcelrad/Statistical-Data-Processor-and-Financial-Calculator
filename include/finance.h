#ifndef STATISTICALDATAPROCESSOR_FINANCE_H
#define STATISTICALDATAPROCESSOR_FINANCE_H

typedef enum {
    LOAN_EQUAL_INSTALLMENTS,
    LOAN_DECREASING_INSTALLMENTS
} LoanType;

typedef struct {
    long double capital;
    long double interest;
    long double payment;
    long double balance;
} Installment;

typedef struct {
    Installment *items;
    int count;
    long double total_interest;
    long double total_paid;
} LoanSchedule;

void free_schedule(LoanSchedule *schedule);

LoanSchedule calculate_schedule(long double principal, long double annual_rate, int months, LoanType type);

LoanSchedule calculate_dynamic_schedule(long double principal, long double annual_rate, int months, LoanType type, const long double *custom_payments);

#endif //STATISTICALDATAPROCESSOR_FINANCE_H
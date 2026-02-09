//
// Created by maryc on 8.02.2026.
//

#ifndef STATISTICALDATAPROCESSOR_FINANCE_H
#define STATISTICALDATAPROCESSOR_FINANCE_H

typedef enum {
    LOAN_EQUAL_INSTALLMENTS,
    LOAN_DECREASING_INSTALLMENTS
} LoanType;

void calculate_and_print_schedule(double principal, double annual_rate, int months, LoanType type);

#endif //STATISTICALDATAPROCESSOR_FINANCE_H
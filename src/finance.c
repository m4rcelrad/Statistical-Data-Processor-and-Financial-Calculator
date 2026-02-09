#include <stdio.h>
#include <math.h>

#include "finance.h"

void calculate_and_print_schedule(double principal, double annual_rate, int months, LoanType type) {
    double sum_interest = 0.0;
    double sum_paid = 0.0;

    double current_balance = principal;
    double monthly_rate = annual_rate / 12.0;

    double fixed_installment = 0.0;
    double fixed_capital_part = 0.0;

    int is_zero_interest_flag = (monthly_rate == 0) ? 1 : 0;

    if (type == LOAN_EQUAL_INSTALLMENTS && monthly_rate > 0) {
        fixed_installment = principal * monthly_rate * pow(1 + monthly_rate, months) / (pow(1 + monthly_rate, months) - 1);
    } else {
        fixed_capital_part = principal / months;
    }

    printf("\nLoan Schedule:\n");
    printf("-------------------------------------------------------------------\n");
    printf("| %3s | %12s | %12s | %12s | %12s |\n", "No.", "Principal", "Interest", "Payment", "Balance");
    printf("-------------------------------------------------------------------\n");

    for (int i = 1; i <= months; i++) {
        double interest_part = 0.0;
        double capital_part;
        double total_payment;

        if (!is_zero_interest_flag) {
            interest_part = current_balance * monthly_rate;
        }

        if (type == LOAN_EQUAL_INSTALLMENTS && !is_zero_interest_flag) {
            total_payment = fixed_installment;
            capital_part = total_payment - interest_part;
        } else {
            capital_part = fixed_capital_part;

            if (i == months) {
                capital_part = current_balance;
            }
            total_payment = capital_part + interest_part;
        }

        if (capital_part > current_balance) {
            capital_part = current_balance;
            total_payment = capital_part + interest_part;
        }

        current_balance -= capital_part;

        if (current_balance < 0) current_balance = 0.0;

        sum_interest += interest_part;
        sum_paid += total_payment;

        printf("| %3d | %12.2f | %12.2f | %12.2f | %12.2f |\n", i, capital_part, interest_part, total_payment, current_balance);
    }
    printf("-------------------------------------------------------------------\n");
    printf("SUMMARY:\n");
    printf("Total Principal Paid: %12.2f\n", sum_paid - sum_interest);
    printf("Total Interest Cost:  %12.2f\n", sum_interest);
    printf("Total Amount Paid:    %12.2f\n", sum_paid);
    printf("-------------------------------------------------------------------\n");
}

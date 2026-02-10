#include <stdio.h>
#include "finance.h"
#include "report.h"

int main(void) {
    Money principal = TO_MINOR_UNIT(100000);
    Rate rate = create_rate(0.05);
    int months = 12;

    LoanSchedule schedule = calculate_dynamic_schedule(
        principal, rate, months, LOAN_EQUAL_INSTALLMENTS, NULL, STRATEGY_REDUCE_TERM
    );

    if (schedule.items) {
        print_schedule_to_console(&schedule);
        save_schedule_to_csv(&schedule, "loan_report.csv");
        free_schedule(&schedule);
    } else {
        printf("Calculation failed.\n");
    }

    Money payments[12] = {0};
    payments[2] = TO_MINOR_UNIT(10000);

    schedule = calculate_dynamic_schedule(
        principal, rate, months, LOAN_EQUAL_INSTALLMENTS, payments, STRATEGY_REDUCE_INSTALLMENT
    );

    if (schedule.items) {
        printf("\n--- STRATEGY_REDUCE_INSTALLMENT ---\n");
        print_schedule_to_console(&schedule);
        free_schedule(&schedule);
    } else {
        printf("Calculation failed.\n");
    }

    schedule = calculate_dynamic_schedule(
        principal, rate, months, LOAN_EQUAL_INSTALLMENTS, payments, STRATEGY_REDUCE_TERM
    );
    if (schedule.items) {
        printf("\n--- STRATEGY_REDUCE_TERM ---\n");
        print_schedule_to_console(&schedule);
        free_schedule(&schedule);
    } else {
        printf("Calculation failed.\n");
    }

    return 0;
}
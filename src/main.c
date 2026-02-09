#include <stdio.h>
#include "finance.h"
#include "report.h"

int main(void) {
    long double principal = 100000.0L;
    long double rate = 0.05L;
    int months = 12;

    LoanSchedule schedule = calculate_dynamic_schedule(
        principal, rate, months, LOAN_EQUAL_INSTALLMENTS, NULL
    );


    if (schedule.items) {
        print_schedule_to_console(&schedule);

        save_schedule_to_csv(&schedule, "loan_report.csv");

        free_schedule(&schedule);
    } else {
        printf("Calculation failed.\n");
    }

    long double payments[12] = {0};
    payments[2] = 10000.00;

    schedule = calculate_dynamic_schedule(
        principal, rate, months, LOAN_EQUAL_INSTALLMENTS, payments
    );

    if (schedule.items) {
        print_schedule_to_console(&schedule);

        save_schedule_to_csv(&schedule, "loan_report.csv");

        free_schedule(&schedule);
    } else {
        printf("Calculation failed.\n");
    }

    return 0;
}
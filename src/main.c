#include <stdio.h>
#include <stdlib.h>
#include "finance.h"
#include "report.h"

int main(void) {
    Money principal = TO_MINOR_UNIT(100000);
    int months = 12;

    Rate *rates = calloc(months, sizeof(Rate));

    for(int i=0; i<6; i++) rates[i] = create_rate(0.05);
    for(int i=6; i<12; i++) rates[i] = create_rate(0.09);

    printf("\n--- SCENARIO: VARIABLE RATES (5%% -> 9%%) ---\n");

    LoanSchedule schedule = calculate_dynamic_schedule(
        principal, rates, months, LOAN_EQUAL_INSTALLMENTS, NULL, STRATEGY_REDUCE_INSTALLMENT
    );

    if (schedule.items) {
        print_schedule_to_console(&schedule);
        free_schedule(&schedule);
    }

    free(rates);
    return 0;
}
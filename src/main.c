#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "loan_math.h"
#include "loan_simulation.h"
#include "money.h"
#include "report.h"

int main(void) {

    int months = 12;
    Rate *rates = malloc(sizeof(Rate) * months);
    if (!rates) {
        fprintf(stderr, "Memory allocation error for rates!\n");
        return 1;
    }

    for(int i = 0; i < months; i++) {
        rates[i] = create_rate(0.05);
    }

    MarketScenario market = {
        .annual_rates = rates
    };

    LoanDefinition loan = {
        .principal = money_from_major(100000.0),
        .term_months = months,
        .type = LOAN_EQUAL_INSTALLMENTS,
    };

    SimulationConfig config = {
        .strategy = STRATEGY_REDUCE_TERM,
        .custom_payments = NULL
    };

    LoanSchedule result;
    FinanceErrorCode err = run_loan_simulation(&loan, &market, &config, &result);

    if (err == FINANCE_SUCCESS) {

        print_schedule_to_console(&result);

        free_schedule(&result);
    } else {
        printf("Error occurred: %s (Code: %d)\n", finance_error_string(err), err);
    }

    free(rates);

    return 0;
}
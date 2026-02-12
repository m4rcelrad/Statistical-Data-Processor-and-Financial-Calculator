#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "finance.h"
#include "report.h"


static int tests_run = 0;
static int tests_passed = 0;

#define ASSERT_TRUE(cond, msg) do { \
    if (!(cond)) { \
        printf("[FAIL] %s: %s\n", __func__, msg); \
        return 0; \
    } \
} while(0)

#define ASSERT_EQ(actual, expected, msg) do { \
    if ((actual) != (expected)) { \
        printf("[FAIL] %s: %s (Expected %d, got %d)\n", __func__, msg, (int)(expected), (int)(actual)); \
        return 0; \
    } \
} while(0)

void run_test(int (*test_func)(), const char *name) {
    tests_run++;
    printf("Running %-40s ... ", name);
    if (test_func()) {
        printf("PASS\n");
        tests_passed++;
    } else {
    }
}


int test_validation_errors() {
    LoanSchedule schedule;
    const Rate rate = create_rate(0.05);

    int res = calculate_schedule(TO_MINOR_UNIT(-100), rate, 12, LOAN_EQUAL_INSTALLMENTS, &schedule);
    ASSERT_EQ(res, FINANCE_ERR_INVALID_PRINCIPAL, "Should reject negative principal");

    res = calculate_schedule(TO_MINOR_UNIT(1000), rate, 0, LOAN_EQUAL_INSTALLMENTS, &schedule);
    ASSERT_EQ(res, FINANCE_ERR_INVALID_MONTHS, "Should reject 0 months");

    res = calculate_dynamic_schedule(TO_MINOR_UNIT(1000), NULL, 12, LOAN_EQUAL_INSTALLMENTS, NULL, STRATEGY_REDUCE_TERM, &schedule);
    ASSERT_EQ(res, FINANCE_ERR_NULL_RATES, "Should reject NULL rates array");

    return 1;
}

int test_standard_annuity() {
    LoanSchedule schedule;
    const Money principal = TO_MINOR_UNIT(1200);
    const Rate rate = create_rate(0.0);

    const int res = calculate_schedule(principal, rate, 12, LOAN_EQUAL_INSTALLMENTS, &schedule);
    ASSERT_EQ(res, FINANCE_SUCCESS, "Calculation should succeed");

    ASSERT_EQ(schedule.count, 12, "Should have 12 installments");
    ASSERT_EQ(schedule.total_paid, principal, "Total paid should equal principal at 0%");

    ASSERT_EQ(schedule.items[0].payment, TO_MINOR_UNIT(100), "Payment should be 100 at 0%");

    free_schedule(&schedule);
    return 1;
}

int test_negative_amortization() {
    LoanSchedule schedule;
    const Money principal = TO_MINOR_UNIT(100000);
    const Rate rate = create_rate(0.05);

    Money bad_payments[12];
    for(int i=0; i<12; i++) bad_payments[i] = TO_MINOR_UNIT(10);

    Rate *rates = malloc(sizeof(Rate) * 12);
    for(int i=0; i<12; i++) rates[i] = rate;

    const int res = calculate_dynamic_schedule(principal, rates, 12, LOAN_EQUAL_INSTALLMENTS, bad_payments, STRATEGY_REDUCE_TERM, &schedule);

    free(rates);
    ASSERT_EQ(res, FINANCE_ERR_NEGATIVE_AMORTIZATION, "Should detect negative amortization");

    return 1;
}

int test_overpayment_reduce_term() {
    LoanSchedule schedule;
    const Money principal = TO_MINOR_UNIT(10000);
    const Rate rate = create_rate(0.05);
    const int months = 12;

    Money payments[12] = {0};
    payments[1] = TO_MINOR_UNIT(5000);

    Rate *rates = malloc(sizeof(Rate) * months);
    for(int i=0; i<months; i++) rates[i] = rate;

    const int res = calculate_dynamic_schedule(principal, rates, months, LOAN_EQUAL_INSTALLMENTS, payments, STRATEGY_REDUCE_TERM, &schedule);

    free(rates);
    ASSERT_EQ(res, FINANCE_SUCCESS, "Calculation should succeed");

    ASSERT_TRUE(schedule.count < 8, "Loan should be repaid early due to huge overpayment");
    ASSERT_EQ(schedule.items[schedule.count-1].balance, 0, "Final balance must be 0");

    free_schedule(&schedule);
    return 1;
}

int test_variable_rates() {
    LoanSchedule schedule;
    const Money principal = TO_MINOR_UNIT(10000);
    const int months = 5;

    Rate *rates = malloc(sizeof(Rate) * months);
    rates[0] = create_rate(0.0);
    rates[1] = create_rate(0.0);
    rates[2] = create_rate(0.0);
    rates[3] = create_rate(0.10);
    rates[4] = create_rate(0.10);

    const int res = calculate_dynamic_schedule(principal, rates, months, LOAN_EQUAL_INSTALLMENTS, NULL, STRATEGY_REDUCE_TERM, &schedule);

    free(rates);
    ASSERT_EQ(res, FINANCE_SUCCESS, "Calculation should succeed");

    ASSERT_EQ(schedule.items[0].interest, 0, "Interest should be 0 at 0%");
    ASSERT_TRUE(schedule.items[3].interest > 0, "Interest should appear when rate rises");

    free_schedule(&schedule);
    return 1;
}

int test_reporting() {
    LoanSchedule schedule;
    const Money principal = TO_MINOR_UNIT(5000);
    const Rate rate = create_rate(0.05);

    calculate_schedule(principal, rate, 6, LOAN_EQUAL_INSTALLMENTS, &schedule);

    const int csv_res = save_schedule_to_csv(&schedule, "test_report.csv");
    ASSERT_EQ(csv_res, 0, "CSV save should return 0 (success)");


    free_schedule(&schedule);
    remove("test_report.csv");
    return 1;
}


int main(void) {
    printf("========================================TESTS========================================\n");

    run_test(test_validation_errors,     "1. Input Validation Logic");
    run_test(test_standard_annuity,      "2. Standard Annuity (0% check)");
    run_test(test_negative_amortization, "3. Negative Amortization Guard");
    run_test(test_overpayment_reduce_term,"4. Overpayment (Strategy: Reduce Term)");
    run_test(test_variable_rates,        "5. Variable Interest Rates");
    run_test(test_reporting,             "6. Reporting Module (CSV)");

    printf("\n=====================================================================================\n");
    printf("TESTS COMPLETED: %d/%d PASSED\n", tests_passed, tests_run);

    if (tests_passed == tests_run) {
        printf("RESULT: ALL GREEN. SYSTEM READY.\n");
        return 0;
    }
    printf("RESULT: SOME TESTS FAILED.\n");
    return 1;
}
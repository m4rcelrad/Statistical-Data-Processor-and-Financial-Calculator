#include <stdlib.h>

#include "loan_math.h"
#include "money.h"
#include "unity/unity.h"

void test_CreateRate(void)
{
    Rate r = create_rate(0.05);
    TEST_ASSERT_EQUAL_FLOAT(0.05, r.value);
}

void test_FreeSchedule(void)
{
    LoanSchedule schedule;
    schedule.items = malloc(10 * sizeof(Installment));
    schedule.count = 10;
    schedule.total_interest = money_from_major(100);
    schedule.total_paid = money_from_major(1000);

    free_schedule(&schedule);

    TEST_ASSERT_NULL(schedule.items);
    TEST_ASSERT_EQUAL_INT(0, schedule.count);
    TEST_ASSERT_TRUE(money_is_zero(schedule.total_interest));
    TEST_ASSERT_TRUE(money_is_zero(schedule.total_paid));
}

void test_FinanceErrorString(void)
{
    TEST_ASSERT_EQUAL_STRING("Success", finance_error_string(FINANCE_SUCCESS));
    TEST_ASSERT_EQUAL_STRING("Invalid principal amount",
                             finance_error_string(FINANCE_ERR_INVALID_PRINCIPAL));
    TEST_ASSERT_EQUAL_STRING("Unknown error", finance_error_string((FinanceErrorCode)999));
}

void test_CalculateMonthlyInterest_Standard(void)
{
    Money balance = money_from_major(120000);
    Rate rate = create_rate(0.05);
    Money interest = calculate_monthly_interest(balance, rate);
    TEST_ASSERT_EQUAL_INT64(50000, interest.value);
}

void test_CalculateMonthlyInterest_ZeroRate(void)
{
    Money balance = money_from_major(100000);
    Rate rate = create_rate(0.00);

    Money interest = calculate_monthly_interest(balance, rate);
    TEST_ASSERT_TRUE(money_is_zero(interest));
}

void test_CalculateBaselinePayment_EqualInstallments(void)
{
    LoanDefinition loan = {
        .term_months = 2, .type = LOAN_EQUAL_INSTALLMENTS, .principal = money_from_major(1000)};

    Rate rates[2] = {create_rate(0.0), create_rate(0.0)};
    MarketScenario market = {.annual_rates = rates};

    SimulationState state = {.current_month = 0, .current_balance = money_from_major(1000)};

    Money interest = MONEY_ZERO;
    Money payment = MONEY_ZERO;

    FinanceErrorCode err = calculate_baseline_payment(&loan, &market, &state, interest, &payment);

    TEST_ASSERT_EQUAL(FINANCE_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT64(50000, payment.value);
}

void test_CalculateBaselinePayment_DecreasingInstallments(void)
{
    LoanDefinition loan = {.term_months = 12,
                           .type = LOAN_DECREASING_INSTALLMENTS,
                           .principal = money_from_major(1200)};

    Rate rates[12];
    MarketScenario market = {.annual_rates = rates};

    SimulationState state = {.current_month = 0, .current_balance = money_from_major(1200)};

    Money interest = money_from_major(50);
    Money payment = MONEY_ZERO;

    FinanceErrorCode err = calculate_baseline_payment(&loan, &market, &state, interest, &payment);

    TEST_ASSERT_EQUAL(FINANCE_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT64(15000, payment.value);
}

void run_loan_math_tests(void)
{
    RUN_TEST(test_CreateRate);
    RUN_TEST(test_FreeSchedule);
    RUN_TEST(test_FinanceErrorString);
    RUN_TEST(test_CalculateMonthlyInterest_Standard);
    RUN_TEST(test_CalculateMonthlyInterest_ZeroRate);
    RUN_TEST(test_CalculateBaselinePayment_EqualInstallments);
    RUN_TEST(test_CalculateBaselinePayment_DecreasingInstallments);
}

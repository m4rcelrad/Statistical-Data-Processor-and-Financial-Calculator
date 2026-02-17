#include "unity.h"
#include "loan_simulation.h"
#include "money.h"
#include "loan_math.h"
#include <stdlib.h>

static Rate* create_constant_rates(const int months, const double value) {
    Rate* rates = malloc(sizeof(Rate) * months);
    for (int i = 0; i < months; i++) {
        rates[i] = create_rate(value);
    }
    return rates;
}

void test_Init(void) {
    SimulationState state;
    Money principal = money_from_major(100000);

    loan_simulation_init(&state, principal);

    TEST_ASSERT_EQUAL_INT64(principal.value, state.current_balance.value);
    TEST_ASSERT_TRUE(money_is_zero(state.last_total_payment));
    TEST_ASSERT_EQUAL_INT(0, state.current_month);
}

void test_IsComplete_NotFinished(void) {
    LoanDefinition loan = { .term_months = 12 };
    SimulationState state = {
        .current_balance = money_from_major(1000),
        .current_month = 5
    };
    TEST_ASSERT_FALSE(loan_simulation_is_complete(&loan, &state));
}

void test_IsComplete_BalanceZero(void) {
    LoanDefinition loan = { .term_months = 12 };
    SimulationState state = {
        .current_balance = MONEY_ZERO,
        .current_month = 5
    };
    TEST_ASSERT_TRUE(loan_simulation_is_complete(&loan, &state));
}

void test_IsComplete_TimeOver(void) {
    LoanDefinition loan = { .term_months = 12 };
    SimulationState state = {
        .current_balance = money_from_major(100),
        .current_month = 12
    };
    TEST_ASSERT_TRUE(loan_simulation_is_complete(&loan, &state));
}

void test_Step_StandardPayment(void) {
    LoanDefinition loan = {
        .principal = money_from_major(1000),
        .term_months = 2,
        .type = LOAN_EQUAL_INSTALLMENTS
    };

    Rate rates[2] = { create_rate(0.0), create_rate(0.0) };
    MarketScenario market = { .annual_rates = rates };

    SimulationConfig config = { .strategy = STRATEGY_REDUCE_TERM, .custom_payments = NULL };

    SimulationState state;
    loan_simulation_init(&state, loan.principal);

    Installment inst;

    FinanceErrorCode err = loan_simulation_step(&loan, &market, &config, &state, &inst);

    TEST_ASSERT_EQUAL(FINANCE_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT64(50000, inst.payment.value);
    TEST_ASSERT_EQUAL_INT64(50000, inst.balance.value);
    TEST_ASSERT_EQUAL_INT(1, state.current_month);
}

void test_Step_CustomPayment_Overpayment(void) {
    LoanDefinition loan = { .principal = money_from_major(1000), .term_months = 10, .type = LOAN_EQUAL_INSTALLMENTS };

    Rate rates[10] = {0}; // 0%
    MarketScenario market = { .annual_rates = rates };

    Money custom_payments[10] = {0};
    custom_payments[0] = money_from_major(800);

    SimulationConfig config = {
        .strategy = STRATEGY_REDUCE_TERM,
        .custom_payments = custom_payments
    };

    SimulationState state;
    loan_simulation_init(&state, loan.principal);
    Installment inst;

    FinanceErrorCode err = loan_simulation_step(&loan, &market, &config, &state, &inst);

    TEST_ASSERT_EQUAL(FINANCE_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT64(80000, inst.payment.value);
    TEST_ASSERT_EQUAL_INT64(20000, inst.balance.value);
}

void test_Run_ValidateInputs_NullPointers(void) {
    LoanDefinition loan = {0};
    MarketScenario market = {0};
    SimulationConfig config = {0};

    TEST_ASSERT_EQUAL(FINANCE_ERR_INVALID_ARGUMENT, run_loan_simulation(&loan, &market, &config, NULL));
}

void test_Run_ValidateInputs_InvalidPrincipal(void) {
    LoanDefinition loan = { .principal = money_from_major(-100), .term_months = 12 };
    MarketScenario market = {0};
    SimulationConfig config = {0};
    LoanSchedule result = {0};

    TEST_ASSERT_EQUAL(FINANCE_ERR_INVALID_PRINCIPAL, run_loan_simulation(&loan, &market, &config, &result));
}

void test_Run_ValidateInputs_InvalidRates(void) {
    LoanDefinition loan = { .principal = money_from_major(1000), .term_months = 1 };

    Rate rates[1];
    rates[0].value = -1.0;
    MarketScenario market = { .annual_rates = rates };

    SimulationConfig config = {0};
    LoanSchedule result = {0};

    TEST_ASSERT_EQUAL(FINANCE_ERR_INVALID_RATE, run_loan_simulation(&loan, &market, &config, &result));
}

void test_Run_HappyPath(void) {
    const int months = 6;
    Rate* rates = create_constant_rates(months, 0.05);
    MarketScenario market = { .annual_rates = rates };

    LoanDefinition loan = {
        .principal = money_from_major(10000),
        .term_months = months,
        .type = LOAN_EQUAL_INSTALLMENTS
    };

    SimulationConfig config = { .strategy = STRATEGY_REDUCE_TERM };
    LoanSchedule result;

    FinanceErrorCode err = run_loan_simulation(&loan, &market, &config, &result);

    TEST_ASSERT_EQUAL(FINANCE_SUCCESS, err);
    TEST_ASSERT_EQUAL_INT(months, result.count);

    TEST_ASSERT_TRUE(money_is_zero(result.items[months-1].balance));

    Money calc_total = money_add(money_from_major(10000), result.total_interest);
    TEST_ASSERT_EQUAL_INT64(calc_total.value, result.total_paid.value);

    free(rates);
    free_schedule(&result);
}

void run_loan_simulation_tests(void) {
    RUN_TEST(test_Init);
    RUN_TEST(test_IsComplete_NotFinished);
    RUN_TEST(test_IsComplete_BalanceZero);
    RUN_TEST(test_IsComplete_TimeOver);
    RUN_TEST(test_Step_StandardPayment);
    RUN_TEST(test_Step_CustomPayment_Overpayment);
    RUN_TEST(test_Run_ValidateInputs_NullPointers);
    RUN_TEST(test_Run_ValidateInputs_InvalidPrincipal);
    RUN_TEST(test_Run_ValidateInputs_InvalidRates);
    RUN_TEST(test_Run_HappyPath);
}
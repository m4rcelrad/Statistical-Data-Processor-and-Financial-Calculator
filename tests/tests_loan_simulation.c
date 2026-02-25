#include "unity.h"
#include "loan_simulation.h"
#include "money.h"
#include "loan_math.h"
#include <stdlib.h>

/**
 * @file tests_loan_simulation.c
 * @brief Unit tests for the step-by-step loan simulation logic.
 *
 * Validates the simulation state, completion conditions, monthly steps,
 * overpayment strategies, and edge-case handling during an entire simulation run.
 */

/**
 * @brief Helper function to generate an array of constant interest rates.
 * @param months The length of the array to allocate.
 * @param value The constant rate value to populate.
 * @return A dynamically allocated array of Rates.
 */
static Rate* create_constant_rates(const int months, const double value) {
    Rate* rates = malloc(sizeof(Rate) * months);
    for (int i = 0; i < months; i++) {
        rates[i] = create_rate(value);
    }
    return rates;
}

/**
 * @brief Tests the initialization of a simulation state.
 * Expected result: The initial balance matches the principal, payment history
 * is zeroed, and the current month counter starts at 0.
 */
void test_Init(void) {
    SimulationState state;
    Money principal = money_from_major(100000);

    loan_simulation_init(&state, principal);

    TEST_ASSERT_EQUAL_INT64(principal.value, state.current_balance.value);
    TEST_ASSERT_TRUE(money_is_zero(state.last_total_payment));
    TEST_ASSERT_EQUAL_INT(0, state.current_month);
}

/**
 * @brief Tests the completion logic when the loan is still ongoing.
 * Expected result: Returns false since the balance is positive and the term is not reached.
 */
void test_IsComplete_NotFinished(void) {
    LoanDefinition loan = { .term_months = 12 };
    SimulationState state = {
        .current_balance = money_from_major(1000),
        .current_month = 5
    };
    TEST_ASSERT_FALSE(loan_simulation_is_complete(&loan, &state));
}

/**
 * @brief Tests the completion logic when the loan balance drops to zero.
 * Expected result: Returns true, terminating the simulation early.
 */
void test_IsComplete_BalanceZero(void) {
    LoanDefinition loan = { .term_months = 12 };
    SimulationState state = {
        .current_balance = MONEY_ZERO,
        .current_month = 5
    };
    TEST_ASSERT_TRUE(loan_simulation_is_complete(&loan, &state));
}

/**
 * @brief Tests the completion logic when the maximum time term is reached.
 * Expected result: Returns true, ending the simulation due to timeout.
 */
void test_IsComplete_TimeOver(void) {
    LoanDefinition loan = { .term_months = 12 };
    SimulationState state = {
        .current_balance = money_from_major(100),
        .current_month = 12
    };
    TEST_ASSERT_TRUE(loan_simulation_is_complete(&loan, &state));
}

/**
 * @brief Tests a standard monthly step calculation without overpayments.
 * Expected result: The payment and remaining balance are updated appropriately,
 * and the month counter increments.
 */
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

/**
 * @brief Tests a monthly step calculation taking into account a custom overpayment.
 * Expected result: The larger custom payment is accepted, causing a faster reduction
 * of the principal balance.
 */
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
    TEST_ASSERT_EQUAL_INT64(90000, inst.payment.value);
    TEST_ASSERT_EQUAL_INT64(10000, inst.balance.value);
}

/**
 * @brief Tests full simulation handling of NULL pointers.
 * Expected result: Returns FINANCE_ERR_INVALID_ARGUMENT without causing a segmentation fault.
 */
void test_Run_ValidateInputs_NullPointers(void) {
    LoanDefinition loan = {0};
    MarketScenario market = {0};
    SimulationConfig config = {0};

    TEST_ASSERT_EQUAL(FINANCE_ERR_INVALID_ARGUMENT, run_loan_simulation(&loan, &market, &config, NULL));
}

/**
 * @brief Tests full simulation handling of an invalid principal amount.
 * Expected result: Returns FINANCE_ERR_INVALID_PRINCIPAL due to the negative balance.
 */
void test_Run_ValidateInputs_InvalidPrincipal(void) {
    LoanDefinition loan = { .principal = money_from_major(-100), .term_months = 12 };
    MarketScenario market = {0};
    SimulationConfig config = {0};
    LoanSchedule result = {0};

    TEST_ASSERT_EQUAL(FINANCE_ERR_INVALID_PRINCIPAL, run_loan_simulation(&loan, &market, &config, &result));
}

/**
 * @brief Tests full simulation handling of invalid negative interest rates.
 * Expected result: Returns FINANCE_ERR_INVALID_RATE, blocking unrealistic market scenarios.
 */
void test_Run_ValidateInputs_InvalidRates(void) {
    LoanDefinition loan = { .principal = money_from_major(1000), .term_months = 1 };

    Rate rates[1];
    rates[0].value = -1.0;
    MarketScenario market = { .annual_rates = rates };

    SimulationConfig config = {0};
    LoanSchedule result = {0};

    TEST_ASSERT_EQUAL(FINANCE_ERR_INVALID_RATE, run_loan_simulation(&loan, &market, &config, &result));
}

/**
 * @brief Tests a complete end-to-end execution of a standard loan.
 * Expected result: The schedule successfully runs through all months, closing out
 * the balance perfectly to zero and tallying the correct totals.
 */
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

/**
 * @brief Test runner for the loan simulation module.
 */
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
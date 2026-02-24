#include "loan_simulation.h"

#include <limits.h>
#include <math.h>
#include <stdlib.h>

#include "loan_math.h"
#include "money.h"

/** * @brief Maximum allowed duration of a loan in months (100 years).
 */
#define MAX_LOAN_MONTHS 1200

/**
 * @brief Validates the primary inputs before starting the simulation.
 * @param loan Pointer to the loan definition.
 * @param market Pointer to the market scenario.
 * @param out_schedule Pointer to the output schedule.
 * @return FINANCE_SUCCESS if inputs are valid, or an error code describing the issue.
 */
static FinanceErrorCode validate_inputs(const LoanDefinition *loan,
                                        const MarketScenario *market,
                                        LoanSchedule *out_schedule)
{
    if (!out_schedule || !loan)
        return FINANCE_ERR_INVALID_ARGUMENT;

    if (!money_is_positive(loan->principal))
        return FINANCE_ERR_INVALID_PRINCIPAL;
    if (loan->term_months <= 0 || loan->term_months > MAX_LOAN_MONTHS)
        return FINANCE_ERR_INVALID_MONTHS;
    if (!market->annual_rates)
        return FINANCE_ERR_NULL_RATES;

    for (int i = 0; i < loan->term_months; i++) {
        if (!isfinite(market->annual_rates[i].value) || market->annual_rates[i].value < 0.0L) {
            return FINANCE_ERR_INVALID_RATE;
        }
    }
    return FINANCE_SUCCESS;
}

/**
 * @brief Validates the simulation configuration.
 * @param config Pointer to the simulation config.
 * @return FINANCE_SUCCESS if valid, or FINANCE_ERR_INVALID_ARGUMENT otherwise.
 */
static FinanceErrorCode validate_config(const SimulationConfig *config)
{
    if (!config)
        return FINANCE_ERR_INVALID_ARGUMENT;

    if (config->strategy != STRATEGY_REDUCE_TERM && config->strategy != STRATEGY_REDUCE_INSTALLMENT)
        return FINANCE_ERR_INVALID_ARGUMENT;

    return FINANCE_SUCCESS;
}

/**
 * @brief Determines the actual payment amount for the current month.
 * * Takes into account custom overpayments or the selected strategy (reducing term
 * vs. reducing installments) to decide the final amount to be paid.
 *
 * @param config Pointer to the simulation config.
 * @param state Pointer to the active simulation state.
 * @param required_payment The calculated baseline payment.
 * @param interest Accrued interest for the month.
 * @param out_final_payment Pointer to store the finalized payment amount.
 * @return FINANCE_SUCCESS or an error code on invalid overpayment scenarios.
 */
static FinanceErrorCode determine_actual_payment(const SimulationConfig *config,
                                                 const SimulationState *state,
                                                 const Money required_payment,
                                                 const Money interest,
                                                 Money *out_final_payment)
{
    const Money custom_amount =
        config->custom_payments ? config->custom_payments[state->current_month] : MONEY_ZERO;

    if (money_is_positive(custom_amount)) {
        const Money max_allowed = money_add(state->current_balance, interest);
        if (money_gt(custom_amount, max_allowed))
            return FINANCE_ERR_PAYMENT_TOO_LARGE;
        if (money_lt(custom_amount, interest))
            return FINANCE_ERR_NEGATIVE_AMORTIZATION;
        *out_final_payment = custom_amount;
    } else {
        Money payment;
        if (config->strategy == STRATEGY_REDUCE_INSTALLMENT) {
            payment = required_payment;
        } else {
            const Money target =
                state->current_month == 0 ? required_payment : state->last_total_payment;
            payment = money_gt(required_payment, target) ? required_payment : target;
        }

        if (money_lte(payment, interest)) {
            const Money one_unit = {1};
            payment = money_add(interest, one_unit);
        }
        *out_final_payment = payment;
    }
    return FINANCE_SUCCESS;
}

/**
 * @brief Appends a single installment record into the output schedule.
 * @param schedule Pointer to the schedule holding the results.
 * @param max_capacity Maximum number of items the schedule can hold.
 * @param installment Pointer to the installment to append.
 * @return FINANCE_SUCCESS on success, or an overflow error if capacity is exceeded.
 */
static FinanceErrorCode
append_result(LoanSchedule *schedule, const int max_capacity, const Installment *installment)
{
    if (schedule->count >= max_capacity) {
        return FINANCE_ERR_NUMERIC_OVERFLOW;
    }
    schedule->items[schedule->count] = *installment;
    schedule->count++;
    return FINANCE_SUCCESS;
}

/**
 * @brief Updates the global totals (interest and paid amount) in the schedule.
 * @param schedule Pointer to the schedule to update.
 * @param installment Pointer to the latest processed installment.
 * @return FINANCE_SUCCESS on success, or an overflow error if values exceed the limit.
 */
static FinanceErrorCode update_totals(LoanSchedule *schedule, const Installment *installment)
{
    if (schedule->total_interest.value > LLONG_MAX - installment->interest.value)
        return FINANCE_ERR_NUMERIC_OVERFLOW;
    if (schedule->total_paid.value > LLONG_MAX - installment->payment.value)
        return FINANCE_ERR_NUMERIC_OVERFLOW;

    schedule->total_interest = money_add(schedule->total_interest, installment->interest);
    schedule->total_paid = money_add(schedule->total_paid, installment->payment);

    return FINANCE_SUCCESS;
}

void loan_simulation_init(SimulationState *state, Money principal)
{
    state->current_balance = principal;
    state->last_total_payment = MONEY_ZERO;
    state->current_month = 0;
}

bool loan_simulation_is_complete(const LoanDefinition *loan, const SimulationState *state)
{
    if (!loan || !state)
        return true;

    if (money_lte(state->current_balance, MONEY_ZERO))
        return true;

    if (state->current_month >= loan->term_months)
        return true;

    return false;
}

FinanceErrorCode loan_simulation_step(const LoanDefinition *loan,
                                      const MarketScenario *market,
                                      const SimulationConfig *config,
                                      SimulationState *state,
                                      Installment *out_installment)
{
    if (!loan || !market || !config || !state || !out_installment)
        return FINANCE_ERR_INVALID_ARGUMENT;

    if (state->current_month >= loan->term_months)
        return FINANCE_ERR_INVALID_MONTHS;

    const Rate current_rate = market->annual_rates[state->current_month];

    const Money interest = calculate_monthly_interest(state->current_balance, current_rate);

    Money required_payment = MONEY_ZERO;
    FinanceErrorCode err =
        calculate_baseline_payment(loan, market, state, interest, &required_payment);
    if (err != FINANCE_SUCCESS)
        return err;

    Money final_payment = MONEY_ZERO;
    err = determine_actual_payment(config, state, required_payment, interest, &final_payment);
    if (err != FINANCE_SUCCESS)
        return err;

    Money final_capital = money_sub(final_payment, interest);

    if (money_gt(final_capital, state->current_balance)) {
        final_capital = state->current_balance;
    }

    bool is_last_month = (state->current_month == loan->term_months - 1);
    Money potential_balance = money_sub(state->current_balance, final_capital);

    if (is_last_month || money_lte(potential_balance, MONEY_ZERO)) {
        final_capital = state->current_balance;
        final_payment = money_add(final_capital, interest);
    }

    state->last_total_payment = final_payment;
    state->current_balance = money_sub(state->current_balance, final_capital);

    if (money_lt(state->current_balance, MONEY_ZERO))
        state->current_balance = MONEY_ZERO;

    out_installment->capital = final_capital;
    out_installment->interest = interest;
    out_installment->payment = final_payment;
    out_installment->balance = state->current_balance;

    state->current_month++;

    return FINANCE_SUCCESS;
}

FinanceErrorCode run_loan_simulation(const LoanDefinition *loan,
                                     const MarketScenario *market,
                                     const SimulationConfig *config,
                                     LoanSchedule *out_result)
{
    FinanceErrorCode err = validate_inputs(loan, market, out_result);
    if (err != FINANCE_SUCCESS)
        return err;

    err = validate_config(config);
    if (err != FINANCE_SUCCESS)
        return err;

    out_result->items = calloc(loan->term_months, sizeof(Installment));
    if (!out_result->items)
        return FINANCE_ERR_ALLOCATION_FAILED;

    out_result->count = 0;
    out_result->total_interest = MONEY_ZERO;
    out_result->total_paid = MONEY_ZERO;

    SimulationState state;
    loan_simulation_init(&state, loan->principal);

    for (int i = 0; i < loan->term_months; i++) {
        Installment current_inst = {0};

        err = loan_simulation_step(loan, market, config, &state, &current_inst);
        if (err != FINANCE_SUCCESS) {
            free_schedule(out_result);
            return err;
        }

        err = append_result(out_result, loan->term_months, &current_inst);
        if (err != FINANCE_SUCCESS) {
            free_schedule(out_result);
            return err;
        }

        err = update_totals(out_result, &current_inst);
        if (err != FINANCE_SUCCESS) {
            free_schedule(out_result);
            return err;
        }

        if (loan_simulation_is_complete(loan, &state)) {
            break;
        }
    }

    return FINANCE_SUCCESS;
}

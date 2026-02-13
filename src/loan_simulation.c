#include "loan_simulation.h"
#include "loan_math.h"
#include "money.h"
#include <limits.h>
#include <math.h>
#include <stdlib.h>

#define MAX_LOAN_MONTHS 1200

static FinanceErrorCode validate_inputs(const LoanDefinition *loan,
                                        const MarketScenario *market,
                                        LoanSchedule *out_schedule) {
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

static FinanceErrorCode validate_config(const SimulationConfig *config) {
    if (!config)
        return FINANCE_ERR_INVALID_ARGUMENT;

    if (config->strategy != STRATEGY_REDUCE_TERM && config->strategy != STRATEGY_REDUCE_INSTALLMENT)
        return FINANCE_ERR_INVALID_ARGUMENT;

    return FINANCE_SUCCESS;
}

static FinanceErrorCode determine_actual_payment(const SimulationConfig *config,
                                                 const SimulationState *state,
                                                 const Money required_payment,
                                                 const Money interest,
                                                 Money *out_final_payment) {
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

static FinanceErrorCode
append_result(const LoanSchedule *schedule, const int index, const Installment *installment) {
    if (index < 0 || index >= schedule->count)
        return FINANCE_ERR_NUMERIC_OVERFLOW;
    schedule->items[index] = *installment;
    return FINANCE_SUCCESS;
}

static FinanceErrorCode update_totals(LoanSchedule *schedule, const Installment *installment) {
    if (schedule->total_interest.value > LLONG_MAX - installment->interest.value)
        return FINANCE_ERR_NUMERIC_OVERFLOW;
    if (schedule->total_paid.value > LLONG_MAX - installment->payment.value)
        return FINANCE_ERR_NUMERIC_OVERFLOW;

    schedule->total_interest = money_add(schedule->total_interest, installment->interest);
    schedule->total_paid = money_add(schedule->total_paid, installment->payment);

    return FINANCE_SUCCESS;
}

static FinanceErrorCode loan_step(const LoanDefinition *loan,
                                  const MarketScenario *market,
                                  const SimulationConfig *config,
                                  SimulationState *state,
                                  Installment *out_installment) {
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

    if (money_gt(final_capital, state->current_balance))
        final_capital = state->current_balance;

    const Money custom_amount =
        (config->custom_payments) ? config->custom_payments[state->current_month] : MONEY_ZERO;
    if (state->current_month == loan->term_months - 1 && money_is_zero(custom_amount))
        final_capital = state->current_balance;

    final_payment = money_add(final_capital, interest);

    state->last_total_payment = final_payment;

    state->current_balance = money_sub(state->current_balance, final_capital);
    if (money_lt(state->current_balance, MONEY_ZERO))
        state->current_balance = MONEY_ZERO;

    out_installment->capital = final_capital;
    out_installment->interest = interest;
    out_installment->payment = final_payment;
    out_installment->balance = state->current_balance;

    return FINANCE_SUCCESS;
}

FinanceErrorCode run_loan_simulation(const LoanDefinition *loan,
                                     const MarketScenario *market,
                                     const SimulationConfig *config,
                                     LoanSchedule *out_result) {
    FinanceErrorCode err = validate_inputs(loan, market, out_result);
    if (err != FINANCE_SUCCESS)
        return err;

    err = validate_config(config);
    if (err != FINANCE_SUCCESS)
        return err;

    out_result->items = calloc(loan->term_months, sizeof(Installment));
    if (!out_result->items)
        return FINANCE_ERR_ALLOCATION_FAILED;

    out_result->count = loan->term_months;
    out_result->total_interest = MONEY_ZERO;
    out_result->total_paid = MONEY_ZERO;

    SimulationState state = {.current_balance = loan->principal,
                             .last_total_payment = MONEY_ZERO,
                             .current_month = 0};

    for (int i = 0; i < loan->term_months; i++) {
        state.current_month = i;
        Installment current_inst = {0};

        err = loan_step(loan, market, config, &state, &current_inst);
        if (err != FINANCE_SUCCESS) {
            free_schedule(out_result);
            return err;
        }

        err = append_result(out_result, i, &current_inst);
        if (err != FINANCE_SUCCESS) {
            free_schedule(out_result);
            return err;
        }

        err = update_totals(out_result, &current_inst);
        if (err != FINANCE_SUCCESS) {
            free_schedule(out_result);
            return err;
        }

        if (money_is_zero(state.current_balance)) {
            out_result->count = i + 1;
            break;
        }
    }

    return FINANCE_SUCCESS;
}

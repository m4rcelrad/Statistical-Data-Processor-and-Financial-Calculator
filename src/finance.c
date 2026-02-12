#include <math.h>
#include <stdlib.h>
#include "finance.h"

#define MAX_LOAN_MONTHS 1200

Rate create_rate(const long double value) {
    Rate r;
    r.value = value;
    return r;
}

void free_schedule(LoanSchedule *schedule) {
    if (schedule->items) {
        free(schedule->items);
        schedule->items = NULL;
    }
    schedule->count = 0;
    schedule->total_interest = 0;
    schedule->total_paid = 0;
}

const char* finance_error_string(const FinanceErrorCode code) {
    switch (code) {
        case FINANCE_SUCCESS: return "Success";
        case FINANCE_ERR_INVALID_PRINCIPAL: return "Invalid principal amount";
        case FINANCE_ERR_INVALID_MONTHS: return "Invalid number of months";
        case FINANCE_ERR_NULL_RATES: return "Rates array is NULL";
        case FINANCE_ERR_INVALID_RATE: return "Invalid interest rate value";
        case FINANCE_ERR_ALLOCATION_FAILED: return "Memory allocation failed";
        case FINANCE_ERR_NEGATIVE_AMORTIZATION: return "Payment is smaller than accrued interest";
        case FINANCE_ERR_PAYMENT_TOO_LARGE: return "Custom payment exceeds loan balance significantly";
        case FINANCE_ERR_NUMERIC_OVERFLOW: return "Numeric overflow during calculation";
        default: return "Unknown error";
    }
}

static FinanceErrorCode validate_inputs(const LoanDefinition *loan, LoanSchedule *out_schedule) {
    if (!out_schedule) return FINANCE_ERR_ALLOCATION_FAILED;
    if (!loan) return FINANCE_ERR_INVALID_PRINCIPAL;

    if (loan->principal <= 0) return FINANCE_ERR_INVALID_PRINCIPAL;
    if (loan->term_months <= 0 || loan->term_months > MAX_LOAN_MONTHS) return FINANCE_ERR_INVALID_MONTHS;
    if (!loan->annual_rates) return FINANCE_ERR_NULL_RATES;

    for (int i = 0; i < loan->term_months; i++) {
        if (!isfinite(loan->annual_rates[i].value) || loan->annual_rates[i].value < 0.0L) {
            return FINANCE_ERR_INVALID_RATE;
        }
    }
    return FINANCE_SUCCESS;
}

static Money calculate_monthly_interest(const Money balance, const Rate current_rate) {
    if (current_rate.value == 0.0L) return 0;
    const long double factor = current_rate.value / 12.0L;
    return llroundl((long double)balance * factor);
}

static FinanceErrorCode calculate_annuity_pmt(const Money balance, const long double monthly_rate, const int remaining_months, Money *out_pmt) {
    *out_pmt = 0;
    if (balance <= 0) return FINANCE_SUCCESS;

    if (remaining_months <= 0) {
        *out_pmt = balance;
        return FINANCE_SUCCESS;
    }

    if (!isfinite(monthly_rate) || monthly_rate < 0.0L) return FINANCE_ERR_INVALID_RATE;

    if (monthly_rate == 0.0L) {
        *out_pmt = balance / remaining_months;
        return FINANCE_SUCCESS;
    }

    const long double factor = powl(1.0L + monthly_rate, remaining_months);

    if (!isfinite(factor)) return FINANCE_ERR_NUMERIC_OVERFLOW;
    if (factor - 1.0L == 0.0L) return FINANCE_ERR_NUMERIC_OVERFLOW;

    const long double exact = (long double)balance * monthly_rate * factor / (factor - 1.0L);

    if (!isfinite(exact)) return FINANCE_ERR_NUMERIC_OVERFLOW;

    if (exact > (long double)LLONG_MAX) return FINANCE_ERR_NUMERIC_OVERFLOW;

    *out_pmt = llroundl(exact);
    return FINANCE_SUCCESS;
}

static FinanceErrorCode calculate_baseline_payment(const LoanDefinition *loan, const SimulationState *state, const Money interest, Money *out_payment) {
    const int remaining_months = loan->term_months - state->current_month;
    const Rate current_rate = loan->annual_rates[state->current_month];
    const long double monthly_rate = current_rate.value / 12.0L;

    if (loan->type == LOAN_EQUAL_INSTALLMENTS) {
        const FinanceErrorCode err = calculate_annuity_pmt(state->current_balance, monthly_rate, remaining_months, out_payment);
        if (err != FINANCE_SUCCESS) return err;

        if (*out_payment < interest && remaining_months > 1) {
            *out_payment = interest + 1;
        }

    } else {
        const Money capital_part = state->current_balance / remaining_months;
        *out_payment = capital_part + interest;
    }
    return FINANCE_SUCCESS;
}

static FinanceErrorCode determine_actual_payment(const SimulationConfig *config, const SimulationState *state, const Money required_payment, const Money interest, Money *out_final_payment) {
    const Money custom_amount = config->custom_payments ? config->custom_payments[state->current_month] : 0;

    if (custom_amount > 0) {
        if (custom_amount > (state->current_balance + interest) * 2) return FINANCE_ERR_PAYMENT_TOO_LARGE;
        if (custom_amount < interest) return FINANCE_ERR_NEGATIVE_AMORTIZATION;
        *out_final_payment = custom_amount;
    } else {
        Money payment = 0;
        if (config->strategy == STRATEGY_REDUCE_INSTALLMENT) {
            payment = required_payment;
        } else {
            const Money target = state->current_month == 0 ? required_payment : state->last_total_payment;
            payment = required_payment > target ? required_payment : target;
        }

        if (payment <= interest) payment = interest + 1;
        *out_final_payment = payment;
    }
    return FINANCE_SUCCESS;
}

FinanceErrorCode run_loan_simulation(const LoanDefinition *loan, const SimulationConfig *config, LoanSchedule *out_result) {

    FinanceErrorCode err = validate_inputs(loan, out_result);
    if (err != FINANCE_SUCCESS) return err;

    out_result->items = calloc(loan->term_months, sizeof(Installment));
    if (!out_result->items) return FINANCE_ERR_ALLOCATION_FAILED;
    out_result->count = loan->term_months;
    out_result->total_interest = 0;
    out_result->total_paid = 0;

    SimulationState state = {
        .current_balance = loan->principal,
        .last_total_payment = 0,
        .current_month = 0
    };

    for (int i = 0; i < loan->term_months; i++) {
        state.current_month = i;
        const Rate current_rate = loan->annual_rates[i];

        const Money interest = calculate_monthly_interest(state.current_balance, current_rate);

        Money required_payment = 0;
        err = calculate_baseline_payment(loan, &state, interest, &required_payment);
        if (err != FINANCE_SUCCESS) {
            free_schedule(out_result);
            return err;
        }

        Money final_payment = 0;
        err = determine_actual_payment(config, &state, required_payment, interest, &final_payment);
        if (err != FINANCE_SUCCESS) {
            free_schedule(out_result);
            return err;
        }

        Money final_capital = final_payment - interest;

        if (final_capital > state.current_balance) final_capital = state.current_balance;

        const Money custom_amount = config->custom_payments ? config->custom_payments[i] : 0;
        if (i == loan->term_months - 1 && custom_amount == 0) final_capital = state.current_balance;

        final_payment = final_capital + interest;

        state.last_total_payment = final_payment;
        state.current_balance -= final_capital;
        if (state.current_balance < 0) state.current_balance = 0;

        out_result->items[i].capital = final_capital;
        out_result->items[i].interest = interest;
        out_result->items[i].payment = final_payment;
        out_result->items[i].balance = state.current_balance;

        if (out_result->total_interest > LLONG_MAX - interest || out_result->total_paid > LLONG_MAX - final_payment) {
            free_schedule(out_result);
            return FINANCE_ERR_NUMERIC_OVERFLOW;
        }

        out_result->total_interest += interest;
        out_result->total_paid += final_payment;

        if (state.current_balance == 0) {
            out_result->count = i + 1;
            break;
        }
    }

    return FINANCE_SUCCESS;
}
#include "loan_math.h"

#include <limits.h>
#include <math.h>
#include <stdlib.h>

#include "money.h"

Rate create_rate(const long double value)
{
    Rate r;
    r.value = value;
    return r;
}

void free_schedule(LoanSchedule *schedule)
{
    if (schedule->items) {
        free(schedule->items);
        schedule->items = NULL;
    }

    schedule->count = 0;
    schedule->total_interest = MONEY_ZERO;
    schedule->total_paid = MONEY_ZERO;
}

const char *finance_error_string(const FinanceErrorCode code)
{
    switch (code) {
    case FINANCE_SUCCESS:
        return "Success";
    case FINANCE_ERR_INVALID_PRINCIPAL:
        return "Invalid principal amount";
    case FINANCE_ERR_INVALID_MONTHS:
        return "Invalid number of months";
    case FINANCE_ERR_NULL_RATES:
        return "Rates array is NULL";
    case FINANCE_ERR_INVALID_RATE:
        return "Invalid interest rate value";
    case FINANCE_ERR_ALLOCATION_FAILED:
        return "Memory allocation failed";
    case FINANCE_ERR_NEGATIVE_AMORTIZATION:
        return "Payment is smaller than accrued interest";
    case FINANCE_ERR_PAYMENT_TOO_LARGE:
        return "Custom payment exceeds loan balance significantly";
    case FINANCE_ERR_NUMERIC_OVERFLOW:
        return "Numeric overflow during calculation";
    default:
        return "Unknown error";
    }
}

Money calculate_monthly_interest(const Money balance, const Rate current_rate)
{
    if (current_rate.value == 0.0L)
        return MONEY_ZERO;
    const long double factor = current_rate.value / 12.0L;
    return money_mul(balance, factor);
}

static FinanceErrorCode calculate_annuity_pmt(const Money balance,
                                              const long double monthly_rate,
                                              const int remaining_months,
                                              Money *out_pmt)
{
    *out_pmt = MONEY_ZERO;
    if (money_lte(balance, MONEY_ZERO))
        return FINANCE_SUCCESS;

    if (remaining_months <= 0) {
        *out_pmt = balance;
        return FINANCE_SUCCESS;
    }

    if (!isfinite(monthly_rate) || monthly_rate < 0.0L)
        return FINANCE_ERR_INVALID_RATE;

    if (monthly_rate == 0.0L) {
        *out_pmt = money_div(balance, remaining_months);
        return FINANCE_SUCCESS;
    }

    const long double factor = powl(1.0L + monthly_rate, remaining_months);

    if (!isfinite(factor))
        return FINANCE_ERR_NUMERIC_OVERFLOW;
    if (factor - 1.0L == 0.0L)
        return FINANCE_ERR_NUMERIC_OVERFLOW;

    const long double exact = (long double)balance.value * monthly_rate * factor / (factor - 1.0L);

    if (!isfinite(exact))
        return FINANCE_ERR_NUMERIC_OVERFLOW;
    if (exact > (long double)LLONG_MAX)
        return FINANCE_ERR_NUMERIC_OVERFLOW;

    out_pmt->value = llroundl(exact);
    return FINANCE_SUCCESS;
}

FinanceErrorCode calculate_baseline_payment(const LoanDefinition *loan,
                                            const MarketScenario *market,
                                            const SimulationState *state,
                                            const Money interest,
                                            Money *out_payment)
{
    const int remaining_months = loan->term_months - state->current_month;
    const Rate current_rate = market->annual_rates[state->current_month];
    const long double monthly_rate = current_rate.value / 12.0L;

    if (loan->type == LOAN_EQUAL_INSTALLMENTS) {
        const FinanceErrorCode err = calculate_annuity_pmt(
            state->current_balance, monthly_rate, remaining_months, out_payment);
        if (err != FINANCE_SUCCESS)
            return err;

        if (money_lt(*out_payment, interest) && remaining_months > 1) {
            const Money one_unit = {1};
            *out_payment = money_add(interest, one_unit);
        }
    } else {
        const Money capital_part = money_div(state->current_balance, remaining_months);
        *out_payment = money_add(capital_part, interest);
    }
    return FINANCE_SUCCESS;
}

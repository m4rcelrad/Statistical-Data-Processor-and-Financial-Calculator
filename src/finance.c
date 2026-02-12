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

    if (!isfinite(factor))
        return FINANCE_ERR_NUMERIC_OVERFLOW;

    if (factor - 1.0L == 0.0L) {
        *out_pmt = balance / remaining_months;
        return FINANCE_SUCCESS;
    }

    const long double exact = (long double)balance * monthly_rate / (factor - 1.0L);

    if (!isfinite(exact) || exact > (long double)LLONG_MAX || exact < 0.0L ) return FINANCE_ERR_NUMERIC_OVERFLOW;

    *out_pmt = llroundl(exact);

    return FINANCE_SUCCESS;

}

static int validate_inputs(const Money principal, const int months, const Rate *annual_rates, LoanSchedule *out_schedule) {
    if (!out_schedule) return FINANCE_ERR_INVALID_ARGUMENT;

    if (principal <= 0) return FINANCE_ERR_INVALID_PRINCIPAL;

    if (months <= 0 || months > MAX_LOAN_MONTHS) return FINANCE_ERR_INVALID_MONTHS;

    if (!annual_rates) return FINANCE_ERR_NULL_RATES;

    for (int i = 0; i < months; i++) {
        if (!isfinite(annual_rates[i].value) || annual_rates[i].value < 0.0L || annual_rates[i].value > 100.0L) return FINANCE_ERR_INVALID_RATE;
    }

    return FINANCE_SUCCESS;
}

static Money calculate_monthly_interest(const Money balance, const Rate current_rate) {
    if (current_rate.value == 0.0L) return 0;
    const long double factor = current_rate.value / 12.0L;
    return llroundl((long double)balance * factor);
}

static int calculate_baseline_payment(const LoanType type, const Money balance, const Rate rate, const int remaining_months, const Money interest, Money *out_payment) {
    if (type == LOAN_EQUAL_INSTALLMENTS) {
        Money pmt = 0;
        const long double factor = rate.value / 12.0L;
        const int err = calculate_annuity_pmt(balance, factor, remaining_months, &pmt);
        if (err != FINANCE_SUCCESS) return err;
        *out_payment = pmt;
        if (*out_payment < interest && remaining_months > 1) {
            *out_payment = interest + 1;
        }
    } else {
        *out_payment = balance / remaining_months + interest;
    }
    return FINANCE_SUCCESS;
}

static int determine_payment(const Money user_custom_amount, const Money required_payment, const Money interest, const Money last_total_payment, const Money current_balance, const OverpaymentStrategy strategy, const int month_index,Money *out_final_payment) {
    if (user_custom_amount > 0) {
        if (user_custom_amount > current_balance + interest) return FINANCE_ERR_PAYMENT_TOO_LARGE;
        if (user_custom_amount < interest) return FINANCE_ERR_NEGATIVE_AMORTIZATION;

        *out_final_payment = user_custom_amount;
    } else {
        Money payment = 0;
        if (strategy == STRATEGY_REDUCE_INSTALLMENT) {
            payment = required_payment;
        } else {
            const Money target = (month_index == 0) ? required_payment : last_total_payment;
            payment = (required_payment > target) ? required_payment : target;
        }

        if (payment <= interest) {
            payment = interest + 1;
        }
        *out_final_payment = payment;
    }
    return FINANCE_SUCCESS;
}

int calculate_dynamic_schedule(const Money principal, const Rate *annual_rates, const int months, const LoanType type, const Money *custom_payments, const OverpaymentStrategy strategy, LoanSchedule *out_schedule) {

    int error_result = validate_inputs(principal, months, annual_rates, out_schedule);
    if (error_result != FINANCE_SUCCESS) return error_result;

    if (type != LOAN_EQUAL_INSTALLMENTS && type != LOAN_DECREASING_INSTALLMENTS) return FINANCE_ERR_INVALID_ARGUMENT;
    if (strategy != STRATEGY_REDUCE_TERM && strategy != STRATEGY_REDUCE_INSTALLMENT) return FINANCE_ERR_INVALID_ARGUMENT;

    out_schedule->items = calloc(months, sizeof(Installment));
    if (!out_schedule->items) return FINANCE_ERR_ALLOCATION_FAILED;

    out_schedule->count = months;
    out_schedule->total_interest = 0;
    out_schedule->total_paid = 0;

    Money current_balance = principal;
    Money last_total_payment = 0;

    for (int i = 0; i < months; i++) {
        const Rate current_rate = annual_rates[i];

        const Money interest = calculate_monthly_interest(current_balance, current_rate);

        Money required_payment = 0;
        error_result = calculate_baseline_payment(type, current_balance, current_rate, months - i, interest, &required_payment);
        if (error_result != FINANCE_SUCCESS) {
            free_schedule(out_schedule);
            return error_result;
        }

        Money final_payment = 0;
        const Money custom_amount = (custom_payments) ? custom_payments[i] : 0;

        error_result = determine_payment(custom_amount, required_payment, interest, last_total_payment, current_balance, strategy, i, &final_payment);
        if (error_result != FINANCE_SUCCESS) {
            free_schedule(out_schedule);
            return error_result;
        }

        Money final_capital = final_payment - interest;

        if (final_capital > current_balance) final_capital = current_balance;
        if (i == months - 1 && custom_amount == 0) final_capital = current_balance;

        final_payment = final_capital + interest;
        last_total_payment = final_payment;

        current_balance -= final_capital;
        if (current_balance < 0) current_balance = 0;

        out_schedule->items[i].capital = final_capital;
        out_schedule->items[i].interest = interest;
        out_schedule->items[i].payment = final_payment;
        out_schedule->items[i].balance = current_balance;

        if (out_schedule->total_interest > LLONG_MAX - interest || out_schedule->total_paid > LLONG_MAX - final_payment) {
            free_schedule(out_schedule);
            return FINANCE_ERR_NUMERIC_OVERFLOW;
        }
        out_schedule->total_interest += interest;
        out_schedule->total_paid += final_payment;

        if (current_balance == 0) {
            out_schedule->count = i + 1;
            break;
        }
    }

    return FINANCE_SUCCESS;
}

int calculate_schedule(const Money principal, const Rate annual_rate, const int months, const LoanType type, LoanSchedule *out_schedule) {
    if (!out_schedule) return FINANCE_ERR_ALLOCATION_FAILED;

    if (months <= 0 || months > MAX_LOAN_MONTHS) return FINANCE_ERR_INVALID_MONTHS;

    Rate *rates = malloc(sizeof(Rate) * months);
    if (!rates) return FINANCE_ERR_ALLOCATION_FAILED;

    for(int i=0; i<months; i++) {
        rates[i] = annual_rate;
    }

    const int result = calculate_dynamic_schedule(principal, rates, months, type, NULL, STRATEGY_REDUCE_TERM, out_schedule);
    free(rates);
    return result;
}

const char* finance_error_string(const FinanceErrorCode code)
{
    switch (code)
    {
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
            return "Custom payment exceeds allowed limit";
        case FINANCE_ERR_NUMERIC_OVERFLOW:
            return "Numeric overflow during calculation";
        default:
            return "Unknown finance error";
    }
}


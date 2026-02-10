#include <math.h>
#include <stdlib.h>
#include "finance.h"

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

static Money calculate_annuity_pmt(const Money balance, const long double monthly_rate, const int remaining_months) {
    if (balance <= 0) return 0;
    if (remaining_months <= 0) return balance;

    if (monthly_rate == 0.0L) return balance / remaining_months;

    const long double factor = powl(1.0L + monthly_rate, remaining_months);
    if (factor - 1.0L == 0.0L) return balance / remaining_months;

    const long double exact = (long double)balance * monthly_rate * factor / (factor - 1.0L);
    return llroundl(exact);
}

LoanSchedule calculate_dynamic_schedule(const Money principal, const Rate *annual_rates, const int months, const LoanType type, const Money *custom_payments, const OverpaymentStrategy strategy) {
    LoanSchedule schedule = {0};

    if (principal <= 0 || months <= 0 || !annual_rates) return schedule;

    schedule.items = calloc(months, sizeof(Installment));
    if (!schedule.items) return schedule;

    schedule.count = months;
    Money current_balance = principal;

    Money last_total_payment = 0;

    for (int i = 0; i < months; i++) {
        const Rate current_rate = annual_rates[i];
        const long double monthly_factor = current_rate.value / 12.0L;
        const int zero_interest = (current_rate.value == 0.0L);
        const int remaining_months = months - i;

        Money interest = 0;
        if (!zero_interest) {
            interest = llroundl((long double)current_balance * monthly_factor);
        }

        Money required_total_payment = 0;

        if (type == LOAN_EQUAL_INSTALLMENTS) {
            required_total_payment = calculate_annuity_pmt(current_balance, monthly_factor, remaining_months);
            if (required_total_payment < interest) required_total_payment = interest;


        } else {
            Money required_capital_part = current_balance / remaining_months;
            required_total_payment = required_capital_part + interest;
        }

        Money final_payment = 0;
        Money final_capital = 0;

        const Money user_custom_amount = (custom_payments != NULL) ? custom_payments[i] : 0;

        if (user_custom_amount > 0) {
            final_payment = user_custom_amount;
            if (final_payment <= interest) final_payment = interest + 1;
            final_capital = final_payment - interest;
        }
        else {
            if (strategy == STRATEGY_REDUCE_INSTALLMENT) {
                final_payment = required_total_payment;
            }
            else {
                Money target_payment = last_total_payment;
                if (i == 0) target_payment = required_total_payment;

                if (required_total_payment > target_payment) {
                    final_payment = required_total_payment;
                } else {
                    final_payment = target_payment;
                }
            }
            final_capital = final_payment - interest;
        }

        if (final_capital > current_balance) final_capital = current_balance;
        if (i == months - 1 && user_custom_amount == 0) final_capital = current_balance;

        final_payment = final_capital + interest;

        last_total_payment = final_payment;

        current_balance -= final_capital;
        if (current_balance < 0) current_balance = 0;

        schedule.items[i].capital = final_capital;
        schedule.items[i].interest = interest;
        schedule.items[i].payment = final_payment;
        schedule.items[i].balance = current_balance;

        schedule.total_interest += interest;
        schedule.total_paid += final_payment;

        if (current_balance == 0) {
            schedule.count = i + 1;
            break;
        }
    }

    return schedule;
}

LoanSchedule calculate_schedule(const Money principal, const Rate annual_rate, const int months, const LoanType type) {
    Rate *rates = malloc(sizeof(Rate) * months);
    if (!rates) return (LoanSchedule){0};

    for(int i=0; i<months; i++) {
        rates[i] = annual_rate;
    }

    const LoanSchedule schedule = calculate_dynamic_schedule(principal, rates, months, type, NULL, STRATEGY_REDUCE_TERM);

    free(rates);
    return schedule;
}

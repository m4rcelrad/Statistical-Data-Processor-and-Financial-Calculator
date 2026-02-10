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

LoanSchedule calculate_dynamic_schedule(const Money principal, const Rate annual_rate, const int months, const LoanType type, const Money *custom_payments) {
    LoanSchedule schedule = {0};

    if (principal <= 0 || months <= 0 || annual_rate.value < 0.0L) return schedule;

    schedule.items = calloc(months, sizeof(Installment));
    if (!schedule.items) return schedule;

    schedule.count = months;

    Money current_balance = principal;

    const long double monthly_rate_factor = annual_rate.value / 12.0L;
    const int zero_interest = (annual_rate.value == 0.0L);

    Money base_fixed_installment = 0;

    if (type == LOAN_EQUAL_INSTALLMENTS && !zero_interest) {
        const long double factor = powl(1.0L + monthly_rate_factor, months);
        if (factor - 1.0L != 0.0L) {
            const long double exact = (long double)principal * monthly_rate_factor * factor / (factor - 1.0L);
            base_fixed_installment = llroundl(exact);
        }
    }

    for (int i = 0; i < months; i++) {
        Money interest = 0;

        if (!zero_interest) {
            interest = llroundl((long double)current_balance * monthly_rate_factor);
        }

        Money payment = 0;
        Money capital = 0;

        const Money user_custom_amount = (custom_payments != NULL) ? custom_payments[i] : 0;

        if (user_custom_amount > 0) {
            payment = user_custom_amount;
            if (payment <= interest) payment = interest + 1;
            capital = payment - interest;
        }
        else {
            if (type == LOAN_EQUAL_INSTALLMENTS && !zero_interest) {
                payment = base_fixed_installment;
                capital = payment - interest;
            } else {
                const int remaining_months = months - i;
                capital = current_balance / remaining_months;
            }
        }

        if (capital > current_balance) {
            capital = current_balance;
        }

        if (i == months - 1 && user_custom_amount == 0) {
            capital = current_balance;
        }

        payment = capital + interest;

        current_balance -= capital;
        if (current_balance < 0) current_balance = 0;

        schedule.items[i].capital = capital;
        schedule.items[i].interest = interest;
        schedule.items[i].payment = payment;
        schedule.items[i].balance = current_balance;

        schedule.total_interest += interest;
        schedule.total_paid += payment;

        if (current_balance == 0) {
            schedule.count = i + 1;
            break;
        }
    }

    return schedule;
}
LoanSchedule calculate_schedule(const Money principal, const Rate annual_rate, const int months, const LoanType type) {
    return calculate_dynamic_schedule(principal, annual_rate, months, type, NULL);
}

#include <math.h>
#include <stdlib.h>
#include "finance.h"

#define EPS 1e-12

static long double round2(const long double value) {
    return roundl(value * 100.0L) / 100.0L;
}

static int is_zero(const long double value) {
    return fabsl(value) < EPS;
}

void free_schedule(LoanSchedule *schedule) {
    if (schedule->items) {
        free(schedule->items);
        schedule->items = NULL;
    }
    schedule->count = 0;
    schedule->total_interest = 0.0L;
    schedule->total_paid = 0.0L;
}

LoanSchedule calculate_dynamic_schedule(const long double principal, const long double annual_rate, const int months, const LoanType type, const long double *custom_payments) {
    LoanSchedule schedule = {0};

    if (principal <= 0.0L || months <= 0 || annual_rate < 0.0L) return schedule;

    schedule.items = calloc(months, sizeof(Installment));
    if (!schedule.items) return schedule;

    schedule.count = months;

    long double current_balance = round2(principal);
    const long double monthly_rate = annual_rate / 12.0L;
    const int is_zero_interest_flag = is_zero(monthly_rate);

    long double base_fixed_installment = 0.0L;

    if (type == LOAN_EQUAL_INSTALLMENTS && !is_zero_interest_flag) {
        const long double factor = powl(1.0L + monthly_rate, months);
        if (!is_zero(factor - 1.0L)) {
            const long double exact = principal * monthly_rate * factor / (factor - 1.0L);
            base_fixed_installment = round2(exact);
        }
    } else {
        base_fixed_installment = round2(principal / months);
    }

    for (int i = 0; i < months; i++) {
        long double interest = 0.0L;
        if (!is_zero_interest_flag) interest = round2(current_balance * monthly_rate);

        long double payment, capital;

        const long double user_custom_amount = (custom_payments != NULL) ? custom_payments[i] : 0.0L;

        if (user_custom_amount > 0.0L) {
            payment = round2(user_custom_amount);
            if (payment <= interest) payment = interest + 0.01L;
            capital = round2(payment - interest);
        } else {
            if (type == LOAN_EQUAL_INSTALLMENTS && !is_zero_interest_flag) {
                payment = base_fixed_installment;
                capital = round2(payment - interest);
            } else {
                const int remaining_months = months - i;
                capital = round2(current_balance / (long double)remaining_months);

                if (i == months - 1) capital = current_balance;

            }
        }

        if (capital > current_balance)
            capital = current_balance;

        if (i == months - 1) capital = current_balance;
        payment = round2(capital + interest);

        current_balance = round2(current_balance - capital);


        if (current_balance < 0.0L) current_balance = 0.0L;

        schedule.items[i].capital = capital;
        schedule.items[i].interest = interest;
        schedule.items[i].payment = payment;
        schedule.items[i].balance = current_balance;

        schedule.total_interest += interest;
        schedule.total_paid += payment;

        if (is_zero(current_balance)) {
            schedule.count = i + 1;
            break;
        }
    }

    schedule.total_interest = round2(schedule.total_interest);
    schedule.total_paid = round2(schedule.total_paid);

    return schedule;
}


LoanSchedule calculate_schedule(const long double principal, const long double annual_rate, const int months, const LoanType type) {
    return calculate_dynamic_schedule(principal, annual_rate, months, type, NULL);
}

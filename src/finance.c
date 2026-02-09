#include <stdio.h>
#include <math.h>

#include "finance.h"

#include <stdlib.h>

#define EPS 1e-12

static long double round2(const long double value) {
    return roundl(value * 100.0L) / 100.0L;
}

static int is_zero(const long double value) {
    return  fabsl(value) < EPS;
}

static void print_schedule(const LoanSchedule *schedule) {
    printf("\nLoan Schedule:\n");
    printf("-------------------------------------------------------------------\n");
    printf("| %3s | %12s | %12s | %12s | %12s |\n", "No.", "Principal", "Interest", "Payment", "Balance");
    printf("-------------------------------------------------------------------\n");

    for (int i = 0; i < schedule->count; i++) {
        printf("| %3d | %12.2Lf | %12.2Lf | %12.2Lf | %12.2Lf |\n",
               i + 1,
               schedule->items[i].capital,
               schedule->items[i].interest,
               schedule->items[i].payment,
               schedule->items[i].balance);
    }

    printf("-------------------------------------------------------------------\n");
    printf("SUMMARY:\n");
    printf("Total Principal Paid: %12.2Lf\n", schedule->total_paid - schedule->total_interest);
    printf("Total Interest Cost:  %12.2Lf\n", schedule->total_interest);
    printf("Total Amount Paid:    %12.2Lf\n", schedule->total_paid);
    printf("-------------------------------------------------------------------\n");
}

static void free_schedule(LoanSchedule *schedule) {
    if (schedule->items) {
        free(schedule->items);
        schedule->items = NULL;
    }
    schedule->count = 0;
    schedule->total_interest = 0.0L;
    schedule->total_paid = 0.0L;
}

static LoanSchedule calculate_schedule(const long double principal, const long double annual_rate, const int months, const LoanType type) {

    LoanSchedule schedule = {0};

    if (principal <= 0.0L || months <= 0 || annual_rate < 0.0L) return schedule;

    schedule.items = calloc(months, sizeof(Installment));
    if (!schedule.items) return schedule;

    schedule.count = months;

    long double current_balance = principal;
    const long double monthly_rate = annual_rate / 12.0L;

    const int is_zero_interest_flag = is_zero(monthly_rate);

    long double fixed_installment_exact = 0.0L;
    long double fixed_capital_part_exact = 0.0L;

    if (type == LOAN_EQUAL_INSTALLMENTS && !is_zero_interest_flag) {
        const long double factor = powl(1.0L + monthly_rate, months);
        if (!is_zero(factor - 1.0L)) {
            fixed_installment_exact = principal * monthly_rate * factor / (factor - 1.0L);
        }
    } else {
        fixed_capital_part_exact = principal / months;
    }

    for (int i = 0; i < months; i++) {
        long double interest_exact = 0.0L;
        long double capital_exact, payment_exact;

        if (!is_zero_interest_flag) interest_exact = current_balance * monthly_rate;

        if (type == LOAN_EQUAL_INSTALLMENTS && !is_zero_interest_flag) {
            payment_exact = fixed_installment_exact;
            capital_exact = round2(payment_exact - interest_exact);
        } else {
            capital_exact = fixed_capital_part_exact;

            if (i == months - 1) capital_exact = current_balance;

            payment_exact = capital_exact + interest_exact;
        }

        if (capital_exact > current_balance) {
            capital_exact = current_balance;
            payment_exact = capital_exact + interest_exact;
        }
        current_balance -= capital_exact;
        if (current_balance < 0.0L) current_balance = 0.0L;

        const long double interest = round2(interest_exact);
        const long double capital = round2(capital_exact);
        const long double payment = round2(payment_exact);
        const long double balance = round2(current_balance);

        schedule.items[i].capital = capital;
        schedule.items[i].interest = interest;
        schedule.items[i].payment = payment;
        schedule.items[i].balance = balance;

        schedule.total_interest += interest;
        schedule.total_paid += payment;

        if (is_zero(current_balance) && i < months - 1) {
            schedule.count = i + 1;
            break;
        }
    }

    schedule.total_interest = round2(schedule.total_interest);
    schedule.total_paid = round2(schedule.total_paid);

    return schedule;
}

void calculate_and_print_schedule(const long double principal, const long double annual_rate, const int months, const LoanType type) {

    LoanSchedule schedule = calculate_schedule(principal, annual_rate, months, type);

    if (schedule.items) {
        print_schedule(&schedule);
        free_schedule(&schedule);
    } else {
        printf("Error: Memory allocation failed or invalid input.\n");
    }
}

LoanSchedule calculate_dynamic_schedule(const long double principal, const long double annual_rate, const int months, const LoanType type, const long double *custom_payments) {
    LoanSchedule schedule = {0};

    if (principal <= 0.0L || months <= 0 || annual_rate < 0.0L) return schedule;

    schedule.items = calloc(months, sizeof(Installment));
    if (!schedule.items) return schedule;

    schedule.count = months;

    long double current_balance = principal;
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

        const long double user_custom_amount = (custom_payments != NULL) ? custom_payments[i] : 0.0;

        if (user_custom_amount > EPS) {
            payment = round2(user_custom_amount);

            if (payment <= interest)
                payment = interest + 0.01L;

            capital = round2(payment - interest);
        } else {
            if (type == LOAN_EQUAL_INSTALLMENTS && !is_zero_interest_flag) {
                payment = base_fixed_installment;
                capital = round2(payment - interest);
            } else {
                const int remaining_months = months - i;
                capital = round2(current_balance / (long double)remaining_months);

                if (i == months - 1) capital = current_balance;
                payment = round2(capital + interest);
            }

        }

        if (capital > current_balance) {
            capital = current_balance;
            payment = round2(capital + interest);
        }
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

void calculate_and_print_dynamic_schedule(const long double principal, const long double annual_rate, const int months, const LoanType type, const long double *custom_payments) {
    LoanSchedule schedule = calculate_dynamic_schedule(principal, annual_rate, months, type, custom_payments);

    if (schedule.items) {
        printf("\n=== DYNAMIC SCHEDULE ===\n");
        print_schedule(&schedule);
        free_schedule(&schedule);
    } else {
        printf("Error: Memory allocation failed or invalid input.\n");
    }
}


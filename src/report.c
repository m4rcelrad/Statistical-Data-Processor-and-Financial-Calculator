#include "report.h"

#include <stdio.h>
#include <stdlib.h>

#include "money.h"

/**
 * @brief Formats and directly writes a monetary value to a file stream.
 * @param stream The output file stream (e.g., stdout or a file pointer).
 * @param amount The internal Money structure to format.
 */
static void fprint_money(FILE *stream, const Money amount)
{
    const long long major = amount.value / CURRENCY_SCALE;
    const long long minor = llabs(amount.value % CURRENCY_SCALE);
    fprintf(stream, "%lld.%02lld", major, minor);
}

/**
 * @brief Formats a monetary value as a string and stores it in a buffer.
 * @param buffer The character array to store the formatted output.
 * @param amount The internal Money structure to format.
 */
static void format_money_str(char *buffer, const Money amount)
{
    const long long major = amount.value / CURRENCY_SCALE;
    const long long minor = llabs(amount.value % CURRENCY_SCALE);
    sprintf(buffer, "%lld.%02lld", major, minor);
}

void print_report_header(void)
{
    printf("\nLoan Schedule:\n");
    printf("-------------------------------------------------------------------\n");
    printf("| %3s | %12s | %12s | %12s | %12s |\n",
           "No.",
           "Principal",
           "Interest",
           "Payment",
           "Balance");
    printf("-------------------------------------------------------------------\n");
}

void print_report_row(const int month_idx, const Installment *inst)
{
    char buf[64];
    printf("| %3d | ", month_idx + 1);

    format_money_str(buf, inst->capital);
    printf("%12s | ", buf);

    format_money_str(buf, inst->interest);
    printf("%12s | ", buf);

    format_money_str(buf, inst->payment);
    printf("%12s | ", buf);

    format_money_str(buf, inst->balance);
    printf("%12s |\n", buf);
}

void print_report_summary(const Money total_interest, const Money total_paid)
{
    char buf[64];
    printf("-------------------------------------------------------------------\n");

    const Money total_principal = money_sub(total_paid, total_interest);
    format_money_str(buf, total_principal);
    printf("Total Principal Paid: %15s\n", buf);

    format_money_str(buf, total_interest);
    printf("Total Interest Cost:  %15s\n", buf);

    format_money_str(buf, total_paid);
    printf("Total Amount Paid:    %15s\n", buf);
    printf("-------------------------------------------------------------------\n");
}

void print_schedule_to_console(const LoanSchedule *schedule)
{
    if (!schedule)
        return;

    print_report_header();

    for (int i = 0; i < schedule->count; i++) {
        print_report_row(i, &schedule->items[i]);
    }

    print_report_summary(schedule->total_interest, schedule->total_paid);
}

int save_schedule_to_csv(const LoanSchedule *schedule, const char *filename)
{
    if (!schedule || !schedule->items)
        return -1;

    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error: Failed to open file for writing.");
        return -1;
    }

    fprintf(file, "Month;Principal;Interest;Payment;Balance\n");

    for (int i = 0; i < schedule->count; i++) {
        fprintf(file, "%d;", i + 1);
        fprint_money(file, schedule->items[i].capital);
        fprintf(file, ";");
        fprint_money(file, schedule->items[i].interest);
        fprintf(file, ";");
        fprint_money(file, schedule->items[i].payment);
        fprintf(file, ";");
        fprint_money(file, schedule->items[i].balance);
        fprintf(file, "\n");
    }

    fprintf(file, ";;;;\n");
    fprintf(file, "SUMMARY;;;;\n");
    fprintf(file, "Total Interest;");
    fprint_money(file, schedule->total_interest);
    fprintf(file, ";;;\n");
    fprintf(file, "Total Paid;");
    fprint_money(file, schedule->total_paid);
    fprintf(file, ";;;\n");

    fclose(file);
    printf("Successfully saved report to: %s\n", filename);
    return 0;
}

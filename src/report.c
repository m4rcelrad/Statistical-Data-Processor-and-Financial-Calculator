#include <stdio.h>
#include <stdlib.h>
#include "report.h"

static void fprint_money(FILE *stream, const Money amount) {
    const long long major = amount / CURRENCY_SCALE;
    const long long minor = llabs(amount % CURRENCY_SCALE);
    fprintf(stream, "%lld.%02lld", major, minor);
}

static void format_money_str(char *buffer, Money amount) {
    const long long major = amount / CURRENCY_SCALE;
    const long long minor = llabs(amount % CURRENCY_SCALE);
    sprintf(buffer, "%lld.%02lld", major, minor);
}

void print_schedule_to_console(const LoanSchedule *schedule) {
    printf("\nLoan Schedule:\n");
    printf("-------------------------------------------------------------------\n");
    printf("| %3s | %12s | %12s | %12s | %12s |\n", "No.", "Principal", "Interest", "Payment", "Balance");
    printf("-------------------------------------------------------------------\n");

    char buf[64];

    for (int i = 0; i < schedule->count; i++) {
        printf("| %3d | ", i + 1);
        format_money_str(buf, schedule->items[i].capital);
        printf("%12s | ", buf);

        format_money_str(buf, schedule->items[i].interest);
        printf("%12s | ", buf);

        format_money_str(buf, schedule->items[i].payment);
        printf("%12s | ", buf);

        format_money_str(buf, schedule->items[i].balance);
        printf("%12s |\n", buf);
    }

    printf("-------------------------------------------------------------------\n");
    format_money_str(buf, schedule->total_paid - schedule->total_interest);
    printf("Total Principal Paid: %15s\n", buf);
    format_money_str(buf, schedule->total_interest);
    printf("Total Interest Cost:  %15s\n", buf);
    format_money_str(buf, schedule->total_paid);
    printf("Total Amount Paid:    %15s\n", buf);
    printf("-------------------------------------------------------------------\n");
}

int save_schedule_to_csv(const LoanSchedule *schedule, const char *filename) {
    if (!schedule || !schedule->items) return -1;

    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error: Failed to open file for writing.");
        return -1;
    }

    fprintf(file, "Month;Principal;Interest;Payment;Balance\n");

    for (int i = 0; i < schedule->count; i++) {
        fprintf(file, "%d;", i + 1);
        fprint_money(file, schedule->items[i].capital);  fprintf(file, ";");
        fprint_money(file, schedule->items[i].interest); fprintf(file, ";");
        fprint_money(file, schedule->items[i].payment);  fprintf(file, ";");
        fprint_money(file, schedule->items[i].balance);  fprintf(file, "\n");
    }

    fprintf(file, ";;;;\n");
    fprintf(file, "SUMMARY;;;;\n");
    fprintf(file, "Total Interest;"); fprint_money(file, schedule->total_interest); fprintf(file, ";;;\n");
    fprintf(file, "Total Paid;");     fprint_money(file, schedule->total_paid);     fprintf(file, ";;;\n");

    fclose(file);
    printf("Successfully saved report to: %s\n", filename);
    return 0;
}
#include <stdio.h>

#include "report.h"

void print_schedule_to_console(const LoanSchedule *schedule) {
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

int save_schedule_to_csv(const LoanSchedule *schedule, const char *filename) {
    if (!schedule || !schedule->items) return -1;

    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error: Failed to open file for writing.");
        return -1;
    }

    fprintf(file, "Month;Principal Part;Interest Part;Total Payment;Remaining Balance\n");

    for (int i = 0; i < schedule->count; i++) {
        fprintf(file, "%d;%.2Lf;%.2Lf;%.2Lf;%.2Lf\n", i+1,
            schedule->items[i].capital,
            schedule->items[i].interest,
            schedule->items[i].payment,
            schedule->items[i].balance);
    }

    fprintf(file, ";;;;\n"); // Pusty wiersz
    fprintf(file, "SUMMARY;;;;\n");
    fprintf(file, "Total Interest;%.2Lf;;;\n", schedule->total_interest);
    fprintf(file, "Total Paid;%.2Lf;;;\n", schedule->total_paid);

    fclose(file);
    printf("Successfully saved report to: %s\n", filename);
    return 0;
}

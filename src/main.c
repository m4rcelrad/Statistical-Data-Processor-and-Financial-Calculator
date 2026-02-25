#include <stdio.h>

#include "input_utils.h"
#include "loan_calculator_ui.h"

void generate_mock_files(void)
{
    printf("\n--- GENERATING MOCK FILES ---\n");

    FILE *file_equal = fopen("mock_loan_equal.csv", "w");
    if (file_equal) {
        fprintf(file_equal,
                "PrincipalAmount,TermMonths,LoanType,AnnualRate,OverpaymentPlan,MonthlyExtra\n");
        fprintf(file_equal, "250000.00,120,0,0.05,0,500.00\n");
        fclose(file_equal);
        printf("Successfully generated: mock_loan_equal.csv\n");
    } else {
        printf("Error: Could not create mock_loan_equal.csv\n");
    }

    FILE *file_decreasing = fopen("mock_loan_decreasing.csv", "w");
    if (file_decreasing) {
        fprintf(file_decreasing,
                "PrincipalAmount,TermMonths,LoanType,AnnualRate,OverpaymentPlan,MonthlyExtra\n");
        fprintf(file_decreasing, "500000.00,60,1,0.07,1,0.00\n");
        fclose(file_decreasing);
        printf("Successfully generated: mock_loan_decreasing.csv\n");
    } else {
        printf("Error: Could not create mock_loan_decreasing.csv\n");
    }

    FILE *file_schedule = fopen("mock_schedule.csv", "w");
    if (file_schedule) {
        fprintf(file_schedule, "Month,Amount\n");
        fprintf(file_schedule, "12,5000.00\n");
        fprintf(file_schedule, "24,5000.00\n");
        fprintf(file_schedule, "36,10000.00\n");
        fclose(file_schedule);
        printf("Successfully generated: mock_schedule.csv\n");
    } else {
        printf("Error: Could not create mock_schedule.csv\n");
    }

    printf("-----------------------------\n\n");
}

int main(void)
{
    int menu_choice;
    generate_mock_files();

    do {
        printf("============================================\n");
        printf("1. Time Series Analyzer\n");
        printf("2. Financial Calculator\n");
        printf("0. Exit\n");
        printf("============================================\n");

        const bool valid_input = read_integer_secure("Select an option [0-2]: ", &menu_choice);

        if (!valid_input)
            menu_choice = -1;

        switch (menu_choice) {
        case 1:
            printf("Running Time Series Analyzer...\n");
            break;
        case 2:
            printf("Running Financial Calculator...\n");
            loan_calculator_menu();
            break;
        case 0:
            printf("Exiting application. Goodbye!\n");
            break;
        default:
            printf("Error: Invalid option. Please enter 0, 1, or 2.\n");
        }
    } while (menu_choice != 0);

    return 0;
}

#include <stdio.h>

#include "input_utils.h"
#include "loan_calculator_ui.h"
#include "time_series_ui.h"

/**
 * @file main.c
 * @brief Central entry point for the Statistical Data Processor and Financial Calculator.
 *
 * This application provides an interactive menu allowing the user to select
 * between time series analysis and complex financial calculations.
 */

/**
 * @brief Generates mock CSV files containing sample data for demonstration purposes.
 * * Automatically creates dummy configuration files on disk to help the user test
 * the CSV import features of the application.
 */
static void generate_mock_files(void)
{
    printf("\n--- GENERATING MOCK FILES ---\n");

    FILE *file_equal = fopen("mock_loan_equal.csv", "w");
    if (file_equal) {
        fprintf(file_equal,
                "PrincipalAmount,TermMonths,LoanType,AnnualRate,OverpaymentPlan,MonthlyExtra\n");
        fprintf(file_equal, "250000.00,120,0,0.05,0,0.00\n");
        fclose(file_equal);
        printf("Successfully generated: mock_loan_equal.csv\n");
    } else {
        printf("Error: Could not create mock_loan_equal.csv\n");
    }

    FILE *file_decreasing = fopen("mock_loan_decreasing.csv", "w");
    if (file_decreasing) {
        fprintf(file_decreasing,
                "PrincipalAmount,TermMonths,LoanType,AnnualRate,OverpaymentPlan,MonthlyExtra\n");
        fprintf(file_decreasing, "500000.00,360,1,0.07,1,0.00\n");
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

    FILE *file_ts = fopen("mock_timeseries.csv", "w");
    if (file_ts) {
        fprintf(file_ts, "Date,Price,Volume\n");
        fprintf(file_ts, "2026-01-01,150.50,1000\n");
        fprintf(file_ts, "2026-01-02,152.00,1100\n");
        fprintf(file_ts, "2026-01-03,151.75,950\n");
        fprintf(file_ts, "2026-01-04,153.20,1200\n");
        fprintf(file_ts, "2026-01-05,155.00,1500\n");
        fprintf(file_ts, "2026-01-06,154.50,1300\n");
        fprintf(file_ts, "2026-01-07,156.10,1400\n");
        fprintf(file_ts, "2026-01-08,158.00,1600\n");
        fprintf(file_ts, "2026-01-09,157.20,1550\n");
        fprintf(file_ts, "2026-01-10,159.50,1700\n");
        fprintf(file_ts, "2026-01-11,161.00,1800\n");
        fprintf(file_ts, "2026-01-12,160.50,1750\n");
        fprintf(file_ts, "2026-01-13,162.20,1900\n");
        fprintf(file_ts, "2026-01-14,164.00,2000\n");
        fprintf(file_ts, "2026-01-15,163.50,1950\n");
        fclose(file_ts);
        printf("Successfully generated: mock_timeseries.csv\n");
    } else {
        printf("Error: Could not create mock_timeseries.csv\n");
    }

    printf("-----------------------------\n\n");
}

/**
 * @brief The main execution block and primary application menu.
 * * Displays the main menu, parses the user's choices safely, and acts as a router
 * switching contexts between different functional subsystems.
 * * @return 0 upon successful execution and normal application termination.
 */
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
            time_series_menu();
            break;
        case 2:
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

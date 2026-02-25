#include "loan_calculator_ui.h"

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "csv_reader.h"
#include "dataframe.h"
#include "input_utils.h"
#include "loan_math.h"
#include "loan_simulation.h"
#include "memory_utils.h"
#include "report.h"

/**
 * @brief Prints the expected CSV format instructions to standard output.
 * * Provides the user with a visual guide on how to structure their CSV
 * file for automated loan parameter loading.
 */
static void print_csv_format_help(void)
{
    printf("\n--- EXPECTED CSV FORMAT ---\n");
    printf("The CSV file must contain a header row followed by exactly one data row.\n");
    printf("Delimiter must be a comma (,).\n\n");
    printf("Columns expected (in order):\n");
    printf("1. PrincipalAmount  : The total loan amount (e.g., 250000.00)\n");
    printf("2. TermMonths       : Duration in months (e.g., 120)\n");
    printf("3. LoanType         : 0 for Equal Installments, 1 for Decreasing Installments\n");
    printf("4. AnnualRate       : Interest rate as a decimal (e.g., 0.05 for 5%%)\n");
    printf("5. OverpaymentPlan  : 0 to Reduce Term, 1 to Reduce Installment\n");
    printf("6. MonthlyExtra     : Flat custom overpayment added each month (e.g., 500.00)\n");
    printf("---------------------------\n\n");
}

/**
 * @brief Applies an irregular custom payment schedule from a CSV file to the payments array.
 * * @param filepath The path to the CSV file containing the irregular payment schedule.
 * @param term_months The total term of the loan in months.
 * @param payments Pointer to the array of custom payments to be updated.
 */
static void
apply_custom_payment_schedule(const char *filepath, const int term_months, Money *payments)
{
    if (!payments || term_months <= 0) {
        return;
    }

    DataFrame *df = NULL;
    const DataframeErrorCode err = read_csv(filepath, true, ",", &df);

    if (err != DATAFRAME_SUCCESS || !df) {
        printf("Warning: Could not load custom payment schedule from %s. Proceeding with standard "
               "payments.\n",
               filepath);
        if (df) {
            free_dataframe(df);
        }
        return;
    }

    if (df->rows < 1 || df->cols < 2) {
        printf("Error: Payment schedule CSV must have at least two columns (Month, Amount).\n");
        free_dataframe(df);
        return;
    }

    for (int i = 0; i < df->rows; i++) {
        const double month_val = df->data[i][0].v_num;
        const double amount_val = df->data[i][1].v_num;

        if (!isnan(month_val) && !isnan(amount_val)) {
            const int month_idx = (int)month_val - 1;

            if (month_idx >= 0 && month_idx < term_months && amount_val > 0.0) {
                const Money schedule_extra = money_from_major(amount_val);
                payments[month_idx] = money_add(payments[month_idx], schedule_extra);
            }
        }
    }

    free_dataframe(df);
    printf("Successfully applied custom payment schedule from %s\n", filepath);
}

/**
 * @brief Orchestrates the loan simulation process using the provided parameters.
 * * Generates the rates array, initializes the simulation configuration, runs the
 * schedule, prints the results, and optionally exports the output to a CSV file.
 * * @param loan The core loan definition structure.
 * @param base_rate The annual interest rate applied to the entire loan term.
 * @param strategy The selected overpayment strategy (reduce term or installment).
 * @param custom_payments Pointer to an array of custom overpayments for each month.
 */
static void execute_simulation(const LoanDefinition loan,
                               const double base_rate,
                               const OverpaymentStrategy strategy,
                               const Money *custom_payments)
{
    Rate *rates = aligned_calloc((size_t)loan.term_months, sizeof(Rate), CACHE_LINE_SIZE);
    if (!rates) {
        printf("Error: Could not allocate memory for rates.\n");
        return;
    }

    for (int i = 0; i < loan.term_months; i++) {
        rates[i] = create_rate(base_rate);
    }

    MarketScenario market;
    market.annual_rates = rates;

    SimulationConfig config;
    config.strategy = strategy;
    config.custom_payments = custom_payments;

    LoanSchedule schedule = {0};
    const FinanceErrorCode err = run_loan_simulation(&loan, &market, &config, &schedule);

    if (err == FINANCE_SUCCESS) {
        print_schedule_to_console(&schedule);

        int save_choice = 0;
        if (read_integer_secure(
                "\nWould you like to export the schedule to CSV? (1 for Yes, 0 for No): ",
                &save_choice)) {
            if (save_choice == 1) {
                char filename[256];
                if (read_string_secure("Enter destination filename (e.g., report.csv): ",
                                       filename,
                                       sizeof(filename))) {
                    save_schedule_to_csv(&schedule, filename);
                }
            }
        }
    } else {
        printf("Simulation failed with error: %s\n", finance_error_string(err));
    }

    free_schedule(&schedule);
    aligned_free(rates);
}

/**
 * @brief Prompts the user for manual entry of all loan parameters and initiates the simulation.
 */
static void process_manual_entry(void)
{
    double principal_major, base_rate, extra_payment_major;
    int term, type_input, strategy_input;

    printf("\n--- MANUAL ENTRY ---\n");

    if (!read_double_secure("Loan Principal Amount: ", &principal_major))
        return;

    if (!read_integer_secure("Term (in months): ", &term))
        return;

    if (!read_integer_secure("Loan Type (0 = Equal Installments, 1 = Decreasing Installments): ",
                             &type_input))
        return;

    if (!read_double_secure("Annual Interest Rate (e.g. 0.05 for 5%%): ", &base_rate))
        return;

    if (!read_integer_secure("Overpayment Strategy (0 = Reduce Term, 1 = Reduce Installment): ",
                             &strategy_input))
        return;

    if (!read_double_secure("Fixed Custom Overpayment Per Month (0 if none): ",
                            &extra_payment_major))
        return;

    LoanDefinition loan;
    loan.principal = money_from_major(principal_major);
    loan.term_months = term;
    loan.type = type_input == 0 ? LOAN_EQUAL_INSTALLMENTS : LOAN_DECREASING_INSTALLMENTS;

    const OverpaymentStrategy strategy =
        strategy_input == 0 ? STRATEGY_REDUCE_TERM : STRATEGY_REDUCE_INSTALLMENT;

    Money *custom_payments = aligned_calloc((size_t)term, sizeof(Money), CACHE_LINE_SIZE);
    if (custom_payments) {
        if (extra_payment_major > 0.0) {
            const Money flat_extra = money_from_major(extra_payment_major);
            for (int i = 0; i < term; i++) {
                custom_payments[i] = flat_extra;
            }
        }

        int load_custom = 0;
        if (read_integer_secure("\nWould you like to load an irregular overpayment schedule CSV? "
                                "(1 for Yes, 0 for No): ",
                                &load_custom)) {
            if (load_custom == 1) {
                char schedule_path[256];
                if (read_string_secure(
                        "Enter path to the schedule CSV: ", schedule_path, sizeof(schedule_path))) {
                    apply_custom_payment_schedule(schedule_path, term, custom_payments);
                }
            }
        }
    }

    execute_simulation(loan, base_rate, strategy, custom_payments);

    if (custom_payments) {
        aligned_free(custom_payments);
    }
}

/**
 * @brief Reads loan parameters from a user-specified CSV file and initiates the simulation.
 */
static void process_csv_entry(void)
{
    print_csv_format_help();

    char filepath[256];
    if (!read_string_secure("Enter path to the CSV file: ", filepath, sizeof(filepath))) {
        return;
    }

    DataFrame *df = NULL;
    const DataframeErrorCode err = read_csv(filepath, true, ",", &df);

    if (err != DATAFRAME_SUCCESS || !df) {
        printf("Failed to load or parse the CSV file. Please check the file path and format.\n");
        if (df) {
            free_dataframe(df);
        }
        return;
    }

    if (df->rows < 1 || !df->data) {
        printf("Error: The CSV file does not contain enough data rows.\n");
        free_dataframe(df);
        return;
    }

    const double principal_major = df->data[0][0].v_num;
    const double term_double = df->data[0][1].v_num;
    const double type_double = df->data[0][2].v_num;
    const double base_rate = df->data[0][3].v_num;
    const double strategy_double = df->data[0][4].v_num;
    const double extra_payment_major = df->data[0][5].v_num;

    if (isnan(principal_major) || isnan(term_double) || isnan(type_double) || isnan(base_rate) ||
        isnan(strategy_double) || isnan(extra_payment_major)) {
        printf("Error: One or more required columns contain invalid numeric data.\n");
        free_dataframe(df);
        return;
    }

    const int term = (int)term_double;
    const int type_input = (int)type_double;
    const int strategy_input = (int)strategy_double;

    if (principal_major <= 0.0 || term <= 0 || base_rate < 0.0 || extra_payment_major < 0.0) {
        printf(
            "Error: CSV contains out-of-bounds mathematical values (e.g., negative principal).\n");
        free_dataframe(df);
        return;
    }

    if (type_input != 0 && type_input != 1) {
        printf("Error: Loan Type must be 0 or 1.\n");
        free_dataframe(df);
        return;
    }

    if (strategy_input != 0 && strategy_input != 1) {
        printf("Error: Overpayment Strategy must be 0 or 1.\n");
        free_dataframe(df);
        return;
    }

    LoanDefinition loan;
    loan.principal = money_from_major(principal_major);
    loan.term_months = term;
    loan.type = type_input == 0 ? LOAN_EQUAL_INSTALLMENTS : LOAN_DECREASING_INSTALLMENTS;

    const OverpaymentStrategy strategy =
        strategy_input == 0 ? STRATEGY_REDUCE_TERM : STRATEGY_REDUCE_INSTALLMENT;

    printf("\nCSV loaded successfully. Preparing simulation...\n");

    Money *custom_payments = aligned_calloc((size_t)term, sizeof(Money), CACHE_LINE_SIZE);
    if (custom_payments) {
        if (extra_payment_major > 0.0) {
            const Money flat_extra = money_from_major(extra_payment_major);
            for (int i = 0; i < term; i++) {
                custom_payments[i] = flat_extra;
            }
        }

        int load_custom = 0;
        if (read_integer_secure("\nWould you like to load an irregular overpayment schedule CSV? "
                                "(1 for Yes, 0 for No): ",
                                &load_custom)) {
            if (load_custom == 1) {
                char schedule_path[256];
                if (read_string_secure(
                        "Enter path to the schedule CSV: ", schedule_path, sizeof(schedule_path))) {
                    apply_custom_payment_schedule(schedule_path, term, custom_payments);
                }
            }
        }
    }

    execute_simulation(loan, base_rate, strategy, custom_payments);

    if (custom_payments) {
        aligned_free(custom_payments);
    }

    free_dataframe(df);
}

void loan_calculator_menu(void)
{
    int menu_choice;

    do {
        printf("\n--- FINANCIAL CALCULATOR ---\n");
        printf("1. Enter loan parameters manually\n");
        printf("2. Load loan parameters from CSV file\n");
        printf("0. Return to Main Menu\n");
        printf("--------------------------------------------\n");

        const bool valid_input = read_integer_secure("Select an option [0-2]: ", &menu_choice);

        if (!valid_input)
            menu_choice = -1;

        switch (menu_choice) {
        case 1:
            process_manual_entry();
            break;
        case 2:
            process_csv_entry();
            break;
        case 0:
            printf("Returning to main menu...\n");
            break;
        default:
            printf("Invalid selection. Please try again.\n");
        }
    } while (menu_choice != 0);
}

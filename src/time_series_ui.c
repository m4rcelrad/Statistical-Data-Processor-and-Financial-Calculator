#include "time_series_ui.h"

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "csv_reader.h"
#include "dataframe.h"
#include "input_utils.h"
#include "memory_utils.h"
#include "statistics.h"

static void process_time_series_file(void)
{
    char filepath[256];
    if (!read_string_secure(
            "\nEnter path to the time series CSV file: ", filepath, sizeof(filepath))) {
        return;
    }

    DataFrame *df = NULL;
    const DataframeErrorCode err = read_csv(filepath, true, ",", &df);

    if (err != DATAFRAME_SUCCESS || !df) {
        printf("Failed to load or parse the CSV file. Please check the file path and format.\n");
        if (df)
            free_dataframe(df);
        return;
    }

    if (df->rows < 1 || df->cols < 1) {
        printf("Error: The CSV file is empty or missing columns.\n");
        free_dataframe(df);
        return;
    }

    printf("\nAvailable columns:\n");
    for (int i = 0; i < df->cols; i++) {
        printf("%d. %s\n", i + 1, df->columns[i] ? df->columns[i] : "Unknown");
    }

    int col_choice = 0;
    if (!read_integer_secure("\nSelect a column number to analyze: ", &col_choice) ||
        col_choice < 1 || col_choice > df->cols) {
        printf("Error: Invalid column selection.\n");
        free_dataframe(df);
        return;
    }

    const int target_col = col_choice - 1;
    if (df->col_types[target_col] != TYPE_NUMERIC) {
        printf("Error: The selected column does not contain numeric data.\n");
        free_dataframe(df);
        return;
    }

    int period = 0;
    if (!read_integer_secure("Enter the period for Moving Averages: ", &period) ||
        period <= 0) {
        printf("Error: Invalid period.\n");
        free_dataframe(df);
        return;
    }

    const size_t length = (size_t)df->rows;
    double *data = aligned_calloc(length, sizeof(double), CACHE_LINE_SIZE);
    double *sma = aligned_calloc(length, sizeof(double), CACHE_LINE_SIZE);
    double *ema = aligned_calloc(length, sizeof(double), CACHE_LINE_SIZE);
    const char **signals = aligned_calloc(length, sizeof(char *), CACHE_LINE_SIZE);

    if (!data || !sma || !ema || !signals) {
        printf("Error: Memory allocation failed.\n");
        aligned_free(data);
        aligned_free(sma);
        aligned_free(ema);
        aligned_free(signals);
        free_dataframe(df);
        return;
    }

    for (size_t i = 0; i < length; i++) {
        data[i] = df->data[i][target_col].v_num;
    }

    SeriesStatistics stats = {0};

    if (calculate_series_statistics(data, length, &stats) == STATS_SUCCESS) {
        printf("\n--- STATISTICAL ANALYSIS N(m, 𝜎) ---\n");
        printf("Mean (m):                 %.4f\n", stats.mean);
        printf("Standard Deviation (𝜎):   %.4f\n", stats.standard_deviation);
        printf("Variance:                 %.4f\n", stats.variance);
    } else {
        printf("\nWarning: Insufficient valid data to calculate N(m, 𝜎).\n");
    }
    const StatisticsErrorCode sma_err = calculate_sma(data, length, period, sma);
    const StatisticsErrorCode ema_err = calculate_ema(data, length, period, ema);

    if (sma_err == STATS_ERR_INSUFFICIENT_DATA || ema_err == STATS_ERR_INSUFFICIENT_DATA) {
        printf("\nWarning: The chosen period (%d) exceeds the dataset length (%zu). Moving "
               "averages cannot be calculated.\n",
               period,
               length);
        for (size_t i = 0; i < length; i++) {
            sma[i] = NAN;
            ema[i] = NAN;
        }
    }

    generate_trading_signals(data, sma, length, signals);

    printf("\n--- TIME SERIES DATA (Last 10 entries) ---\n");
    printf("%-10s | %-12s | %-12s | %-12s | %-10s\n", "Row", "Value", "SMA", "EMA", "Signal");
    printf("----------------------------------------------------------------------\n");

    const size_t start_idx = length > 10 ? length - 10 : 0;
    for (size_t i = start_idx; i < length; i++) {
        printf("%-10zu | ", i + 1);

        if (isnan(data[i]))
            printf("%-12s | ", "NaN");
        else
            printf("%-12.4f | ", data[i]);

        if (isnan(sma[i]))
            printf("%-12s | ", "NaN");
        else
            printf("%-12.4f | ", sma[i]);

        if (isnan(ema[i]))
            printf("%-12s | ", "NaN");
        else
            printf("%-12.4f | ", ema[i]);

        printf("%-10s\n", signals[i] ? signals[i] : "HOLD");
    }
    printf("----------------------------------------------------------------------\n");

    aligned_free(data);
    aligned_free(sma);
    aligned_free(ema);
    aligned_free(signals);
    free_dataframe(df);
}

void time_series_menu(void)
{
    int menu_choice;

    do {
        printf("\n--- TIME SERIES ANALYZER ---\n");
        printf("1. Load CSV and Analyze\n");
        printf("0. Return to Main Menu\n");
        printf("--------------------------------------------\n");

        if (!read_integer_secure("Select an option: ", &menu_choice)) {
            menu_choice = -1;
        }

        switch (menu_choice) {
        case 1:
            process_time_series_file();
            break;
        case 0:
            printf("Returning to main menu...\n");
            break;
        default:
            printf("Invalid selection. Please try again.\n");
        }
    } while (menu_choice != 0);
}

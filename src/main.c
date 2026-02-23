#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "csv_reader.h"
#include "dataframe.h"
#include "memory_utils.h"
#include "statistics.h"


#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

static ssize_t find_column_index(const DataFrame *df, const char *col_name) {
    if (!df || !col_name) return -1;
    for (size_t i = 0; i < (size_t)df->cols; i++) {
        if (df->columns[i] && strcmp(df->columns[i], col_name) == 0) {
            return (ssize_t)i;
        }
    }
    return -1;
}

int main(void) {
    const char *filepath = "market_data.csv";

    FILE *file = fopen(filepath, "w");
    if (file) {
        fprintf(file, "Date,ClosePrice\n");
        fprintf(file, "2023-10-01,150.0\n");
        fprintf(file, "2023-10-02,152.0\n");
        fprintf(file, "2023-10-03,151.5\n");
        fprintf(file, "2023-10-04,155.0\n");
        fprintf(file, "2023-10-05,153.0\n");
        fprintf(file, "2023-10-06,157.0\n");
        fprintf(file, "2023-10-07,156.5\n");
        fprintf(file, "2023-10-08,159.0\n");
        fclose(file);
    }

    DataFrame *df = NULL;
    const DataframeErrorCode status = read_csv(filepath, true, ",", &df);

    if (status != DATAFRAME_SUCCESS || !df) {
        printf("Error: Failed to load DataFrame. Code: %d\n", status);
        return 1;
    }

    const ssize_t price_col_idx = find_column_index(df, "ClosePrice");
    if (price_col_idx == -1 || df->col_types[price_col_idx] != TYPE_NUMERIC) {
        printf("Error: 'ClosePrice' numeric column not found in dataset!\n");
        free_dataframe(df);
        return 1;
    }

    const size_t row_count = (size_t)df->rows;
    double *close_prices = (double *)aligned_calloc(row_count, sizeof(double), CACHE_LINE_SIZE);
    if (!close_prices) {
        printf("Error: Aligned memory allocation failed for prices.\n");
        free_dataframe(df);
        return 1;
    }

    for (size_t i = 0; i < row_count; i++) {
        close_prices[i] = df->data[i][price_col_idx].v_num;
    }

    double mean_val = 0.0;
    double std_dev_val = 0.0;

    StatisticsErrorCode stats_error_code = calculate_mean(close_prices, row_count, &mean_val);
    if (stats_error_code != STATS_SUCCESS) {
        printf("Error calculating mean: %d\n", stats_error_code);
    }

    stats_error_code = calculate_standard_deviation(close_prices, row_count, &std_dev_val);
    if (stats_error_code != STATS_SUCCESS) {
        printf("Error calculating standard deviation: %d\n", stats_error_code);
    }

    double *sma_values = (double *)aligned_calloc(row_count, sizeof(double), CACHE_LINE_SIZE);
    const char **signals = (const char **)aligned_calloc(row_count, sizeof(char*), CACHE_LINE_SIZE);


    if (sma_values && signals) {
        const int sma_period = 3;

        if (calculate_sma(close_prices, row_count, sma_period, sma_values) == STATS_SUCCESS &&
            generate_trading_signals(close_prices, sma_values, row_count, signals) == STATS_SUCCESS) {

            printf("=== TIME SERIES ANALYSIS REPORT ===\n");
            printf("Distribution Parameters: N(m=%.2f, sigma=%.2f)\n", mean_val, std_dev_val);
            printf("----------------------------------------------------\n");
            printf("%-12s | %-10s | %-10s | %-10s\n", "Date", "Price", "SMA(3)", "Signal");
            printf("----------------------------------------------------\n");

            for (size_t i = 0; i < row_count; i++) {
                const char *date_str = (df->col_types[0] == TYPE_STRING) ? df->data[i][0].v_str : "N/A";

                printf("%-12s | %-10.2f | ", date_str, close_prices[i]);
                if (isnan(sma_values[i])) {
                    printf("%-10s | ", "NaN");
                } else {
                    printf("%-10.2f | ", sma_values[i]);
                }
                printf("%-10s\n", signals[i]);
            }
        } else {
            printf("Error: Statistics computation failed.\n");
        }
    }

    if (signals) aligned_free((void*)signals);
    if (sma_values) aligned_free(sma_values);
    if (close_prices) aligned_free(close_prices);
    free_dataframe(df);

    return 0;
}
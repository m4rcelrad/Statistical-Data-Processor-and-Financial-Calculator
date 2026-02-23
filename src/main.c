#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "dataframe.h"
#include "csv_reader.h"
#include "statistics.h"

static int find_column_index(const DataFrame *df, const char *col_name) {
    if (!df || !col_name) return -1;
    for (int i = 0; i < df->cols; i++) {
        if (df->columns[i] && strcmp(df->columns[i], col_name) == 0) {
            return i;
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

    int price_col_idx = find_column_index(df, "ClosePrice");
    if (price_col_idx == -1 || df->col_types[price_col_idx] != TYPE_NUMERIC) {
        printf("Error: 'ClosePrice' numeric column not found in dataset!\n");
        free_dataframe(df);
        return 1;
    }

    double *close_prices = calloc(df->rows, sizeof(double));
    if (!close_prices) {
        printf("Error: Memory allocation failed.\n");
        free_dataframe(df);
        return 1;
    }

    for (int i = 0; i < df->rows; i++) {
        close_prices[i] = df->data[i][price_col_idx].v_num;
    }

    const double m = calculate_mean(close_prices, df->rows);
    const double sigma = calculate_standard_deviation(close_prices, df->rows);

    double *sma_values = calloc(df->rows, sizeof(double));
    char **signals = calloc(df->rows, sizeof(char*));

    if (sma_values && signals) {
        const int sma_period = 3;
        calculate_sma(close_prices, df->rows, sma_period, sma_values);
        generate_trading_signals(close_prices, sma_values, df->rows, signals);

        printf("=== TIME SERIES ANALYSIS REPORT ===\n");
        printf("Distribution Parameters: N(m=%.2f, sigma=%.2f)\n", m, sigma);
        printf("----------------------------------------------------\n");
        printf("%-12s | %-10s | %-10s | %-10s\n", "Date", "Price", "SMA(3)", "Signal");
        printf("----------------------------------------------------\n");

        for (int i = 0; i < df->rows; i++) {
            const char *date_str = (df->col_types[0] == TYPE_STRING) ? df->data[i][0].v_str : "N/A";

            printf("%-12s | %-10.2f | ", date_str, close_prices[i]);
            if (isnan(sma_values[i])) {
                printf("%-10s | ", "NaN");
            } else {
                printf("%-10.2f | ", sma_values[i]);
            }
            printf("%-10s\n", signals[i] ? signals[i] : "HOLD");
        }
    }

    if (signals) {
        for (int i = 0; i < df->rows; i++) {
            if (signals[i]) free(signals[i]);
        }
        free(signals);
    }
    if (sma_values) free(sma_values);
    if (close_prices) free(close_prices);
    free_dataframe(df);

    return 0;
}
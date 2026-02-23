#include "dataframe.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

DataFrame *create_dataframe(const int rows, const int cols) {
    DataFrame *df = malloc(sizeof(DataFrame));
    if (!df) return NULL;

    df->rows = rows;
    df->cols = cols;

    df->columns = calloc(cols, sizeof(char *));
    if (!df->columns) {
        free(df);
        return NULL;
    }

    df->data = calloc(rows, sizeof(double*));
    if (!df->data) {
        free(df->columns);
        free(df);
        return NULL;
    }

    for (int i = 0; i < rows; i++) {
        df->data[i] = calloc(cols, sizeof(double));
        if (!df->data[i]) {
            free_dataframe(df);
            return NULL;
        }
    }

    return df;
}

void free_dataframe(DataFrame *df) {
    if (!df) return;

    if (df->columns) {
        for (int i = 0; i < df->cols; i++) {
            free(df->columns[i]);
        }
        free(df->columns);
    }

    if (df->data) {
        for (int i = 0; i < df->rows; i++) {
            free(df->data[i]);
        }
        free(df->data);
    }

    free(df);
}

void print_head_dataframe(const DataFrame *df, const int limit) {
    if (!df) return;

    printf("\nData Preview (Top %d):\n", limit);
    printf("-------------------------------------------------------------------\n");

    for (int i = 0; i < df->cols; i++) {
        printf("%-12s ", df->columns[i] ? df->columns[i] : "N/A");
    }
    printf("\n-------------------------------------------------------------------\n");

    const int print_rows = limit < df->rows ? limit : df->rows;
    for (int r = 0; r < print_rows; r++) {
        for (int c = 0; c < df->cols; c++) {
            if ( isnan(df->data[r][c])) {
                printf("%-12s ", "NaN");
            } else {
                printf("%-12.4f ", df->data[r][c]);
            }
        }
        printf("\n");
    }
    printf("\n[Shape: %d rows x %d cols]\n", df->rows, df->cols);
}
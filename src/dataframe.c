#include "dataframe.h"
#include "memory_utils.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

DataFrame* create_dataframe(const size_t rows, const size_t cols) {
    if (rows == 0 || cols == 0) return NULL;

    DataFrame *df = (DataFrame *)aligned_calloc(1, sizeof(DataFrame), CACHE_LINE_SIZE);
    if (!df) return NULL;

    df->rows = (int)rows;
    df->cols = (int)cols;

    df->columns = (char **)aligned_calloc(cols, sizeof(char *), CACHE_LINE_SIZE);
    df->col_types = (DataType *)aligned_calloc(cols, sizeof(DataType), CACHE_LINE_SIZE);
    df->data = (DataCell **)aligned_calloc(rows, sizeof(DataCell *), CACHE_LINE_SIZE);

    if (!df->columns || !df->col_types || !df->data) {
        free_dataframe(df);
        return NULL;
    }

    for (size_t i = 0; i < rows; i++) {
        df->data[i] = (DataCell *)aligned_calloc(cols, sizeof(DataCell), CACHE_LINE_SIZE);
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
            if (df->columns[i]) {
                free(df->columns[i]);
            }
        }
        aligned_free(df->columns);
    }

    if (df->data && df->col_types) {
        for (int r = 0; r < df->rows; r++) {
            if (df->data[r]) {
                for (int c = 0; c < df->cols; c++) {
                    if (df->col_types[c] == TYPE_STRING && df->data[r][c].v_str) {
                        free(df->data[r][c].v_str);
                    }
                }
                aligned_free(df->data[r]);
            }
        }
        aligned_free(df->data);
    }

    if (df->col_types) {
        aligned_free(df->col_types);
    }

    aligned_free(df);
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
            if (df->col_types[c] == TYPE_STRING) {
                printf("%-12s ", df->data[r][c].v_str ? df->data[r][c].v_str : "NULL");
            } else {
                if (isnan(df->data[r][c].v_num)) {
                    printf("%-12s ", "NaN");
                } else {
                    printf("%-12.4f ", df->data[r][c].v_num);
                }
            }
        }
        printf("\n");
    }
    printf("\n[Shape: %d rows x %d cols]\n", df->rows, df->cols);
}
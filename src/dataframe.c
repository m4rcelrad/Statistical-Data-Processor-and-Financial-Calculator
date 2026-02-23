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

    df->col_types = calloc(cols, sizeof(DataType));
    if (!df->col_types) {
        free(df->columns);
        free(df);
        return NULL;
    }

    df->data = calloc(rows, sizeof(DataCell*));
    if (!df->data) {
        free(df->col_types);
        free(df->columns);
        free(df);
        return NULL;
    }

    for (int i = 0; i < rows; i++) {
        df->data[i] = calloc(cols, sizeof(DataCell));
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

    if (df->data && df->col_types) {
        for (int r = 0; r < df->rows; r++) {
            if (df->data[r]) {
                for (int c = 0; c < df->cols; c++) {
                    if (df->col_types[c] == TYPE_STRING && df->data[r][c].v_str) {
                        free(df->data[r][c].v_str);
                    }
                }
                free(df->data[r]);
            }
        }
        free(df->data);
    }

    if (df->col_types) {
        free(df->col_types);
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
#include "dataframe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 2048
#define DELIMITERS ";,\r\n"

DataFrame* create_dataframe(const int rows, const int cols) {
    DataFrame *df = malloc(sizeof(DataFrame));
    if (!df) return NULL;

    df->rows = rows;
    df->cols = cols;

    df->columns = calloc(cols, sizeof(char*));

    df->data = malloc(rows * sizeof(double*));
    for (int i = 0; i < rows; i++) {
        df->data[i] = calloc(cols, sizeof(double));
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

DataFrame* read_csv(const char *path, const bool has_header) {
    FILE *file = fopen(path, "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    char line[MAX_LINE_LENGTH];

    if (!fgets(line, sizeof(line), file)) {
        fclose(file);
        return NULL;
    }

    int cols = 0;
    const char *token = strtok(line, DELIMITERS);
    while (token) {
        cols++;
        token = strtok(NULL, DELIMITERS);
    }

    int rows = has_header ? 0 : 1;
    while (fgets(line, sizeof(line), file)) {
        if (strlen(line) > 1) rows++;
    }

    rewind(file);

    DataFrame *df = create_dataframe(rows, cols);
    if (!df) {
        fclose(file);
        return NULL;
    }

    if (has_header) {
        if (fgets(line, sizeof(line), file)) {
            int c = 0;
            const char *header = strtok(line, DELIMITERS);
            while (header && c < cols) {
                df->columns[c] = strdup(header);
                header = strtok(NULL, DELIMITERS);
                c++;
            }
        }
    } else {
        for (int c = 0; c < cols; c++) {
            char buffer[32];
            sprintf(buffer, "Col %d", c + 1);
            df->columns[c] = strdup(buffer);
        }
    }

    int r = 0;
    while (r < rows && fgets(line, sizeof(line), file)) {
        int c = 0;
        const char *val_str = strtok(line, DELIMITERS);
        while (val_str && c < cols) {
            df->data[r][c] = strtod(val_str, NULL);
            val_str = strtok(NULL, DELIMITERS);
            c++;
        }
        r++;
    }

    fclose(file);
    return df;
}

void print_head_dataframe(const DataFrame *df, int limit) {
    if (!df) return;

    printf("\nData Preview (Top %d):\n", limit);
    printf("-------------------------------------------------------------------\n");

    for (int i = 0; i < df->cols; i++) {
        printf("%-12s ", df->columns[i] ? df->columns[i] : "N/A");
    }
    printf("\n-------------------------------------------------------------------\n");

    const int print_rows = (limit < df->rows) ? limit : df->rows;
    for (int r = 0; r < print_rows; r++) {
        for (int c = 0; c < df->cols; c++) {
            printf("%-12.4f ", df->data[r][c]);
        }
        printf("\n");
    }
    printf("\n[Shape: %d rows x %d cols]\n", df->rows, df->cols);
}
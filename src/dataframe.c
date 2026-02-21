#include "dataframe.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_LINE_LENGTH 8192

static char *trim_and_unquote(char *str) {
    while (isspace((unsigned char) *str))
        str++;
    if (*str == '"') {
        str++;
        char *end = str + strlen(str) - 1;
        while (end > str && (isspace((unsigned char) *end) || *end == '"')) {
            *end = '\0';
            end--;
        }
    } else {
        char *end = str + strlen(str) - 1;
        while (end > str && isspace((unsigned char) *end)) {
            *end = '\0';
            end--;
        }
    }

    return str;
}

static int parse_line_to_tokens(char *line, char **tokens, const int max_cols, const char *delim) {
    int count = 0;
    char *ptr = line;
    bool in_quotes = false;
    char *start = line;

    while (*ptr && count < max_cols) {
        if (*ptr == '"') {
            in_quotes = !in_quotes;
        } else if (*ptr == delim[0] && !in_quotes) {
            *ptr = '\0';
            tokens[count++] = trim_and_unquote(start);
            start = ptr + 1;
        }
        ptr++;
    }

    if (count < max_cols) {
        tokens[count++] = trim_and_unquote(start);
    }
    return count;
}

DataFrame *create_dataframe(const int rows, const int cols) {
    DataFrame *df = malloc(sizeof(DataFrame));
    if (!df) return NULL;

    df->rows = rows;
    df->cols = cols;
    df->columns = calloc(cols, sizeof(char *));
    df->data = malloc(rows * sizeof(double *));
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

DataframeErrorCode read_csv(const char *path, const bool has_header, const char *delim, DataFrame **out_df) {
    if (!out_df) return DATAFRAME_ERR_ALLOCATION_FAILED;
    *out_df = NULL;

    FILE *file = fopen(path, "r");
    if (!file) return DATAFRAME_ERR_FILE_NOT_FOUND;

    char line[MAX_LINE_LENGTH];
    if (!fgets(line, sizeof(line), file)) {
        fclose(file);
        return DATAFRAME_ERR_EMPTY_FILE;
    }

    int expected_cols = 0;
    bool in_q = false;
    for (const char *p = line; *p; p++) {
        if (*p == '"') in_q = !in_q;
        if (*p == delim[0] && !in_q) expected_cols++;
    }
    expected_cols++;

    int data_rows_count = 0;
    while (fgets(line, sizeof(line), file)) {
        if (strlen(line) > 1)
            data_rows_count++;
    }

    if (!has_header) data_rows_count++;
    rewind(file);

    DataFrame *df = create_dataframe(data_rows_count, expected_cols);
    if (!df) {
        fclose(file);
        return DATAFRAME_ERR_ALLOCATION_FAILED;
    }

    char **row_tokens = malloc(expected_cols * sizeof(char *));
    if (!row_tokens) {
        free_dataframe(df);
        fclose(file);
        return DATAFRAME_ERR_ALLOCATION_FAILED;
    }

    int current_r = 0;
    bool is_first_line = true;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = 0;
        int const actual_cols = parse_line_to_tokens(line, row_tokens, expected_cols, delim);

        if (actual_cols != expected_cols) {
            free(row_tokens);
            free_dataframe(df);
            fclose(file);
            return DATAFRAME_ERR_COLUMN_MISMATCH;
        }

        if (is_first_line) {
            is_first_line = false;

            if (has_header) {
                for (int c = 0; c < expected_cols; c++) {
                    df->columns[c] = strdup(row_tokens[c]);
                }
                continue;
            }

            for (int c = 0; c < expected_cols; c++) {
                char buf[32];
                snprintf(buf, sizeof(buf), "col_%d", c + 1);
                df->columns[c] = strdup(buf);
            }
        }

        for (int c = 0; c < expected_cols; c++) {
            char *endptr;
            const char *token = row_tokens[c];
            while(isspace((unsigned char)*token)) token++;

            if (*token == '\0') {
                df->data[current_r][c] = NAN;
            } else {
                const double val = strtod(token, &endptr);
                df->data[current_r][c] = token == endptr ? NAN : val;
            }
        }
        current_r++;
    }
    df->rows = current_r;

    free(row_tokens);
    fclose(file);

    *out_df = df;
    return DATAFRAME_SUCCESS;
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
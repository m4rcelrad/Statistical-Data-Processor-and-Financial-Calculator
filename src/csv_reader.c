#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "csv_reader.h"

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

static ssize_t read_line(FILE *file, char **buffer, size_t *capacity) {
    if (*buffer == NULL || *capacity == 0) {
        *capacity = 256;
        *buffer = malloc(*capacity);
        if (!*buffer)
            return -1;
    }

    size_t length = 0;
    int c;

    while ((c = fgetc(file)) != EOF) {
        if (length + 1 >= *capacity) {
            const size_t new_cap = *capacity * 2;
            char *tmp = realloc(*buffer, new_cap);
            if (!tmp)
                return -1;
            *buffer = tmp;
            *capacity = new_cap;
        }

        (*buffer)[length++] = (char) c;

        if (c == '\n')
            break;
    }

    if (length == 0 && c == EOF)
        return -1;

    (*buffer)[length] = '\0';
    return (ssize_t) length;
}

static char *trim_and_unquote(char *str) {
    while (isspace((unsigned char) *str)) str++;
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

    while (*ptr) {
        if (*ptr == '"') {
            in_quotes = !in_quotes;
        } else if (*ptr == delim[0] && !in_quotes) {
            *ptr = '\0';

            if (count < max_cols) {
                tokens[count] = trim_and_unquote(start);
            }

            count++;
            start = ptr + 1;
        }
        ptr++;
    }

    if (count < max_cols) {
        tokens[count] = trim_and_unquote(start);
    }
    count++;

    return count;
}

DataframeErrorCode read_csv(const char *path, const bool has_header, const char *delim, DataFrame **out_df) {
    if (!out_df)
        return DATAFRAME_ERR_ALLOCATION_FAILED;
    *out_df = NULL;

    FILE *file = fopen(path, "r");
    if (!file)
        return DATAFRAME_ERR_FILE_NOT_FOUND;

    char *line = NULL;
    size_t capacity = 0;

    ssize_t read_len = read_line(file, &line, &capacity);
    if (read_len <= 0) {
        if (line)
            free(line);
        fclose(file);
        return DATAFRAME_ERR_EMPTY_FILE;
    }

    int expected_cols = 0;
    bool in_q = false;
    for (const char *p = line; *p; p++) {
        if (*p == '"')
            in_q = !in_q;
        if (*p == delim[0] && !in_q)
            expected_cols++;
    }
    expected_cols++;

    int data_rows_count = 0;
    while ((read_len = read_line(file, &line, &capacity)) > 0) {
        if (read_len > 1 || (line[0] != '\n' && line[0] != '\r')) {
            data_rows_count++;
        }
    }

    if (!has_header)
        data_rows_count++;
    rewind(file);

    DataFrame *df = create_dataframe(data_rows_count, expected_cols);
    if (!df) {
        free(line);
        fclose(file);
        return DATAFRAME_ERR_ALLOCATION_FAILED;
    }

    char **row_tokens = malloc(expected_cols * sizeof(char *));
    if (!row_tokens) {
        free_dataframe(df);
        free(line);
        fclose(file);
        return DATAFRAME_ERR_ALLOCATION_FAILED;
    }

    int current_r = 0;
    bool is_first_line = true;
    bool types_detected = false;

    while (read_line(file, &line, &capacity) > 0) {
        line[strcspn(line, "\r\n")] = 0;

        if (line[0] == '\0')
            continue;

        int const actual_cols = parse_line_to_tokens(line, row_tokens, expected_cols, delim);

        if (actual_cols != expected_cols) {
            free(row_tokens);
            free_dataframe(df);
            free(line);
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

        if (!types_detected) {
            for (int c = 0; c < expected_cols; c++) {
                char *endptr;
                const char *token = row_tokens[c];
                while (isspace((unsigned char)*token)) token++;

                if (*token == '\0') {
                    df->col_types[c] = TYPE_NUMERIC;
                } else {
                    strtod(token, &endptr);
                    if (token != endptr && *endptr == '\0') {
                        df->col_types[c] = TYPE_NUMERIC;
                    } else {
                        df->col_types[c] = TYPE_STRING;
                    }
                }
            }
            types_detected = true;
        }

        for (int c = 0; c < expected_cols; c++) {
            const char *token = row_tokens[c];
            while (isspace((unsigned char)*token)) token++;

            if (df->col_types[c] == TYPE_STRING) {
                df->data[current_r][c].v_str = strdup(token);
            } else {
                if (*token == '\0') {
                    df->data[current_r][c].v_num = NAN;
                } else {
                    char *endptr;
                    const double val = strtod(token, &endptr);
                    df->data[current_r][c].v_num = token == endptr ? NAN : val;
                }
            }
        }
        current_r++;
    }

    df->rows = current_r;

    free(row_tokens);
    free(line);
    fclose(file);

    *out_df = df;
    return DATAFRAME_SUCCESS;
}
#include "input_utils.h"
#include "typedefs.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool read_integer_secure(const char *prompt, int *out_value)
{
    char input_buffer[64];

    if (!out_value) {
        return false;
    }

    while (true) {
        if (prompt) {
            printf("%s", prompt);
            fflush(stdout);
        }

        if (!fgets(input_buffer, sizeof(input_buffer), stdin)) {
            return false;
        }

        if (strchr(input_buffer, '\n') == NULL) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
                ;
        } else {
            input_buffer[strcspn(input_buffer, "\r\n")] = 0;
        }

        char *endptr;
        errno = 0;
        const long parsed_value = strtol(input_buffer, &endptr, 10);

        if (endptr == input_buffer) {
            printf("Error: No digits were found. Please enter a valid number.\n");
            continue;
        }

        while (isspace((unsigned char)*endptr)) {
            endptr++;
        }

        if (*endptr != '\0') {
            printf("Error: Invalid characters detected after the number.\n");
            continue;
        }

        if (errno == ERANGE || parsed_value < INT_MIN || parsed_value > INT_MAX) {
            printf("Error: The number entered is out of the allowed range.\n");
            continue;
        }

        *out_value = (int)parsed_value;
        return true;
    }
}

bool read_double_secure(const char *prompt, double *out_value)
{
    char input_buffer[64];

    if (!out_value) {
        return false;
    }

    while (true) {
        if (prompt) {
            printf("%s", prompt);
            fflush(stdout);
        }

        if (!fgets(input_buffer, sizeof(input_buffer), stdin)) {
            return false;
        }

        if (strchr(input_buffer, '\n') == NULL) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
                ;
        } else {
            input_buffer[strcspn(input_buffer, "\r\n")] = 0;
        }

        char *endptr;
        errno = 0;
        const double parsed_value = strtod(input_buffer, &endptr);

        if (endptr == input_buffer) {
            printf("Error: No digits were found. Please enter a valid number.\n");
            continue;
        }

        while (isspace((unsigned char)*endptr)) {
            endptr++;
        }

        if (*endptr != '\0') {
            printf("Error: Invalid characters detected after the number.\n");
            continue;
        }

        if (errno == ERANGE) {
            printf("Error: The number entered is out of the allowed range.\n");
            continue;
        }

        *out_value = parsed_value;
        return true;
    }
}

bool read_string_secure(const char *prompt, char *buffer, const size_t size)
{
    if (!buffer || size == 0) {
        return false;
    }

    while (true) {
        if (prompt) {
            printf("%s", prompt);
            fflush(stdout);
        }

        if (!fgets(buffer, (int)size, stdin)) {
            return false;
        }

        if (strchr(buffer, '\n') == NULL) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
                ;
        } else {
            buffer[strcspn(buffer, "\r\n")] = 0;
        }

        if (buffer[0] != '\0') {
            return true;
        }

        printf("Error: Input cannot be empty. Please try again.\n");
    }
}

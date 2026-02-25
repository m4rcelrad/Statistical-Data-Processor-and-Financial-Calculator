#include "input_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

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
            while ((c = getchar()) != '\n' && c != EOF);
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
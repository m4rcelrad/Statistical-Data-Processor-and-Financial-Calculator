#ifndef STATISTICALDATAPROCESSOR_INPUT_UTILS_H
#define STATISTICALDATAPROCESSOR_INPUT_UTILS_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @file input_utils.h
 * @brief Utilities for secure and robust console input.
 */

/**
 * @brief Prompts the user and safely reads an integer from standard input.
 *
 * This function handles buffer overflows, invalid characters, trailing whitespace,
 * and out-of-range numbers. It will loop and continually prompt the user until
 * a valid integer is successfully provided.
 *
 * @param prompt The text string to display to the user before reading input.
 * @param out_value Pointer to the integer where the valid input will be stored.
 * @return true if successful, false if EOF or an unrecoverable read error occurs.
 */
bool read_integer_secure(const char *prompt, int *out_value);

bool read_double_secure(const char *prompt, double *out_value);

bool read_string_secure(const char *prompt, char *out_buffer, size_t buffer_size);

#endif // STATISTICALDATAPROCESSOR_INPUT_UTILS_H
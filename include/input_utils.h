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

/**
 * @brief Prompts the user and safely reads a double-precision floating-point number from standard input.
 *
 * Similar to the integer read function, this handles buffer overflows, invalid characters,
 * trailing whitespace, and limits (ERANGE). It loops and prompts the user continually
 * until a valid, in-range double is provided.
 *
 * @param prompt The text string to display to the user before reading input.
 * @param out_value Pointer to the double where the valid parsed input will be stored.
 * @return true if successful, false if EOF or an unrecoverable read error occurs.
 */
bool read_double_secure(const char *prompt, double *out_value);

/**
 * @brief Prompts the user and safely reads a string from standard input.
 *
 * This function reads up to the specified buffer size, mitigating overflow risks.
 * It automatically strips the trailing newline character and ensures that the provided
 * input is not empty, prompting the user again if necessary.
 *
 * @param prompt The text string to display to the user before reading input.
 * @param out_buffer Pointer to the character array where the input string will be stored.
 * @param buffer_size The maximum capacity of the buffer (including the null terminator).
 * @return true if successful, false if EOF or an unrecoverable read error occurs.
 */
bool read_string_secure(const char *prompt, char *out_buffer, size_t buffer_size);

#endif // STATISTICALDATAPROCESSOR_INPUT_UTILS_H

#ifndef STATISTICALDATAPROCESSOR_MONEY_H
#define STATISTICALDATAPROCESSOR_MONEY_H

#include <stdbool.h>

/**
 * @file money.h
 * @brief Module for handling currency operations using fixed-point arithmetic.
 *
 * This module provides a robust way to represent and manipulate monetary values,
 * avoiding common floating-point precision issues by storing values as integers.
 */

/**
 * @brief The scale factor used to convert major currency units to minor units.
 * For example, a scale of 100 means the internal value represents cents.
 */
#define CURRENCY_SCALE 100

/**
 * @brief Structure representing a monetary value.
 *
 * The value is stored as a 64-bit integer to prevent overflow during calculations
 * and to maintain precision. It represents the minor currency unit (e.g., cents).
 */
typedef struct {
    long long value;
} Money;

/**
 * @brief A constant representing a zero monetary value.
 */
extern const Money MONEY_ZERO;

/**
 * @brief Converts a major currency value (e.g., dollars) to the internal Money representation.
 * @param major_amount The floating-point amount in major units.
 * @return A Money structure representing the equivalent value in minor units.
 */
Money money_from_major(long double major_amount);

/**
 * @brief Converts the internal Money representation back to a major currency value.
 * @param amount The Money structure to convert.
 * @return The floating-point representation in major units.
 */
long double money_to_major(Money amount);

/**
 * @brief Adds two monetary values.
 * @param a The first amount.
 * @param b The second amount.
 * @return The sum of the two amounts.
 */
Money money_add(Money a, Money b);

/**
 * @brief Subtracts one monetary value from another.
 * @param a The base amount.
 * @param b The amount to subtract.
 * @return The difference between the two amounts.
 */
Money money_sub(Money a, Money b);

/**
 * @brief Multiplies a monetary value by a floating-point factor.
 * @param base The base monetary amount.
 * @param factor The multiplier.
 * @return The product, properly rounded to the nearest minor unit.
 */
Money money_mul(Money base, long double factor);

/**
 * @brief Divides a monetary value by an integer divisor.
 * @param base The base monetary amount.
 * @param divisor The integer divisor.
 * @return The quotient. Returns MONEY_ZERO if the divisor is 0.
 */
Money money_div(Money base, int divisor);

/**
 * @brief Checks if two monetary values are strictly equal.
 * @param a The first amount.
 * @param b The second amount.
 * @return true if amounts are equal, false otherwise.
 */
bool money_eq(Money a, Money b);

/**
 * @brief Checks if the first monetary value is strictly greater than the second.
 * @param a The first amount.
 * @param b The second amount.
 * @return true if a > b, false otherwise.
 */
bool money_gt(Money a, Money b);

/**
 * @brief Checks if the first monetary value is strictly less than the second.
 * @param a The first amount.
 * @param b The second amount.
 * @return true if a < b, false otherwise.
 */
bool money_lt(Money a, Money b);

/**
 * @brief Checks if the first monetary value is greater than or equal to the second.
 * @param a The first amount.
 * @param b The second amount.
 * @return true if a >= b, false otherwise.
 */
bool money_gte(Money a, Money b);

/**
 * @brief Checks if the first monetary value is less than or equal to the second.
 * @param a The first amount.
 * @param b The second amount.
 * @return true if a <= b, false otherwise.
 */
bool money_lte(Money a, Money b);

/**
 * @brief Checks if the monetary value is exactly zero.
 * @param a The amount to check.
 * @return true if the amount is zero, false otherwise.
 */
bool money_is_zero(Money a);

/**
 * @brief Checks if the monetary value is strictly positive.
 * @param a The amount to check.
 * @return true if the amount is greater than zero, false otherwise.
 */
bool money_is_positive(Money a);

#endif // STATISTICALDATAPROCESSOR_MONEY_H

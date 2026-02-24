#include <math.h>

#include "money.h"
#include "unity/unity.h"

/**
 * @file tests_money.c
 * @brief Unit tests for the money module.
 *
 * This file contains test cases validating the behavior of financial
 * operations, rounding, and comparisons.
 */

/**
 * @brief Tests the standard conversion from major currency units to the internal Money representation.
 * * Expected result: The floating-point value 123.45 is correctly scaled and stored as 12345.
 */
void test_MoneyFromMajor_Standard(void)
{
    Money m = money_from_major(123.45);
    TEST_ASSERT_EQUAL_INT64(12345, m.value);
}

/**
 * @brief Tests the rounding behavior during conversion from major to minor units.
 * * Expected result: Values rounding up (e.g., .456) and rounding down (e.g., .451)
 * are correctly transformed to the nearest integer.
 */
void test_MoneyFromMajor_Rounding(void)
{
    Money m = money_from_major(123.456);
    TEST_ASSERT_EQUAL_INT64(12346, m.value);

    m = money_from_major(123.451);
    TEST_ASSERT_EQUAL_INT64(12345, m.value);
}

/**
 * @brief Tests the conversion from the internal Money representation back to a major currency value.
 * * Expected result: The minor unit value 12345 is correctly returned as the floating-point value 123.45.
 */
void test_MoneyToMajor(void)
{
    Money m = {12345};
    long double val = money_to_major(m);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 123.45, val);
}

/**
 * @brief Tests the addition of two Money values.
 * * Expected result: The sum of the internal representations is mathematically accurate.
 */
void test_MoneyAdd(void)
{
    Money a = {12345};
    Money b = {54321};
    Money c = money_add(a, b);
    TEST_ASSERT_EQUAL_INT64(66666, c.value);
}

/**
 * @brief Tests the subtraction of one Money value from another.
 * * Expected result: The difference is mathematically accurate.
 */
void test_MoneySub(void)
{
    Money a = {54321};
    Money b = {12345};
    Money c = money_sub(a, b);
    TEST_ASSERT_EQUAL_INT64(41976, c.value);
}

/**
 * @brief Tests subtraction resulting in a negative Money value.
 * * Expected result: Subtraction accurately yields a negative internal representation.
 */
void test_MoneySub_NegativeResult(void)
{
    Money m1 = {1000};
    Money m2 = {2000};
    Money res = money_sub(m1, m2);
    TEST_ASSERT_EQUAL_INT64(-1000, res.value);
}

/**
 * @brief Tests the multiplication of a Money value by an integer factor.
 * * Expected result: The multiplied value is correct without loss of precision.
 */
void test_MoneyMul_IntFactor(void)
{
    Money m = {10000};
    Money res = money_mul(m, 5);
    TEST_ASSERT_EQUAL_INT64(50000, res.value);
}

/**
 * @brief Tests the multiplication of a Money value by a floating-point factor.
 * * Expected result: The product is appropriately scaled and rounded to the nearest integer.
 */
void test_MoneyMul_FloatFactor(void)
{
    Money m = {10000};
    Money res = money_mul(m, 1.0 / 3.0);
    TEST_ASSERT_EQUAL_INT64(3333, res.value);
}

/**
 * @brief Tests the division of a Money value by a non-zero integer.
 * * Expected result: The division behaves as standard integer division.
 */
void test_MoneyDiv(void)
{
    Money m = {10000};
    Money res = money_div(m, 2);
    TEST_ASSERT_EQUAL_INT64(5000, res.value);
}

/**
 * @brief Tests the division of a Money value by zero.
 * * Expected result: The function safely handles the division and returns zero, avoiding a crash.
 */
void test_MoneyDiv_ByZero(void)
{
    Money m = {10000};
    Money res = money_div(m, 0);
    TEST_ASSERT_EQUAL_INT64(0, res.value);
}

/**
 * @brief Tests all logical comparison operations between Money values.
 * * Expected result: Less-than, greater-than, equal, less-than-or-equal, and greater-than-or-equal
 * evaluate correctly according to standard logic.
 */
void test_MoneyComparisons(void)
{
    Money small = {1000};
    Money big = {2000};
    Money equal = {1000};

    TEST_ASSERT_TRUE(money_lt(small, big));
    TEST_ASSERT_TRUE(money_gt(big, small));
    TEST_ASSERT_TRUE(money_eq(small, equal));
    TEST_ASSERT_FALSE(money_eq(small, big));
    TEST_ASSERT_TRUE(money_lte(small, equal));
    TEST_ASSERT_TRUE(money_gte(big, small));
}

/**
 * @brief Tests the zero-checking function for Money values.
 * * Expected result: Correctly identifies the global MONEY_ZERO constant as zero,
 * and strictly non-zero amounts as false.
 */
void test_MoneyIsZero(void)
{
    TEST_ASSERT_TRUE(money_is_zero(MONEY_ZERO));
    TEST_ASSERT_FALSE(money_is_zero(money_from_major(0.01)));
}

/**
 * @brief Test runner function that registers and executes all money module tests.
 */
void run_money_tests(void)
{
    RUN_TEST(test_MoneyFromMajor_Standard);
    RUN_TEST(test_MoneyFromMajor_Rounding);
    RUN_TEST(test_MoneyToMajor);
    RUN_TEST(test_MoneyAdd);
    RUN_TEST(test_MoneySub);
    RUN_TEST(test_MoneySub_NegativeResult);
    RUN_TEST(test_MoneyMul_IntFactor);
    RUN_TEST(test_MoneyMul_FloatFactor);
    RUN_TEST(test_MoneyDiv);
    RUN_TEST(test_MoneyDiv_ByZero);
    RUN_TEST(test_MoneyComparisons);
    RUN_TEST(test_MoneyIsZero);
}

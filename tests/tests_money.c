#include "unity/unity.h"
#include "money.h"
#include <math.h>

void setUp(void) {}
void tearDown(void) {}

void test_MoneyFromMajor_Standard(void) {
    Money m = money_from_major(123.45);
    TEST_ASSERT_EQUAL_INT64(12345, m.value);
}

void test_MoneyFromMajor_Rounding(void) {
    Money m = money_from_major(123.456);
    TEST_ASSERT_EQUAL_INT64(12346, m.value);

    m = money_from_major(123.451);
    TEST_ASSERT_EQUAL_INT64(12345, m.value);
}

void test_MoneyToMajor(void) {
    Money m = {12345};
    long double val = money_to_major(m);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 123.45, val);
}

void test_MoneyAdd(void) {
    Money a = {12345};
    Money b = {54321};
    Money c = money_add(a, b);
    TEST_ASSERT_EQUAL_INT64(66666, c.value);
}

void test_MoneySub(void) {
    Money a = {54321};
    Money b = {12345};
    Money c = money_sub(a, b);
    TEST_ASSERT_EQUAL_INT64(41976, c.value);
}

void test_MoneySub_NegativeResult(void) {
    Money m1 = {1000};
    Money m2 = {2000};
    Money res = money_sub(m1, m2);
    TEST_ASSERT_EQUAL_INT64(-1000, res.value);
}

void test_MoneyMul_IntFactor(void) {
    Money m = {10000};
    Money res = money_mul(m, 5);
    TEST_ASSERT_EQUAL_INT64(50000, res.value);
}

void test_MoneyMul_FloatFactor(void) {
    Money m = {10000};
    Money res = money_mul(m, 1.0/3.0);
    TEST_ASSERT_EQUAL_INT64(3333, res.value);
}

void test_MoneyDiv(void) {
    Money m = {10000};
    Money res = money_div(m, 2);
    TEST_ASSERT_EQUAL_INT64(5000, res.value);
}

void test_MoneyDiv_ByZero(void) {
    Money m = {10000};
    Money res = money_div(m, 0);
    TEST_ASSERT_EQUAL_INT64(0, res.value);
}

void test_MoneyComparisons(void) {
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

void test_MoneyIsZero(void) {
    TEST_ASSERT_TRUE(money_is_zero(MONEY_ZERO));
    TEST_ASSERT_FALSE(money_is_zero(money_from_major(0.01)));
}

int main(void) {
    UNITY_BEGIN();

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

    return UNITY_END();
}
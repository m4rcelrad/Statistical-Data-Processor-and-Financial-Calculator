#include "unity/unity.h"
#include "statistics.h"
#include <math.h>
#include <stdlib.h>

void test_CalculateMean_ValidData(void) {
    double data[] = { 10.0, 20.0, 30.0, 40.0, 50.0 };
    double m = calculate_mean(data, 5);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 30.0, m);
}

void test_CalculateMean_WithNaN(void) {
    double data[] = { 10.0, NAN, 30.0 };
    double m = calculate_mean(data, 3);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 20.0, m);
}

void test_CalculateMean_NullOrEmpty(void) {
    TEST_ASSERT_TRUE(isnan(calculate_mean(NULL, 5)));
    double data[] = { 10.0 };
    TEST_ASSERT_TRUE(isnan(calculate_mean(data, 0)));
}

void test_CalculateStandardDeviation_ValidData(void) {
    double data[] = { 2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0 };
    double sigma = calculate_standard_deviation(data, 8);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 2.0, sigma);
}

void test_CalculateStandardDeviation_InsufficientData(void) {
    double data[] = { 10.0 };
    TEST_ASSERT_TRUE(isnan(calculate_standard_deviation(data, 1)));
    TEST_ASSERT_TRUE(isnan(calculate_standard_deviation(NULL, 5)));
}

void test_CalculateSMA(void) {
    double data[] = { 10.0, 20.0, 30.0, 40.0, 50.0 };
    double sma[5];

    calculate_sma(data, 5, 3, sma);

    TEST_ASSERT_TRUE(isnan(sma[0]));
    TEST_ASSERT_TRUE(isnan(sma[1]));
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 20.0, sma[2]);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 30.0, sma[3]);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 40.0, sma[4]);
}

void test_CalculateEMA(void) {
    double data[] = { 10.0, 20.0, 30.0 };
    double ema[3];

    calculate_ema(data, 3, 2, ema);

    TEST_ASSERT_DOUBLE_WITHIN(0.001, 10.0, ema[0]);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 16.666, ema[1]);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 25.555, ema[2]);
}

void test_GenerateTradingSignals(void) {
    double prices[] = { 10.0, 15.0, 12.0, 8.0 };
    double sma[]    = { 12.0, 14.0, 14.0, 10.0 };
    char *signals[4];

    generate_trading_signals(prices, sma, 4, signals);

    TEST_ASSERT_EQUAL_STRING("HOLD", signals[0]);
    TEST_ASSERT_EQUAL_STRING("BUY", signals[1]);
    TEST_ASSERT_EQUAL_STRING("SELL", signals[2]);
    TEST_ASSERT_EQUAL_STRING("HOLD", signals[3]);

    for (int i = 0; i < 4; i++) {
        if (signals[i]) free(signals[i]);
    }
}

void run_statistics_tests(void) {
    RUN_TEST(test_CalculateMean_ValidData);
    RUN_TEST(test_CalculateMean_WithNaN);
    RUN_TEST(test_CalculateMean_NullOrEmpty);
    RUN_TEST(test_CalculateStandardDeviation_ValidData);
    RUN_TEST(test_CalculateStandardDeviation_InsufficientData);
    RUN_TEST(test_CalculateSMA);
    RUN_TEST(test_CalculateEMA);
    RUN_TEST(test_GenerateTradingSignals);
}
#include "unity/unity.h"
#include "statistics.h"
#include <math.h>
#include <stdlib.h>

void test_CalculateMean_ValidData(void) {
    double data[] = { 10.0, 20.0, 30.0, 40.0, 50.0 };
    size_t length = sizeof(data) / sizeof(data[0]);
    double mean;

    StatisticsErrorCode err = calculate_mean(data, length, &mean);

    TEST_ASSERT_EQUAL_INT(STATS_SUCCESS, err);
    TEST_ASSERT_EQUAL_FLOAT(30.0, mean);
}

void test_CalculateMean_WithNaN(void) {
    double data[] = { 10.0, NAN, 30.0 };
    double mean;

    StatisticsErrorCode err = calculate_mean(data, 3, &mean);

    TEST_ASSERT_EQUAL_INT(STATS_SUCCESS, err);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 20.0, mean);
}

void test_CalculateMean_NullOrEmpty(void) {
    double mean;
    TEST_ASSERT_EQUAL_INT(STATS_ERR_NULL_POINTER, calculate_mean(NULL, 5, &mean));
    double data[] = { 10.0 };
    TEST_ASSERT_EQUAL_INT(STATS_ERR_INVALID_LENGTH, calculate_mean(data, 0, &mean));
}

void test_CalculateStandardDeviation_ValidData(void) {
    double data[] = { 2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0 };
    size_t length = sizeof(data) / sizeof(data[0]);
    double std_dev;

    StatisticsErrorCode err = calculate_standard_deviation(data, length, &std_dev);

    TEST_ASSERT_EQUAL_INT(STATS_SUCCESS, err);
    TEST_ASSERT_EQUAL_FLOAT(2.13808994, std_dev);
}

void test_CalculateStandardDeviation_InsufficientData(void) {
    double std_dev;
    double data[] = { 10.0, NAN, NAN };

    TEST_ASSERT_EQUAL_INT(STATS_ERR_INSUFFICIENT_DATA, calculate_standard_deviation(data, 1, &std_dev));
    TEST_ASSERT_EQUAL_INT(STATS_ERR_INSUFFICIENT_DATA, calculate_standard_deviation(data, 3, &std_dev));
}

void test_CalculateSMA(void) {
    double data[] = { 10.0, 20.0, 30.0, 40.0, 50.0 };
    double sma[5];
    size_t length = sizeof(data) / sizeof(data[0]);

    StatisticsErrorCode err = calculate_sma(data, length, 3, sma);

    TEST_ASSERT_EQUAL_INT(STATS_SUCCESS, err);
    TEST_ASSERT_TRUE(isnan(sma[0]));
    TEST_ASSERT_TRUE(isnan(sma[1]));
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 20.0, sma[2]);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 30.0, sma[3]);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 40.0, sma[4]);
}

void test_CalculateEMA(void) {
    double data[] = {10.0, 10.0, 10.0, 10.0, 10.0};
    int period = 3;
    double out[5];
    size_t length = sizeof(data) / sizeof(data[0]);

    StatisticsErrorCode err = calculate_ema(data, length, period, out);

    TEST_ASSERT_EQUAL_INT(STATS_SUCCESS, err);
    TEST_ASSERT_TRUE(isnan(out[0]));
    TEST_ASSERT_TRUE(isnan(out[1]));
    TEST_ASSERT_EQUAL_FLOAT(10.0, out[2]);
}

void test_GenerateTradingSignals(void) {
    double prices[] = { 10.0, 15.0, 12.0, 8.0 };
    double sma[]    = { 12.0, 14.0, 14.0, 10.0 };
    const char *signals[4];
    size_t length = sizeof(prices) / sizeof(prices[0]);

    StatisticsErrorCode err = generate_trading_signals(prices, sma, length, signals);

    TEST_ASSERT_EQUAL_INT(STATS_SUCCESS, err);
    TEST_ASSERT_EQUAL_STRING("HOLD", signals[0]);
    TEST_ASSERT_EQUAL_STRING("BUY", signals[1]);
    TEST_ASSERT_EQUAL_STRING("SELL", signals[2]);
    TEST_ASSERT_EQUAL_STRING("HOLD", signals[3]);
}

void test_CalculateSMA_Negative(void) {
    double data[] = { 10.0, 20.0 };
    double sma[2];

    TEST_ASSERT_EQUAL_INT(STATS_ERR_INSUFFICIENT_DATA, calculate_sma(data, 2, 3, sma));
    TEST_ASSERT_EQUAL_INT(STATS_ERR_INVALID_PERIOD, calculate_sma(data, 2, 0, sma));
    TEST_ASSERT_EQUAL_INT(STATS_ERR_INVALID_LENGTH, calculate_sma(data, 0, 3, sma));
}

void test_CalculateEMA_Negative(void) {
    double data[] = { 10.0, 20.0 };
    double ema[2];

    TEST_ASSERT_EQUAL_INT(STATS_ERR_INSUFFICIENT_DATA, calculate_ema(data, 2, 3, ema));

    double data_with_nan[] = { 10.0, NAN, 30.0 };
    double ema_nan[3];
    TEST_ASSERT_EQUAL_INT(STATS_ERR_INSUFFICIENT_DATA, calculate_ema(data_with_nan, 3, 3, ema_nan));
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
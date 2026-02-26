#include <math.h>
#include <stdlib.h>

#include "statistics.h"
#include "unity/unity.h"

/**
 * @file tests_statistics.c
 * @brief Unit tests for the statistical analysis module.
 *
 * This file verifies the correctness of mathematical and statistical algorithms,
 * including parameters of the normal distribution N(m, 𝜎) (mean and standard deviation),
 * moving averages, Bollinger Bands, covariance, correlation, and trading signals generation.
 */

/**
 * @brief Tests the calculation of core statistics (m and 𝜎) using valid, finite numbers.
 * Expected result: The arithmetic mean and standard deviation are calculated correctly.
 */
void test_CalculateSeriesStatistics_ValidData(void)
{
    double data[] = {2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0};
    size_t length = sizeof(data) / sizeof(data[0]);
    SeriesStatistics stats;

    StatisticsErrorCode err = calculate_series_statistics(data, length, &stats);

    TEST_ASSERT_EQUAL_INT(STATS_SUCCESS, err);
    TEST_ASSERT_EQUAL_FLOAT(5.0, stats.mean);
    TEST_ASSERT_EQUAL_FLOAT(2.13808994, stats.standard_deviation);
    TEST_ASSERT_EQUAL_FLOAT(4.57142857, stats.variance);
}

/**
 * @brief Tests statistics calculation when the input data contains NaN values.
 * Expected result: The NaN value is safely ignored, and the statistics are calculated
 * correctly based solely on the valid data points.
 */
void test_CalculateSeriesStatistics_WithNaN(void)
{
    double data[] = {10.0, NAN, 30.0};
    SeriesStatistics stats;

    StatisticsErrorCode err = calculate_series_statistics(data, 3, &stats);

    TEST_ASSERT_EQUAL_INT(STATS_SUCCESS, err);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 20.0, stats.mean);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 14.1421356, stats.standard_deviation);
}

/**
 * @brief Tests error handling with invalid pointer or zero-length inputs.
 * Expected result: The function gracefully fails and returns appropriate error codes.
 */
void test_CalculateSeriesStatistics_NullOrEmpty(void)
{
    SeriesStatistics stats;
    TEST_ASSERT_EQUAL_INT(STATS_ERR_NULL_POINTER, calculate_series_statistics(NULL, 5, &stats));

    double data[] = {10.0};
    TEST_ASSERT_EQUAL_INT(STATS_ERR_INVALID_LENGTH, calculate_series_statistics(data, 0, &stats));
}

/**
 * @brief Tests statistics calculation when there is only one valid data point.
 * Expected result: Mean is computed successfully, but variance and std_dev are set to NAN
 * to prevent mathematical errors.
 */
void test_CalculateSeriesStatistics_SinglePoint(void)
{
    double data[] = {10.0, NAN, NAN};
    SeriesStatistics stats;

    StatisticsErrorCode err = calculate_series_statistics(data, 3, &stats);

    TEST_ASSERT_EQUAL_INT(STATS_SUCCESS, err);
    TEST_ASSERT_EQUAL_FLOAT(10.0, stats.mean);
    TEST_ASSERT_TRUE(isnan(stats.standard_deviation));
    TEST_ASSERT_TRUE(isnan(stats.variance));
}

/**
 * @brief Tests the Simple Moving Average (SMA) calculation over a sliding window.
 */
void test_CalculateSMA(void)
{
    double data[] = {10.0, 20.0, 30.0, 40.0, 50.0};
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

/**
 * @brief Tests the Exponential Moving Average (EMA) calculation over a specific period.
 */
void test_CalculateEMA(void)
{
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

/**
 * @brief Tests the logic for generating standard trading signals (BUY/SELL/HOLD).
 */
void test_GenerateTradingSignals(void)
{
    double prices[] = {10.0, 15.0, 12.0, 8.0};
    double sma[] = {12.0, 14.0, 14.0, 10.0};
    const char *signals[4];
    size_t length = sizeof(prices) / sizeof(prices[0]);

    StatisticsErrorCode err = generate_trading_signals(prices, sma, length, signals);

    TEST_ASSERT_EQUAL_INT(STATS_SUCCESS, err);
    TEST_ASSERT_EQUAL_STRING("HOLD", signals[0]);
    TEST_ASSERT_EQUAL_STRING("BUY", signals[1]);
    TEST_ASSERT_EQUAL_STRING("SELL", signals[2]);
    TEST_ASSERT_EQUAL_STRING("HOLD", signals[3]);
}

/**
 * @brief Tests edge cases and invalid parameters for the SMA function.
 */
void test_CalculateSMA_Negative(void)
{
    double data[] = {10.0, 20.0};
    double sma[2];

    TEST_ASSERT_EQUAL_INT(STATS_ERR_INSUFFICIENT_DATA, calculate_sma(data, 2, 3, sma));
    TEST_ASSERT_EQUAL_INT(STATS_ERR_INVALID_PERIOD, calculate_sma(data, 2, 0, sma));
    TEST_ASSERT_EQUAL_INT(STATS_ERR_INVALID_LENGTH, calculate_sma(data, 0, 3, sma));
}

/**
 * @brief Tests edge cases and error handling for the EMA function.
 */
void test_CalculateEMA_Negative(void)
{
    double data[] = {10.0, 20.0};
    double ema[2];

    TEST_ASSERT_EQUAL_INT(STATS_ERR_INSUFFICIENT_DATA, calculate_ema(data, 2, 3, ema));

    double data_with_nan[] = {10.0, NAN, 30.0};
    double ema_nan[3];
    TEST_ASSERT_EQUAL_INT(STATS_ERR_INSUFFICIENT_DATA, calculate_ema(data_with_nan, 3, 3, ema_nan));
}

/**
 * @brief Tests the rolling standard deviation calculation over a sliding window.
 */
void test_CalculateRollingStd(void)
{
    double data[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double rolling_std[5];
    size_t length = sizeof(data) / sizeof(data[0]);

    StatisticsErrorCode err = calculate_rolling_std(data, length, 3, rolling_std);

    TEST_ASSERT_EQUAL_INT(STATS_SUCCESS, err);
    TEST_ASSERT_TRUE(isnan(rolling_std[0]));
    TEST_ASSERT_TRUE(isnan(rolling_std[1]));

    TEST_ASSERT_DOUBLE_WITHIN(0.001, 1.0, rolling_std[2]);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 1.0, rolling_std[3]);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 1.0, rolling_std[4]);
}

/**
 * @brief Tests the Bollinger Bands calculation based on N(m, 𝜎) distribution boundaries.
 */
void test_CalculateBollingerBands(void)
{
    double sma[] = {10.0, 20.0, 30.0};
    double std_dev[] = {1.0, 2.0, 3.0};
    double upper[3], lower[3];
    size_t length = 3;

    StatisticsErrorCode err = calculate_bollinger_bands(sma, std_dev, length, 2.0, upper, lower);

    TEST_ASSERT_EQUAL_INT(STATS_SUCCESS, err);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 12.0, upper[0]);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 8.0, lower[0]);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 24.0, upper[1]);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 16.0, lower[1]);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 36.0, upper[2]);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 24.0, lower[2]);
}

/**
 * @brief Tests the sample covariance calculation between two series.
 */
void test_CalculateCovariance(void)
{
    double data_x[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double data_y[] = {2.0, 4.0, 6.0, 8.0, 10.0};
    double covariance;
    size_t length = sizeof(data_x) / sizeof(data_x[0]);

    StatisticsErrorCode err = calculate_covariance(data_x, data_y, length, &covariance);

    TEST_ASSERT_EQUAL_INT(STATS_SUCCESS, err);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 5.0, covariance);
}

/**
 * @brief Tests the Pearson correlation calculation ensuring limits of -1.0 to 1.0.
 */
void test_CalculateCorrelation(void)
{
    double data_x[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double data_y_pos[] = {2.0, 4.0, 6.0, 8.0, 10.0};
    double data_y_neg[] = {5.0, 4.0, 3.0, 2.0, 1.0};
    double correlation;
    size_t length = sizeof(data_x) / sizeof(data_x[0]);

    StatisticsErrorCode err1 = calculate_correlation(data_x, data_y_pos, length, &correlation);
    TEST_ASSERT_EQUAL_INT(STATS_SUCCESS, err1);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 1.0, correlation);

    StatisticsErrorCode err2 = calculate_correlation(data_x, data_y_neg, length, &correlation);
    TEST_ASSERT_EQUAL_INT(STATS_SUCCESS, err2);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, -1.0, correlation);
}

/**
 * @brief Tests the correlation calculation when one series has zero variance.
 * Expected result: The mathematical domain error is caught, returning NaN.
 */
void test_CalculateCorrelation_ZeroVariance(void)
{
    double data_x[] = {5.0, 5.0, 5.0, 5.0, 5.0};
    double data_y[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double correlation;
    size_t length = sizeof(data_x) / sizeof(data_x[0]);

    StatisticsErrorCode err = calculate_correlation(data_x, data_y, length, &correlation);

    TEST_ASSERT_EQUAL_INT(STATS_SUCCESS, err);
    TEST_ASSERT_TRUE(isnan(correlation));
}

/**
 * @brief Test runner function that registers and executes all statistical module tests.
 */
void run_statistics_tests(void)
{
    RUN_TEST(test_CalculateSeriesStatistics_ValidData);
    RUN_TEST(test_CalculateSeriesStatistics_WithNaN);
    RUN_TEST(test_CalculateSeriesStatistics_NullOrEmpty);
    RUN_TEST(test_CalculateSeriesStatistics_SinglePoint);

    RUN_TEST(test_CalculateSMA);
    RUN_TEST(test_CalculateSMA_Negative);
    RUN_TEST(test_CalculateEMA);
    RUN_TEST(test_CalculateEMA_Negative);
    RUN_TEST(test_GenerateTradingSignals);

    RUN_TEST(test_CalculateRollingStd);
    RUN_TEST(test_CalculateBollingerBands);
    RUN_TEST(test_CalculateCovariance);
    RUN_TEST(test_CalculateCorrelation);
    RUN_TEST(test_CalculateCorrelation_ZeroVariance);
}
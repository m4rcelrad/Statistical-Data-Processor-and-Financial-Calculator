#ifndef STATISTICALDATAPROCESSOR_STATISTICS_H
#define STATISTICALDATAPROCESSOR_STATISTICS_H

#include <stddef.h>

/**
 * @file statistics.h
 * @brief Module for performing statistical and time-series analysis.
 *
 * Provides functions for basic descriptive statistics, moving averages (SMA, EMA),
 * Bollinger Bands, covariance, correlation, and trading signal generation.
 * For calculations dealing with the normal distribution, the standard notation
 * N(m, 𝜎) is assumed, where m is the mean and 𝜎 is the standard deviation.
 */

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

/**
 * @brief Error codes related to statistical calculations.
 */
typedef enum {
    STATS_SUCCESS = 0,        /*!< Operation completed successfully. */
    STATS_ERR_NULL_POINTER,   /*!< A required pointer parameter is NULL. */
    STATS_ERR_INVALID_LENGTH, /*!< The provided data length is invalid (e.g., zero). */
    STATS_ERR_INVALID_PERIOD, /*!< The specified period for a moving average/window is invalid. */
    STATS_ERR_INSUFFICIENT_DATA /*!< Not enough valid (non-NaN) data points to perform the calculation. */
} StatisticsErrorCode;

/**
 * @brief Descriptive statistics representing the normal distribution N(m, 𝜎).
 */
typedef struct {
    double mean;               /*!< The calculated mean (m) */
    double standard_deviation; /*!< The calculated standard deviation (𝜎) */
    double variance;           /*!< The calculated sample variance (𝜎^2) */
} SeriesStatistics;

/**
 * @brief Calculates comprehensive descriptive statistics (mean, variance, and standard deviation).
 * These parameters describe the normal distribution N(m, 𝜎) of the provided dataset.
 * @param data Array of double-precision input values.
 * @param length Total number of elements in the data array.
 * @param out_stats Pointer to the SeriesStatistics structure where results will be stored.
 * @return STATS_SUCCESS on success, or an error code.
 */
StatisticsErrorCode calculate_series_statistics(const double *restrict data,
                                                size_t length,
                                                SeriesStatistics *restrict out_stats);

/**
 * @brief Calculates the Simple Moving Average (SMA) over a given period.
 * @param data Array of input values (e.g., stock prices).
 * @param length Total number of elements in the array.
 * @param period The sliding window size for the average.
 * @param out_sma Array where the resulting SMA values will be stored.
 * @return STATS_SUCCESS on success, or an error code.
 */
StatisticsErrorCode
calculate_sma(const double *restrict data, size_t length, int period, double *restrict out_sma);

/**
 * @brief Calculates the Exponential Moving Average (EMA) over a given period.
 * @param data Array of input values.
 * @param length Total number of elements in the array.
 * @param period The smoothing period for the EMA calculation.
 * @param out_ema Array where the resulting EMA values will be stored.
 * @return STATS_SUCCESS on success, or an error code.
 */
StatisticsErrorCode
calculate_ema(const double *restrict data, size_t length, int period, double *restrict out_ema);

/**
 * @brief Generates standard trading signals (BUY, SELL, HOLD) based on price vs. SMA crossovers.
 * @param prices Array of input price values.
 * @param sma Array of corresponding SMA values.
 * @param length Total number of elements in the arrays.
 * @param out_signals Array of string pointers where the resulting signals will be stored.
 * @return STATS_SUCCESS on success, or an error code.
 */
StatisticsErrorCode generate_trading_signals(const double *restrict prices,
                                             const double *restrict sma,
                                             size_t length,
                                             const char **restrict out_signals);

/**
 * @brief Calculates the rolling sample standard deviation (𝜎) over a sliding window.
 * @param data Array of double-precision input values.
 * @param length Total number of elements in the data array.
 * @param period The sliding window size.
 * @param out_std Array where the resulting rolling standard deviation values will be stored.
 * @return STATS_SUCCESS on success, or an error code.
 */
StatisticsErrorCode calculate_rolling_std(const double *restrict data,
                                          size_t length,
                                          int period,
                                          double *restrict out_std);

/**
 * @brief Calculates Bollinger Bands based on a given SMA (m) and rolling standard deviation (𝜎).
 * The bands describe the dynamic boundaries of the N(m, 𝜎) normal distribution.
 * @param sma Pre-calculated Simple Moving Average array (m).
 * @param rolling_std Pre-calculated Rolling Standard Deviation array (𝜎).
 * @param length Total number of elements in the arrays.
 * @param k The standard deviation multiplier (usually 2.0).
 * @param out_upper Array to store the upper band values (m + k*𝜎).
 * @param out_lower Array to store the lower band values (m - k*𝜎).
 * @return STATS_SUCCESS on success, or an error code.
 */
StatisticsErrorCode calculate_bollinger_bands(const double *restrict sma,
                                              const double *restrict rolling_std,
                                              size_t length,
                                              double k,
                                              double *restrict out_upper,
                                              double *restrict out_lower);

/**
 * @brief Calculates the sample covariance between two distinct time series.
 * Automatically aligns and ignores pairs where at least one value is NaN.
 * @param data_x First array of input values.
 * @param data_y Second array of input values.
 * @param length Number of elements in both arrays.
 * @param out_covariance Pointer to store the resulting sample covariance.
 * @return STATS_SUCCESS on success, or an error code.
 */
StatisticsErrorCode calculate_covariance(const double *restrict data_x,
                                         const double *restrict data_y,
                                         size_t length,
                                         double *restrict out_covariance);

/**
 * @brief Calculates the Pearson correlation coefficient between two time series.
 * Mapped between -1.0 (perfect inverse correlation) and 1.0 (perfect correlation).
 * @param data_x First array of input values.
 * @param data_y Second array of input values.
 * @param length Number of elements in both arrays.
 * @param out_correlation Pointer to store the resulting correlation.
 * @return STATS_SUCCESS on success, or an error code.
 */
StatisticsErrorCode calculate_correlation(const double *restrict data_x,
                                          const double *restrict data_y,
                                          size_t length,
                                          double *restrict out_correlation);

#endif

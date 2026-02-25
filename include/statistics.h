#ifndef STATISTICALDATAPROCESSOR_STATISTICS_H
#define STATISTICALDATAPROCESSOR_STATISTICS_H

#include <stddef.h>

/**
 * @file statistics.h
 * @brief Module for performing statistical and time-series analysis.
 *
 * Provides functions for basic descriptive statistics, moving averages (SMA, EMA),
 * and trading signal generation. For calculations dealing with the normal distribution,
 * the standard notation N(m, 𝜎) is assumed, where m is the mean and 𝜎 is the standard deviation.
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
    STATS_ERR_INVALID_PERIOD, /*!< The specified period for a moving average is invalid. */
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
 * @brief Calculates the core descriptive statistics (m, 𝜎, and variance) in a single pass.
 * @param data Array of double-precision input values.
 * @param length The number of elements in the data array.
 * @param out_stats Pointer where the calculated SeriesStatistics will be stored.
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
 * @param out_sma Array where the resulting SMA values will be stored. It must have at least `length` capacity.
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

#endif

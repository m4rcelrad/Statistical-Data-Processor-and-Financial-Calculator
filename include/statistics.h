#ifndef STATISTICALDATAPROCESSOR_STATISTICS_H
#define STATISTICALDATAPROCESSOR_STATISTICS_H

#include <stddef.h>

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

typedef enum {
    STATS_SUCCESS = 0,
    STATS_ERR_NULL_POINTER,
    STATS_ERR_INVALID_LENGTH,
    STATS_ERR_INVALID_PERIOD,
    STATS_ERR_INSUFFICIENT_DATA
} StatisticsErrorCode;

StatisticsErrorCode calculate_mean(const double *restrict data, size_t length, double *restrict out_mean);
StatisticsErrorCode calculate_standard_deviation(const double *restrict data, size_t length, double *restrict out_std);
StatisticsErrorCode calculate_sma(const double *restrict data, size_t length, int period, double *restrict out_sma);
StatisticsErrorCode calculate_ema(const double *restrict data, size_t length, int period, double *restrict out_ema);
StatisticsErrorCode generate_trading_signals(const double *restrict prices, const double *restrict sma, size_t length, const char **restrict out_signals);

#endif
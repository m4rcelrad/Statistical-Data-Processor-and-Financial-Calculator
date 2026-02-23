#ifndef STATISTICALDATAPROCESSOR_STATISTICS_H
#define STATISTICALDATAPROCESSOR_STATISTICS_H

#include <stddef.h>

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

double calculate_mean(const double *data, size_t length);
double calculate_standard_deviation(const double *data, size_t length);
void calculate_sma(const double *data, size_t length, int period, double *out_sma);
void calculate_ema(const double *data, size_t length, int period, double *out_ema);
void generate_trading_signals(const double *prices, const double *sma, size_t length, const char **out_signals);

#endif
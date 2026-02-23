#ifndef STATISTICALDATAPROCESSOR_STATISTICS_H
#define STATISTICALDATAPROCESSOR_STATISTICS_H

double calculate_mean(const double *data, int length);
double calculate_standard_deviation(const double *data, int length);
void calculate_sma(const double *data, int length, int period, double *out_sma);
void calculate_ema(const double *data, int length, int period, double *out_ema);
void generate_trading_signals(const double *prices, const double *sma, int length, const char **out_signals);

#endif
#include "statistics.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static ssize_t calculate_welford_stats(const double *data, const size_t length, double *out_mean, double *out_m2) {
    if (!data) return -1;
    if (length == 0) return 0;

    double mean = 0.0;
    double m2 = 0.0;
    size_t count = 0;

    for (size_t i = 0; i < length; i++) {
        const double x = data[i];
        if (!isnan(x)) {
            count++;
            const double delta = x - mean;
            mean += delta / (double)count;
            m2 += delta * (x - mean);
        }
    }

    *out_mean = count > 0 ? mean : NAN;
    if (out_m2) *out_m2 = count > 0 ? m2 : NAN;

    return (ssize_t)count;
}

double calculate_mean(const double *data, const size_t length) {
    double mean;
    if (calculate_welford_stats(data, length, &mean, NULL) <= 0) return NAN;
    return mean;
}

double calculate_standard_deviation(const double *data, const size_t length) {
    double mean, m2;
    const ssize_t count = calculate_welford_stats(data, length, &mean, &m2);

    if (count <= 1) return NAN;

    return sqrt(m2 / (double)(count - 1));
}

void calculate_sma(const double *data, const size_t length, const int period, double *out_sma) {
    if (!data || !out_sma || length == 0 || period <= 0 || length < (size_t)period) return;

    double window_sum = 0.0;
    size_t nan_in_window = 0;
    const size_t u_period = (size_t)period;

    for (size_t i = 0; i < length; i++) {
        if (isnan(data[i])) {
            nan_in_window++;
        } else {
            window_sum += data[i];
        }

        if (i >= u_period) {
            const double old_val = data[i - u_period];
            if (isnan(old_val)) {
                nan_in_window--;
            } else {
                window_sum -= old_val;
            }
        }

        if (i < u_period - 1) {
            out_sma[i] = NAN;
        } else {
            out_sma[i] = nan_in_window > 0 ? NAN : window_sum / (double)period;
        }
    }
}



void calculate_ema(const double *data, const size_t length, const int period, double *out_ema) {
    if (!data || !out_ema || length == 0 || period <= 0 || length < (size_t)period) return;

    const double multiplier = 2.0 / (period + 1.0);
    const size_t u_period = (size_t)period;
    double initial_sma_sum = 0.0;

    for (size_t i = 0; i < u_period - 1; i++) {
        out_ema[i] = NAN;
    }

    for (size_t i = 0; i < u_period; i++) {
        if (isnan(data[i])) {
            for (size_t j = 0; j < length; j++) out_ema[j] = NAN;
            return;
        }
        initial_sma_sum += data[i];
    }

    double current_ema = initial_sma_sum / (double)period;
    out_ema[u_period - 1] = current_ema;

    for (size_t i = u_period; i < length; i++) {
        if (isnan(data[i])) {
            out_ema[i] = NAN;
        } else {
            current_ema = (data[i] - current_ema) * multiplier + current_ema;
            out_ema[i] = current_ema;
        }
    }
}

void generate_trading_signals(const double *prices, const double *sma, const size_t length, const char **out_signals) {
    if (!prices || !sma || !out_signals || length == 0) return;

    out_signals[0] = "HOLD";

    for (size_t i = 1; i < length; i++) {
        if (isnan(sma[i]) || isnan(sma[i-1]) || isnan(prices[i]) || isnan(prices[i-1])) {
            out_signals[i] = "HOLD";
            continue;
        }

        if (prices[i - 1] <= sma[i - 1] && prices[i] > sma[i]) {
            out_signals[i] = "BUY";
        }
        else if (prices[i - 1] >= sma[i - 1] && prices[i] < sma[i]) {
            out_signals[i] = "SELL";
        }
        else {
            out_signals[i] = "HOLD";
        }
    }
}
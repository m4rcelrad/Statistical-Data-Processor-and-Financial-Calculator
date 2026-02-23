#include "statistics.h"
#include <math.h>
#include <string.h>

double calculate_mean(const double *data, const int length) {
    if (!data || length <= 0) return NAN;

    double sum = 0.0;
    int valid_count = 0;
    for (int i = 0; i < length; i++) {
        if (!isnan(data[i])) {
            sum += data[i];
            valid_count++;
        }
    }

    if (valid_count == 0) return NAN;
    return sum / valid_count;

}

double calculate_standard_deviation(const double *data, const int length) {
    if (!data || length <= 0) return NAN;

    const double m = calculate_mean(data, length);

    if (isnan(m)) return NAN;

    double sum_sq_diff = 0.0;
    int valid_count = 0;
    for (int i = 0; i < length; i++) {
        if (!isnan(data[i])) {
            const double diff = data[i] - m;
            sum_sq_diff += diff * diff;
            valid_count++;
        }
    }

    if (valid_count <= 1) return NAN;
    const double variance = sum_sq_diff / valid_count;
    const double sigma = sqrt(variance);

    return sigma;
}

void calculate_sma(const double *data, const int length, const int period, double *out_sma) {
    if (!data || !out_sma || length <= 0 || period <= 0) return;

    for (int i = 0; i < length; i++) {
        if (i < period - 1) {
            out_sma[i] = NAN;
        } else {
            double sum = 0.0;
            int valid_count = 0;
            for (int j = 0; j < period; j++) {
                if (!isnan(data[i - j])) {
                    sum += data[i - j];
                    valid_count++;
                }
            }
            if (valid_count == period) {
                out_sma[i] = sum / period;
            } else {
                out_sma[i] = NAN;
            }
        }
    }
}

void calculate_ema(const double *data, const int length, const int period, double *out_ema) {
    if (!data || !out_ema || length <= 0 || period <= 0) return;

    const double multiplier = 2.0 / (period + 1.0);
    int start_idx = 0;

    while (start_idx < length && isnan(data[start_idx])) {
        out_ema[start_idx] = NAN;
        start_idx++;
    }

    if (start_idx < length) {
        out_ema[start_idx] = data[start_idx];
        for (int i = start_idx + 1; i < length; i++) {
            if (isnan(data[i]) || isnan(out_ema[i - 1])) {
                out_ema[i] = NAN;
            } else {
                out_ema[i] = (data[i] - out_ema[i - 1]) * multiplier + out_ema[i - 1];
            }
        }
    }
}

void generate_trading_signals(const double *prices, const double *sma, const int length, char **out_signals) {
    if (!prices || !sma || !out_signals || length <= 0) return;

    out_signals[0] = strdup("HOLD");

    for (int i = 1; i < length; i++) {
        if (isnan(sma[i]) || isnan(sma[i-1]) || isnan(prices[i]) || isnan(prices[i-1])) {
            out_signals[i] = strdup("HOLD");
            continue;
        }

        if (prices[i - 1] < sma[i - 1] && prices[i] > sma[i]) {
            out_signals[i] = strdup("BUY");
        }
        else if (prices[i - 1] > sma[i - 1] && prices[i] < sma[i]) {
            out_signals[i] = strdup("SELL");
        }
        else {
            out_signals[i] = strdup("HOLD");
        }
    }
}
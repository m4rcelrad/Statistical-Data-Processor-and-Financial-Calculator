#include "statistics.h"
#include <math.h>
#include <string.h>

static int calculate_welford_stats(const double *data, const int length, double *out_mean, double *out_m2) {
    double mean = 0.0;
    double m2 = 0.0;
    int count = 0;

    if (!data || length <= 0) return 0;

    for (int i = 0; i < length; i++) {
        const double x = data[i];
        if (!isnan(x)) {
            count++;
            const double delta = x - mean;
            mean += delta / count;
            m2 += delta * (x - mean);
        }
    }

    *out_mean = count > 0 ? mean : NAN;
    if (out_m2) *out_m2 = count > 0 ? m2 : NAN;

    return count;
}

double calculate_mean(const double *data, const int length) {
    double mean;
    const int count = calculate_welford_stats(data, length, &mean, NULL);
    return count > 0 ? mean : NAN;
}


double calculate_standard_deviation(const double *data, const int length) {
    double mean, m2;
    const int count = calculate_welford_stats(data, length, &mean, &m2);

    if (count <= 1) return NAN;

    return sqrt(m2 / (count - 1));
}

void calculate_sma(const double *data, const int length, const int period, double *out_sma) {
    if (!data || !out_sma || length <= 0 || period <= 0 || length < period) return;

    double window_sum = 0.0;
    int nan_in_window = 0;

    for (int i = 0; i < length; i++) {
        if (isnan(data[i])) {
            nan_in_window++;
        } else {
            window_sum += data[i];
        }

        if (i >= period) {
            const double old_val = data[i - period];
            if (isnan(old_val)) {
                nan_in_window--;
            } else {
                window_sum -= old_val;
            }
        }

        if (i < period - 1) {
            out_sma[i] = NAN;
        } else {
            if (nan_in_window > 0) {
                out_sma[i] = NAN;
            } else {
                out_sma[i] = window_sum / period;
            }
        }
    }
}

void calculate_ema(const double *data, const int length, const int period, double *out_ema) {
    if (!data || !out_ema || length <= 0 || period <= 0 || length < period) return;

    const double multiplier = 2.0 / (period + 1.0);
    double initial_sma_sum = 0.0;

    for (int i = 0; i < period - 1; i++) {
        out_ema[i] = NAN;
    }

    for (int i = 0; i < period; i++) {
        if (isnan(data[i])) {
            for (int j = 0; j < length; j++) out_ema[j] = NAN;
            return;
        }
        initial_sma_sum += data[i];
    }

    double current_ema = initial_sma_sum / period;
    out_ema[period - 1] = current_ema;

    for (int i = period; i < length; i++) {
        if (isnan(data[i])) {
            out_ema[i] = NAN;
        } else {
            current_ema = (data[i] - current_ema) * multiplier + current_ema;
            out_ema[i] = current_ema;
        }
    }
}

void generate_trading_signals(const double *prices, const double *sma, const int length, const char **out_signals) {
    if (!prices || !sma || !out_signals || length <= 0) return;

    out_signals[0] = "HOLD";

    for (int i = 1; i < length; i++) {
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
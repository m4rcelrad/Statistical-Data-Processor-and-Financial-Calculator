#include "statistics.h"
#include <math.h>

static StatisticsErrorCode calculate_welford_stats(const double *data, const size_t length, double *out_mean, double *out_m2, size_t *out_count) {
    if (!data) return STATS_ERR_NULL_POINTER;
    if (length == 0) return STATS_ERR_INVALID_LENGTH;

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

    if (count == 0) return STATS_ERR_INSUFFICIENT_DATA;

    *out_mean = mean;
    if (out_m2) *out_m2 = m2;
    if (out_count) *out_count = count;

    return STATS_SUCCESS;
}

StatisticsErrorCode calculate_mean(const double *data, const size_t length, double *out_mean) {
    if (!out_mean) return STATS_ERR_NULL_POINTER;
    return calculate_welford_stats(data, length, out_mean, NULL, NULL);
}

StatisticsErrorCode calculate_standard_deviation(const double *data, size_t length, double *out_std) {
    if (!out_std) return STATS_ERR_NULL_POINTER;

    double mean, m2;
    size_t count;
    const StatisticsErrorCode err = calculate_welford_stats(data, length, &mean, &m2, &count);

    if (err != STATS_SUCCESS) return err;
    if (count <= 1) return STATS_ERR_INSUFFICIENT_DATA;

    *out_std = sqrt(m2 / (double)(count - 1));
    return STATS_SUCCESS;
}

StatisticsErrorCode calculate_sma(const double *data, const size_t length, const int period, double *out_sma) {
    if (!data || !out_sma) return STATS_ERR_NULL_POINTER;
    if (length == 0) return STATS_ERR_INVALID_LENGTH;
    if (period <= 0) return STATS_ERR_INVALID_PERIOD;
    if (length < (size_t)period) return STATS_ERR_INSUFFICIENT_DATA;

    double window_sum = 0.0;
    size_t nan_count = 0;
    const size_t u_period = (size_t)period;

    for (size_t i = 0; i < length; i++) {
        if (isnan(data[i])) nan_count++;
        else window_sum += data[i];

        if (i >= u_period) {
            if (isnan(data[i - u_period])) nan_count--;
            else window_sum -= data[i - u_period];
        }

        if (i < u_period - 1) out_sma[i] = NAN;
        else out_sma[i] = (nan_count > 0) ? NAN : (window_sum / (double)period);
    }
    return STATS_SUCCESS;
}

StatisticsErrorCode calculate_ema(const double *data, const size_t length, const int period, double *out_ema) {
    if (!data || !out_ema) return STATS_ERR_NULL_POINTER;
    if (length == 0 || length < (size_t)period) return STATS_ERR_INVALID_LENGTH;
    if (period <= 0) return STATS_ERR_INSUFFICIENT_DATA;

    const double multiplier = 2.0 / (period + 1.0);
    double current_sum = 0.0;

    for (size_t i = 0; i < (size_t)period - 1; i++) out_ema[i] = NAN;

    for (size_t i = 0; i < (size_t)period; i++) {
        if (isnan(data[i])) {
            for (size_t j = 0; j < length; j++) out_ema[j] = NAN;
            return STATS_ERR_INSUFFICIENT_DATA;
        }
        current_sum += data[i];
    }

    double current_ema = current_sum / (double)period;
    out_ema[period - 1] = current_ema;

    for (size_t i = (size_t)period; i < length; i++) {
        if (isnan(data[i])) {
            out_ema[i] = NAN;
        } else {
            current_ema = (data[i] - current_ema) * multiplier + current_ema;
            out_ema[i] = current_ema;
        }
    }
    return STATS_SUCCESS;
}

StatisticsErrorCode generate_trading_signals(const double *prices, const double *sma, const size_t length, const char **out_signals) {
    if (!prices || !sma || !out_signals) return STATS_ERR_NULL_POINTER;
    if (length == 0) return STATS_ERR_INVALID_LENGTH;

    out_signals[0] = "HOLD";
    for (size_t i = 1; i < length; i++) {
        if (isnan(sma[i]) || isnan(sma[i-1]) || isnan(prices[i]) || isnan(prices[i-1])) {
            out_signals[i] = "HOLD";
            continue;
        }
        if (prices[i - 1] <= sma[i - 1] && prices[i] > sma[i]) out_signals[i] = "BUY";
        else if (prices[i - 1] >= sma[i - 1] && prices[i] < sma[i]) out_signals[i] = "SELL";
        else out_signals[i] = "HOLD";
    }
    return STATS_SUCCESS;
}
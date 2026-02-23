#include "statistics.h"
#include <math.h>

static StatisticsErrorCode calculate_welford_stats(const double *restrict data, size_t length,
                                                   double *restrict out_mean, double *restrict out_m2,
                                                   size_t *restrict out_count) {
    if (!data || !out_mean) return STATS_ERR_NULL_POINTER;
    if (length == 0) return STATS_ERR_INVALID_LENGTH;

    double mean = 0.0;
    double m2 = 0.0;
    size_t count = 0;

    for (size_t i = 0; i < length; i++) {
        const double x = data[i];
        if (!isnan(x)) {
            count++;
            const double delta = x - mean;
            mean += delta / (double) count;
            m2 += delta * (x - mean);
        }
    }

    if (count == 0)
        return STATS_ERR_INSUFFICIENT_DATA;

    *out_mean = mean;
    if (out_m2)
        *out_m2 = m2;
    if (out_count)
        *out_count = count;

    return STATS_SUCCESS;
}

StatisticsErrorCode calculate_mean(const double *restrict data, const size_t length, double *restrict out_mean) {
    if (!out_mean) return STATS_ERR_NULL_POINTER;
    return calculate_welford_stats(data, length, out_mean, NULL, NULL);
}

StatisticsErrorCode calculate_standard_deviation(const double *restrict data, size_t length, double *restrict out_std) {
    if (!out_std) return STATS_ERR_NULL_POINTER;

    double mean, m2;
    size_t count;
    const StatisticsErrorCode err = calculate_welford_stats(data, length, &mean, &m2, &count);

    if (err != STATS_SUCCESS) return err;
    if (count <= 1) return STATS_ERR_INSUFFICIENT_DATA;

    *out_std = sqrt(m2 / (double) (count - 1));
    return STATS_SUCCESS;
}

StatisticsErrorCode calculate_sma(const double *restrict data, size_t length, int period, double *restrict out_sma) {
    if (!data || !out_sma)
        return STATS_ERR_NULL_POINTER;
    if (length == 0)
        return STATS_ERR_INVALID_LENGTH;
    if (period <= 0)
        return STATS_ERR_INVALID_PERIOD;
    if (length < (size_t) period)
        return STATS_ERR_INSUFFICIENT_DATA;

    double window_sum = 0.0;
    size_t nan_count = 0;
    const size_t u_period = (size_t) period;

    for (size_t i = 0; i < length; i++) {
        if (isnan(data[i]))
            nan_count++;
        else
            window_sum += data[i];

        if (i >= u_period) {
            if (isnan(data[i - u_period]))
                nan_count--;
            else
                window_sum -= data[i - u_period];
        }

        if (i < u_period - 1)
            out_sma[i] = NAN;
        else
            out_sma[i] = (nan_count > 0) ? NAN : (window_sum / (double) period);
    }
    return STATS_SUCCESS;
}

StatisticsErrorCode calculate_ema(const double *restrict data, size_t length, int period, double *restrict out_ema) {
    if (!data || !out_ema)
        return STATS_ERR_NULL_POINTER;
    if (period <= 0)
        return STATS_ERR_INVALID_PERIOD;
    if (length == 0 || length < (size_t) period)
        return STATS_ERR_INSUFFICIENT_DATA;

    const double multiplier = 2.0 / ((double)period + 1.0);
    size_t valid_streak = 0;
    double current_sum = 0.0;
    double current_ema = NAN;
    int has_valid_output = 0;

    for (size_t i = 0; i < length; i++) {
        if (isnan(data[i])) {
            out_ema[i] = NAN;
            valid_streak = 0;
            current_sum = 0.0;
            current_ema = NAN;
        } else {
            if (valid_streak < (size_t) period) {
                current_sum += data[i];
                valid_streak++;

                if (valid_streak == (size_t) period) {
                    current_ema = current_sum / (double) period;
                    out_ema[i] = current_ema;
                    has_valid_output = 1;
                } else {
                    out_ema[i] = NAN;
                }
            } else {
                current_ema = (data[i] - current_ema) * multiplier + current_ema;
                out_ema[i] = current_ema;
                has_valid_output = 1;
            }
        }
    }

    if (!has_valid_output) return STATS_ERR_INSUFFICIENT_DATA;
    return STATS_SUCCESS;
}

StatisticsErrorCode generate_trading_signals(const double *restrict prices, const double *restrict sma,
                                             const size_t length, const char **restrict out_signals) {
    if (!prices || !sma || !out_signals)
        return STATS_ERR_NULL_POINTER;
    if (length == 0)
        return STATS_ERR_INVALID_LENGTH;

    out_signals[0] = "HOLD";
    for (size_t i = 1; i < length; i++) {
        if (isnan(sma[i]) || isnan(sma[i - 1]) || isnan(prices[i]) || isnan(prices[i - 1])) {
            out_signals[i] = "HOLD";
            continue;
        }
        if (prices[i - 1] <= sma[i - 1] && prices[i] > sma[i])
            out_signals[i] = "BUY";
        else if (prices[i - 1] >= sma[i - 1] && prices[i] < sma[i])
            out_signals[i] = "SELL";
        else
            out_signals[i] = "HOLD";
    }
    return STATS_SUCCESS;
}

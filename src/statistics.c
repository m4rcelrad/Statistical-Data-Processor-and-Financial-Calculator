#include "statistics.h"

#include <math.h>

/**
 * @brief Internal helper using Welford's online algorithm for computing variance.
 * Welford's algorithm calculates the mean (m) and the sum of squared differences
 * in a single pass, which is numerically stable and avoids catastrophic cancellation.
 * @param data Array of input values.
 * @param length Number of elements in the array.
 * @param out_mean Pointer to store the calculated mean.
 * @param out_m2 Pointer to store the sum of squared differences from the mean (can be NULL).
 * @param out_count Pointer to store the number of valid non-NaN elements processed (can be NULL).
 * @return STATS_SUCCESS or a relevant error code.
 */
static StatisticsErrorCode calculate_welford_stats(const double *restrict data,
                                                   const size_t length,
                                                   double *restrict out_mean,
                                                   double *restrict out_m2,
                                                   size_t *restrict out_count)
{
    if (!data || !out_mean)
        return STATS_ERR_NULL_POINTER;
    if (length == 0)
        return STATS_ERR_INVALID_LENGTH;

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

    if (count == 0)
        return STATS_ERR_INSUFFICIENT_DATA;

    *out_mean = mean;
    if (out_m2)
        *out_m2 = m2;
    if (out_count)
        *out_count = count;

    return STATS_SUCCESS;
}

StatisticsErrorCode calculate_series_statistics(const double *restrict data,
                                                const size_t length,
                                                SeriesStatistics *restrict out_stats)
{
    if (!out_stats)
        return STATS_ERR_NULL_POINTER;

    double m, m2;
    size_t count;

    const StatisticsErrorCode err = calculate_welford_stats(data, length, &m, &m2, &count);
    if (err != STATS_SUCCESS)
        return err;

    out_stats->mean = m;

    if (count > 1) {
        out_stats->variance = m2 / (double)(count - 1);
        out_stats->standard_deviation = sqrt(out_stats->variance);
    } else {
        out_stats->variance = NAN;
        out_stats->standard_deviation = NAN;
    }

    return STATS_SUCCESS;
}

StatisticsErrorCode calculate_sma(const double *restrict data,
                                  const size_t length,
                                  const int period,
                                  double *restrict out_sma)
{
    if (!data || !out_sma)
        return STATS_ERR_NULL_POINTER;
    if (length == 0)
        return STATS_ERR_INVALID_LENGTH;
    if (period <= 0)
        return STATS_ERR_INVALID_PERIOD;
    if (length < (size_t)period)
        return STATS_ERR_INSUFFICIENT_DATA;

    double window_sum = 0.0;
    size_t nan_count = 0;
    const size_t u_period = (size_t)period;

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
            out_sma[i] = nan_count > 0 ? NAN : window_sum / (double)period;
    }
    return STATS_SUCCESS;
}

StatisticsErrorCode calculate_ema(const double *restrict data,
                                  const size_t length,
                                  const int period,
                                  double *restrict out_ema)
{
    if (!data || !out_ema)
        return STATS_ERR_NULL_POINTER;
    if (period <= 0)
        return STATS_ERR_INVALID_PERIOD;
    if (length == 0 || length < (size_t)period)
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
            if (valid_streak < (size_t)period) {
                current_sum += data[i];
                valid_streak++;

                if (valid_streak == (size_t)period) {
                    current_ema = current_sum / (double)period;
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

    if (!has_valid_output)
        return STATS_ERR_INSUFFICIENT_DATA;
    return STATS_SUCCESS;
}

StatisticsErrorCode generate_trading_signals(const double *restrict prices,
                                             const double *restrict sma,
                                             const size_t length,
                                             const char **restrict out_signals)
{
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

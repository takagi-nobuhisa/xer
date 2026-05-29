/**
 * @file xer/bits/statistics.h
 * @brief Descriptive statistical utility functions.
 */

#pragma once

#ifndef XER_BITS_STATISTICS_H_INCLUDED_
#define XER_BITS_STATISTICS_H_INCLUDED_

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <expected>
#include <initializer_list>
#include <limits>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

#include <xer/bits/arithmetic_concepts.h>
#include <xer/error.h>

namespace xer::detail {

struct statistics_summary {
    std::size_t count{};
    long double mean{};
    long double m2{};
};

struct statistics_mode_group {
    std::size_t count{};
    long double sum{};
};

struct statistics_product_summary {
    std::size_t count{};
    long double product{1.0L};
};

struct statistics_geometric_summary {
    std::size_t count{};
    bool has_zero{};
    long double log_sum{};
};

struct statistics_harmonic_summary {
    std::size_t count{};
    long double reciprocal_sum{};
};

[[nodiscard]] inline auto statistics_is_finite(long double value) noexcept -> bool
{
    return std::isfinite(value);
}

[[nodiscard]] inline auto statistics_to_double(long double value)
    -> result<double>
{
    if (!statistics_is_finite(value) ||
        value > static_cast<long double>(std::numeric_limits<double>::max()) ||
        value < static_cast<long double>(std::numeric_limits<double>::lowest())) {
        return std::unexpected(make_error(error_t::range_error));
    }

    return static_cast<double>(value);
}

template<class Range>
concept statistics_range =
    std::ranges::input_range<Range> &&
    non_bool_arithmetic<
        std::remove_cvref_t<std::ranges::range_reference_t<Range>>>;


template<statistics_range Range>
[[nodiscard]] auto sum_statistics_values(Range&& range) -> result<long double>
{
    std::size_t count{};
    long double total{};

    for (auto&& element : range) {
        const long double value = static_cast<long double>(element);
        if (!statistics_is_finite(value)) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        ++count;
        total += value;

        if (!statistics_is_finite(total)) {
            return std::unexpected(make_error(error_t::range_error));
        }
    }

    if (count == 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return total;
}

template<statistics_range Range>
[[nodiscard]] auto product_statistics_values(Range&& range)
    -> result<statistics_product_summary>
{
    statistics_product_summary summary{};

    for (auto&& element : range) {
        const long double value = static_cast<long double>(element);
        if (!statistics_is_finite(value)) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        ++summary.count;
        summary.product *= value;

        if (!statistics_is_finite(summary.product)) {
            return std::unexpected(make_error(error_t::range_error));
        }
    }

    if (summary.count == 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return summary;
}

template<statistics_range Range>
[[nodiscard]] auto summarize_geometric_statistics(Range&& range)
    -> result<statistics_geometric_summary>
{
    statistics_geometric_summary summary{};

    for (auto&& element : range) {
        const long double value = static_cast<long double>(element);
        if (!statistics_is_finite(value) || value < 0.0L) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        ++summary.count;
        if (value == 0.0L) {
            summary.has_zero = true;
            continue;
        }

        summary.log_sum += std::log(value);
        if (!statistics_is_finite(summary.log_sum)) {
            return std::unexpected(make_error(error_t::range_error));
        }
    }

    if (summary.count == 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return summary;
}

template<statistics_range Range>
[[nodiscard]] auto summarize_harmonic_statistics(Range&& range)
    -> result<statistics_harmonic_summary>
{
    statistics_harmonic_summary summary{};

    for (auto&& element : range) {
        const long double value = static_cast<long double>(element);
        if (!statistics_is_finite(value) || value <= 0.0L) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        ++summary.count;
        summary.reciprocal_sum += 1.0L / value;

        if (!statistics_is_finite(summary.reciprocal_sum)) {
            return std::unexpected(make_error(error_t::range_error));
        }
    }

    if (summary.count == 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return summary;
}

[[nodiscard]] inline auto geometric_mean_from_summary(
    const statistics_geometric_summary& summary) -> result<double>
{
    if (summary.count == 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (summary.has_zero) {
        return 0.0;
    }

    return statistics_to_double(
        std::exp(summary.log_sum / static_cast<long double>(summary.count)));
}

[[nodiscard]] inline auto harmonic_mean_from_summary(
    const statistics_harmonic_summary& summary) -> result<double>
{
    if (summary.count == 0 || summary.reciprocal_sum == 0.0L) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return statistics_to_double(
        static_cast<long double>(summary.count) / summary.reciprocal_sum);
}

template<statistics_range Range>
[[nodiscard]] auto summarize_statistics(Range&& range)
    -> result<statistics_summary>
{
    statistics_summary summary{};

    for (auto&& element : range) {
        const long double value = static_cast<long double>(element);
        if (!statistics_is_finite(value)) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        ++summary.count;
        const long double n = static_cast<long double>(summary.count);
        const long double delta = value - summary.mean;
        summary.mean += delta / n;
        const long double delta2 = value - summary.mean;
        summary.m2 += delta * delta2;

        if (!statistics_is_finite(summary.mean) ||
            !statistics_is_finite(summary.m2)) {
            return std::unexpected(make_error(error_t::range_error));
        }
    }

    if (summary.count == 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return summary;
}

template<statistics_range Range>
[[nodiscard]] auto collect_statistics_values(Range&& range)
    -> result<std::vector<long double>>
{
    std::vector<long double> values;

    for (auto&& element : range) {
        const long double value = static_cast<long double>(element);
        if (!statistics_is_finite(value)) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }
        values.push_back(value);
    }

    if (values.empty()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return values;
}

[[nodiscard]] inline auto population_variance_from_summary(
    const statistics_summary& summary) -> result<double>
{
    if (summary.count == 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return statistics_to_double(
        summary.m2 / static_cast<long double>(summary.count));
}

[[nodiscard]] inline auto sample_variance_from_summary(
    const statistics_summary& summary) -> result<double>
{
    if (summary.count < 2) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return statistics_to_double(
        summary.m2 / static_cast<long double>(summary.count - 1));
}

[[nodiscard]] inline auto stddev_from_variance(double value) -> result<double>
{
    if (!std::isfinite(value) || value < 0.0) {
        return std::unexpected(make_error(error_t::range_error));
    }

    return std::sqrt(value);
}

[[nodiscard]] inline auto median_from_values(std::vector<long double> values)
    -> result<double>
{
    if (values.empty()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::ranges::sort(values);

    const std::size_t size = values.size();
    const std::size_t middle = size / 2;

    if ((size % 2) != 0) {
        return statistics_to_double(values[middle]);
    }

    const long double value =
        (values[middle - 1] / 2.0L) + (values[middle] / 2.0L);
    return statistics_to_double(value);
}

[[nodiscard]] inline auto statistics_quantile_fraction(double q)
    -> result<long double>
{
    if (!std::isfinite(q) || q < 0.0 || q > 1.0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return static_cast<long double>(q);
}

[[nodiscard]] inline auto statistics_percentile_fraction(double p)
    -> result<long double>
{
    if (!std::isfinite(p) || p < 0.0 || p > 100.0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return static_cast<long double>(p) / 100.0L;
}

[[nodiscard]] inline auto quantile_from_values(
    std::vector<long double> values,
    long double fraction) -> result<double>
{
    if (values.empty()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (!statistics_is_finite(fraction) || fraction < 0.0L ||
        fraction > 1.0L) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::ranges::sort(values);

    if (values.size() == 1) {
        return statistics_to_double(values.front());
    }

    const long double position =
        fraction * static_cast<long double>(values.size() - 1);
    const auto lower = static_cast<std::size_t>(std::floor(position));
    const auto upper = static_cast<std::size_t>(std::ceil(position));
    const long double weight = position - static_cast<long double>(lower);

    const long double value = values[lower] +
        ((values[upper] - values[lower]) * weight);
    return statistics_to_double(value);
}

[[nodiscard]] inline auto statistics_mode_tolerance(double tolerance)
    -> result<long double>
{
    if (!std::isfinite(tolerance) || tolerance < 0.0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return static_cast<long double>(tolerance);
}

[[nodiscard]] inline auto exact_mode_groups(const std::vector<long double>& values)
    -> std::vector<statistics_mode_group>
{
    std::vector<statistics_mode_group> groups;

    for (std::size_t index = 0; index < values.size();) {
        const long double current = values[index];
        statistics_mode_group group{};

        do {
            ++group.count;
            group.sum += values[index];
            ++index;
        } while (index < values.size() && values[index] == current);

        groups.push_back(group);
    }

    return groups;
}

[[nodiscard]] inline auto tolerant_mode_groups(
    const std::vector<long double>& values,
    long double tolerance) -> std::vector<statistics_mode_group>
{
    std::vector<statistics_mode_group> groups;

    for (std::size_t index = 0; index < values.size();) {
        const long double first = values[index];
        statistics_mode_group group{};

        do {
            ++group.count;
            group.sum += values[index];
            ++index;
        } while (index < values.size() && values[index] - first <= tolerance);

        groups.push_back(group);
    }

    return groups;
}

[[nodiscard]] inline auto mode_from_values(
    std::vector<long double> values,
    double tolerance) -> result<std::vector<double>>
{
    if (values.empty()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const auto checked_tolerance = statistics_mode_tolerance(tolerance);
    if (!checked_tolerance) {
        return std::unexpected(checked_tolerance.error());
    }

    std::ranges::sort(values);

    const auto groups = (*checked_tolerance == 0.0L)
        ? exact_mode_groups(values)
        : tolerant_mode_groups(values, *checked_tolerance);

    std::size_t max_count = 0;
    for (const auto& group : groups) {
        max_count = std::max(max_count, group.count);
    }

    std::vector<double> modes;
    if (max_count < 2) {
        return modes;
    }

    for (const auto& group : groups) {
        if (group.count != max_count) {
            continue;
        }

        const long double representative =
            group.sum / static_cast<long double>(group.count);
        const auto value = statistics_to_double(representative);
        if (!value) {
            return std::unexpected(value.error());
        }
        modes.push_back(*value);
    }

    return modes;
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Computes the arithmetic mean of a non-empty arithmetic range.
 *
 * @tparam Range Input range of non-bool arithmetic values.
 * @param range Input range.
 * @return Arithmetic mean as double, or an error for an empty range or invalid
 *         numeric input.
 */
template<detail::statistics_range Range>
[[nodiscard]] auto mean(Range&& range) -> result<double>
{
    const auto summary = detail::summarize_statistics(
        std::forward<Range>(range));
    if (!summary) {
        return std::unexpected(summary.error());
    }

    return detail::statistics_to_double(summary->mean);
}

/**
 * @brief Computes the arithmetic mean of a non-empty initializer list.
 *
 * @tparam T Non-bool arithmetic value type.
 * @param values Input values.
 * @return Arithmetic mean as double.
 */
template<class T>
    requires non_bool_arithmetic<T>
[[nodiscard]] auto mean(std::initializer_list<T> values) -> result<double>
{
    const auto summary = detail::summarize_statistics(values);
    if (!summary) {
        return std::unexpected(summary.error());
    }

    return detail::statistics_to_double(summary->mean);
}


/**
 * @brief Computes the sum of a non-empty arithmetic range.
 *
 * @tparam Range Input range of non-bool arithmetic values.
 * @param range Input range.
 * @return Sum as double.
 */
template<detail::statistics_range Range>
[[nodiscard]] auto sum(Range&& range) -> result<double>
{
    const auto value = detail::sum_statistics_values(std::forward<Range>(range));
    if (!value) {
        return std::unexpected(value.error());
    }

    return detail::statistics_to_double(*value);
}

/**
 * @brief Computes the sum of a non-empty initializer list.
 *
 * @tparam T Non-bool arithmetic value type.
 * @param values Input values.
 * @return Sum as double.
 */
template<class T>
    requires non_bool_arithmetic<T>
[[nodiscard]] auto sum(std::initializer_list<T> values) -> result<double>
{
    const auto value = detail::sum_statistics_values(values);
    if (!value) {
        return std::unexpected(value.error());
    }

    return detail::statistics_to_double(*value);
}

/**
 * @brief Computes the product of a non-empty arithmetic range.
 *
 * @tparam Range Input range of non-bool arithmetic values.
 * @param range Input range.
 * @return Product as double.
 */
template<detail::statistics_range Range>
[[nodiscard]] auto product(Range&& range) -> result<double>
{
    const auto summary = detail::product_statistics_values(
        std::forward<Range>(range));
    if (!summary) {
        return std::unexpected(summary.error());
    }

    return detail::statistics_to_double(summary->product);
}

/**
 * @brief Computes the product of a non-empty initializer list.
 *
 * @tparam T Non-bool arithmetic value type.
 * @param values Input values.
 * @return Product as double.
 */
template<class T>
    requires non_bool_arithmetic<T>
[[nodiscard]] auto product(std::initializer_list<T> values) -> result<double>
{
    const auto summary = detail::product_statistics_values(values);
    if (!summary) {
        return std::unexpected(summary.error());
    }

    return detail::statistics_to_double(summary->product);
}

/**
 * @brief Computes the geometric mean of a non-empty arithmetic range.
 *
 * All input values must be finite and non-negative.  If any value is zero,
 * the result is zero.
 *
 * @tparam Range Input range of non-bool arithmetic values.
 * @param range Input range.
 * @return Geometric mean as double.
 */
template<detail::statistics_range Range>
[[nodiscard]] auto geometric_mean(Range&& range) -> result<double>
{
    const auto summary = detail::summarize_geometric_statistics(
        std::forward<Range>(range));
    if (!summary) {
        return std::unexpected(summary.error());
    }

    return detail::geometric_mean_from_summary(*summary);
}

/**
 * @brief Computes the geometric mean of a non-empty initializer list.
 *
 * @tparam T Non-bool arithmetic value type.
 * @param values Input values.
 * @return Geometric mean as double.
 */
template<class T>
    requires non_bool_arithmetic<T>
[[nodiscard]] auto geometric_mean(std::initializer_list<T> values)
    -> result<double>
{
    const auto summary = detail::summarize_geometric_statistics(values);
    if (!summary) {
        return std::unexpected(summary.error());
    }

    return detail::geometric_mean_from_summary(*summary);
}

/**
 * @brief Computes the harmonic mean of a non-empty arithmetic range.
 *
 * All input values must be finite and positive.
 *
 * @tparam Range Input range of non-bool arithmetic values.
 * @param range Input range.
 * @return Harmonic mean as double.
 */
template<detail::statistics_range Range>
[[nodiscard]] auto harmonic_mean(Range&& range) -> result<double>
{
    const auto summary = detail::summarize_harmonic_statistics(
        std::forward<Range>(range));
    if (!summary) {
        return std::unexpected(summary.error());
    }

    return detail::harmonic_mean_from_summary(*summary);
}

/**
 * @brief Computes the harmonic mean of a non-empty initializer list.
 *
 * @tparam T Non-bool arithmetic value type.
 * @param values Input values.
 * @return Harmonic mean as double.
 */
template<class T>
    requires non_bool_arithmetic<T>
[[nodiscard]] auto harmonic_mean(std::initializer_list<T> values)
    -> result<double>
{
    const auto summary = detail::summarize_harmonic_statistics(values);
    if (!summary) {
        return std::unexpected(summary.error());
    }

    return detail::harmonic_mean_from_summary(*summary);
}

/**
 * @brief Computes the median of a non-empty arithmetic range.
 *
 * For an even number of values, this function returns the arithmetic mean of
 * the two middle values after sorting.
 *
 * @tparam Range Input range of non-bool arithmetic values.
 * @param range Input range.
 * @return Median as double.
 */
template<detail::statistics_range Range>
[[nodiscard]] auto median(Range&& range) -> result<double>
{
    auto values = detail::collect_statistics_values(std::forward<Range>(range));
    if (!values) {
        return std::unexpected(values.error());
    }

    return detail::median_from_values(std::move(*values));
}

/**
 * @brief Computes the median of a non-empty initializer list.
 *
 * @tparam T Non-bool arithmetic value type.
 * @param values Input values.
 * @return Median as double.
 */
template<class T>
    requires non_bool_arithmetic<T>
[[nodiscard]] auto median(std::initializer_list<T> values) -> result<double>
{
    auto collected = detail::collect_statistics_values(values);
    if (!collected) {
        return std::unexpected(collected.error());
    }

    return detail::median_from_values(std::move(*collected));
}

/**
 * @brief Computes a quantile of a non-empty arithmetic range.
 *
 * The quantile fraction `q` must be finite and in the range `[0.0, 1.0]`.
 * Values are copied, sorted, and linearly interpolated.
 * `q == 0.0` returns the minimum value, `q == 0.5` returns the median,
 * and `q == 1.0` returns the maximum value.
 *
 * @tparam Range Input range of non-bool arithmetic values.
 * @param range Input range.
 * @param q Quantile fraction in the range `[0.0, 1.0]`.
 * @return Quantile value as double.
 */
template<detail::statistics_range Range>
[[nodiscard]] auto quantile(Range&& range, double q) -> result<double>
{
    auto fraction = detail::statistics_quantile_fraction(q);
    if (!fraction) {
        return std::unexpected(fraction.error());
    }

    auto values = detail::collect_statistics_values(std::forward<Range>(range));
    if (!values) {
        return std::unexpected(values.error());
    }

    return detail::quantile_from_values(std::move(*values), *fraction);
}

/**
 * @brief Computes a quantile of a non-empty initializer list.
 *
 * @tparam T Non-bool arithmetic value type.
 * @param values Input values.
 * @param q Quantile fraction in the range `[0.0, 1.0]`.
 * @return Quantile value as double.
 */
template<class T>
    requires non_bool_arithmetic<T>
[[nodiscard]] auto quantile(std::initializer_list<T> values, double q)
    -> result<double>
{
    auto fraction = detail::statistics_quantile_fraction(q);
    if (!fraction) {
        return std::unexpected(fraction.error());
    }

    auto collected = detail::collect_statistics_values(values);
    if (!collected) {
        return std::unexpected(collected.error());
    }

    return detail::quantile_from_values(std::move(*collected), *fraction);
}

/**
 * @brief Computes a percentile of a non-empty arithmetic range.
 *
 * The percentile value `p` must be finite and in the range `[0.0, 100.0]`.
 * This function uses the same linear interpolation rule as `quantile`;
 * `percentile(range, p)` is equivalent to `quantile(range, p / 100.0)`.
 *
 * @tparam Range Input range of non-bool arithmetic values.
 * @param range Input range.
 * @param p Percentile in the range `[0.0, 100.0]`.
 * @return Percentile value as double.
 */
template<detail::statistics_range Range>
[[nodiscard]] auto percentile(Range&& range, double p) -> result<double>
{
    auto fraction = detail::statistics_percentile_fraction(p);
    if (!fraction) {
        return std::unexpected(fraction.error());
    }

    auto values = detail::collect_statistics_values(std::forward<Range>(range));
    if (!values) {
        return std::unexpected(values.error());
    }

    return detail::quantile_from_values(std::move(*values), *fraction);
}

/**
 * @brief Computes a percentile of a non-empty initializer list.
 *
 * @tparam T Non-bool arithmetic value type.
 * @param values Input values.
 * @param p Percentile in the range `[0.0, 100.0]`.
 * @return Percentile value as double.
 */
template<class T>
    requires non_bool_arithmetic<T>
[[nodiscard]] auto percentile(std::initializer_list<T> values, double p)
    -> result<double>
{
    auto fraction = detail::statistics_percentile_fraction(p);
    if (!fraction) {
        return std::unexpected(fraction.error());
    }

    auto collected = detail::collect_statistics_values(values);
    if (!collected) {
        return std::unexpected(collected.error());
    }

    return detail::quantile_from_values(std::move(*collected), *fraction);
}

/**
 * @brief Computes the mode values of a non-empty arithmetic range.
 *
 * If no value appears at least twice, this function returns an empty vector.
 * If multiple values share the highest frequency, all of them are returned in
 * ascending order.  When `tolerance` is positive, sorted values whose distance
 * from the first value of the current group is at most `tolerance` are treated
 * as the same group, and the returned representative is the arithmetic mean of
 * that group.
 *
 * @tparam Range Input range of non-bool arithmetic values.
 * @param range Input range.
 * @param tolerance Non-negative grouping tolerance.  The default `0.0` uses
 *        exact equality.
 * @return Mode values as doubles.
 */
template<detail::statistics_range Range>
[[nodiscard]] auto mode(Range&& range, double tolerance = 0.0)
    -> result<std::vector<double>>
{
    auto values = detail::collect_statistics_values(std::forward<Range>(range));
    if (!values) {
        return std::unexpected(values.error());
    }

    return detail::mode_from_values(std::move(*values), tolerance);
}

/**
 * @brief Computes the mode values of a non-empty initializer list.
 *
 * @tparam T Non-bool arithmetic value type.
 * @param values Input values.
 * @param tolerance Non-negative grouping tolerance.  The default `0.0` uses
 *        exact equality.
 * @return Mode values as doubles.
 */
template<class T>
    requires non_bool_arithmetic<T>
[[nodiscard]] auto mode(std::initializer_list<T> values, double tolerance = 0.0)
    -> result<std::vector<double>>
{
    auto collected = detail::collect_statistics_values(values);
    if (!collected) {
        return std::unexpected(collected.error());
    }

    return detail::mode_from_values(std::move(*collected), tolerance);
}

/**
 * @brief Computes the population variance of a non-empty arithmetic range.
 *
 * The population variance divides the sum of squared deviations by `n`.
 *
 * @tparam Range Input range of non-bool arithmetic values.
 * @param range Input range.
 * @return Population variance as double.
 */
template<detail::statistics_range Range>
[[nodiscard]] auto variance(Range&& range) -> result<double>
{
    const auto summary = detail::summarize_statistics(
        std::forward<Range>(range));
    if (!summary) {
        return std::unexpected(summary.error());
    }

    return detail::population_variance_from_summary(*summary);
}

/**
 * @brief Computes the population variance of a non-empty initializer list.
 *
 * @tparam T Non-bool arithmetic value type.
 * @param values Input values.
 * @return Population variance as double.
 */
template<class T>
    requires non_bool_arithmetic<T>
[[nodiscard]] auto variance(std::initializer_list<T> values) -> result<double>
{
    const auto summary = detail::summarize_statistics(values);
    if (!summary) {
        return std::unexpected(summary.error());
    }

    return detail::population_variance_from_summary(*summary);
}

/**
 * @brief Computes the sample variance of an arithmetic range.
 *
 * The sample variance divides the sum of squared deviations by `n - 1`.
 * At least two values are required.
 *
 * @tparam Range Input range of non-bool arithmetic values.
 * @param range Input range.
 * @return Sample variance as double.
 */
template<detail::statistics_range Range>
[[nodiscard]] auto sample_variance(Range&& range) -> result<double>
{
    const auto summary = detail::summarize_statistics(
        std::forward<Range>(range));
    if (!summary) {
        return std::unexpected(summary.error());
    }

    return detail::sample_variance_from_summary(*summary);
}

/**
 * @brief Computes the sample variance of an initializer list.
 *
 * @tparam T Non-bool arithmetic value type.
 * @param values Input values.
 * @return Sample variance as double.
 */
template<class T>
    requires non_bool_arithmetic<T>
[[nodiscard]] auto sample_variance(std::initializer_list<T> values)
    -> result<double>
{
    const auto summary = detail::summarize_statistics(values);
    if (!summary) {
        return std::unexpected(summary.error());
    }

    return detail::sample_variance_from_summary(*summary);
}

/**
 * @brief Computes the population standard deviation of a non-empty range.
 *
 * This is the square root of `variance(range)`.
 *
 * @tparam Range Input range of non-bool arithmetic values.
 * @param range Input range.
 * @return Population standard deviation as double.
 */
template<detail::statistics_range Range>
[[nodiscard]] auto stddev(Range&& range) -> result<double>
{
    const auto value = variance(std::forward<Range>(range));
    if (!value) {
        return std::unexpected(value.error());
    }

    return detail::stddev_from_variance(*value);
}

/**
 * @brief Computes the population standard deviation of an initializer list.
 *
 * @tparam T Non-bool arithmetic value type.
 * @param values Input values.
 * @return Population standard deviation as double.
 */
template<class T>
    requires non_bool_arithmetic<T>
[[nodiscard]] auto stddev(std::initializer_list<T> values) -> result<double>
{
    const auto value = variance(values);
    if (!value) {
        return std::unexpected(value.error());
    }

    return detail::stddev_from_variance(*value);
}

/**
 * @brief Computes the sample standard deviation of an arithmetic range.
 *
 * This is the square root of `sample_variance(range)`.
 * At least two values are required.
 *
 * @tparam Range Input range of non-bool arithmetic values.
 * @param range Input range.
 * @return Sample standard deviation as double.
 */
template<detail::statistics_range Range>
[[nodiscard]] auto sample_stddev(Range&& range) -> result<double>
{
    const auto value = sample_variance(std::forward<Range>(range));
    if (!value) {
        return std::unexpected(value.error());
    }

    return detail::stddev_from_variance(*value);
}

/**
 * @brief Computes the sample standard deviation of an initializer list.
 *
 * @tparam T Non-bool arithmetic value type.
 * @param values Input values.
 * @return Sample standard deviation as double.
 */
template<class T>
    requires non_bool_arithmetic<T>
[[nodiscard]] auto sample_stddev(std::initializer_list<T> values)
    -> result<double>
{
    const auto value = sample_variance(values);
    if (!value) {
        return std::unexpected(value.error());
    }

    return detail::stddev_from_variance(*value);
}

} // namespace xer

#endif /* XER_BITS_STATISTICS_H_INCLUDED_ */

/**
 * @file xer/bits/statistics.h
 * @brief Mean, variance, and standard deviation helpers.
 */

#pragma once

#ifndef XER_BITS_STATISTICS_H_INCLUDED_
#define XER_BITS_STATISTICS_H_INCLUDED_

#include <cmath>
#include <cstddef>
#include <expected>
#include <initializer_list>
#include <limits>
#include <ranges>
#include <type_traits>
#include <utility>

#include <xer/bits/arithmetic_concepts.h>
#include <xer/error.h>

namespace xer::detail {

struct statistics_summary {
    std::size_t count{};
    long double mean{};
    long double m2{};
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

/**
 * @file xer/bits/is_close.h
 * @brief Floating-point approximate comparison helpers.
 */

#pragma once

#ifndef XER_BITS_IS_CLOSE_H_INCLUDED_
#define XER_BITS_IS_CLOSE_H_INCLUDED_

#include <expected>
#include <limits>
#include <type_traits>

#include <xer/bits/arithmetic_concepts.h>
#include <xer/error.h>

namespace xer::detail {

template<typename T>
    requires non_bool_arithmetic<T>
[[nodiscard]] constexpr auto to_is_close_long_double(T value) noexcept
    -> long double
{
    return static_cast<long double>(value);
}

[[nodiscard]] constexpr auto is_close_finite(long double value) noexcept -> bool
{
    return value == value &&
           value <= std::numeric_limits<long double>::max() &&
           value >= std::numeric_limits<long double>::lowest();
}

[[nodiscard]] constexpr auto is_close_abs(long double value) noexcept
    -> long double
{
    return value < 0.0L ? -value : value;
}

template<typename T>
[[nodiscard]] constexpr auto is_close_rounding_margin(
    long double lhs,
    long double rhs,
    long double tolerance) noexcept -> long double
{
    if constexpr (std::is_floating_point_v<T>) {
        const long double a = is_close_abs(lhs);
        const long double b = is_close_abs(rhs);
        const long double e = is_close_abs(tolerance);
        long double scale = 1.0L;

        if (scale < a) {
            scale = a;
        }
        if (scale < b) {
            scale = b;
        }
        if (scale < e) {
            scale = e;
        }

        return scale *
               static_cast<long double>(std::numeric_limits<T>::epsilon()) *
               16.0L;
    } else {
        return 0.0L;
    }
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Tests whether two arithmetic values are within an absolute tolerance.
 *
 * The comparison is inclusive: `abs(lhs - rhs) <= epsilon`, with a small
 * implementation rounding margin for positive tolerance values.
 *
 * If either operand is NaN or infinity, or if `epsilon` is NaN, infinity, or
 * negative, this function returns false.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @tparam E Tolerance type.
 * @param lhs Left-hand side value.
 * @param rhs Right-hand side value.
 * @param epsilon Absolute tolerance.
 * @return true if the values are within the specified tolerance.
 */
template<typename A, typename B, typename E>
    requires non_bool_arithmetic<A> && non_bool_arithmetic<B> &&
             non_bool_arithmetic<E>
[[nodiscard]] constexpr auto is_close(A lhs, B rhs, E epsilon) noexcept -> bool
{
    const long double left = detail::to_is_close_long_double(lhs);
    const long double right = detail::to_is_close_long_double(rhs);
    const long double tolerance = detail::to_is_close_long_double(epsilon);

    if (!detail::is_close_finite(left) ||
        !detail::is_close_finite(right) ||
        !detail::is_close_finite(tolerance) || tolerance < 0.0L) {
        return false;
    }

    const long double difference = left - right;
    if (!detail::is_close_finite(difference)) {
        return false;
    }

    const long double absolute_difference = detail::is_close_abs(difference);
    if (absolute_difference <= tolerance) {
        return true;
    }

    if (tolerance == 0.0L) {
        return false;
    }

    using common_type = std::common_type_t<A, B, E>;
    const long double margin = detail::is_close_rounding_margin<common_type>(
        left,
        right,
        tolerance);

    return absolute_difference <= tolerance + margin;
}

template<typename A, typename B, typename E>
    requires non_bool_arithmetic<A> && non_bool_arithmetic<B> &&
             non_bool_arithmetic<E>
[[nodiscard]] constexpr auto is_close(
    const result<A>& lhs,
    B rhs,
    E epsilon) noexcept -> result<bool>
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    return is_close(*lhs, rhs, epsilon);
}

template<typename A, typename B, typename E>
    requires non_bool_arithmetic<A> && non_bool_arithmetic<B> &&
             non_bool_arithmetic<E>
[[nodiscard]] constexpr auto is_close(
    A lhs,
    const result<B>& rhs,
    E epsilon) noexcept -> result<bool>
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return is_close(lhs, *rhs, epsilon);
}

template<typename A, typename B, typename E>
    requires non_bool_arithmetic<A> && non_bool_arithmetic<B> &&
             non_bool_arithmetic<E>
[[nodiscard]] constexpr auto is_close(
    const result<A>& lhs,
    const result<B>& rhs,
    E epsilon) noexcept -> result<bool>
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return is_close(*lhs, *rhs, epsilon);
}

} // namespace xer

#endif /* XER_BITS_IS_CLOSE_H_INCLUDED_ */

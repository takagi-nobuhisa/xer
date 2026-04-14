/**
 * @file xer/bits/clamp.h
 * @brief Internal clamp function implementations.
 */

#pragma once

#ifndef XER_BITS_CLAMP_H_INCLUDED_
#define XER_BITS_CLAMP_H_INCLUDED_

#include <cmath>
#include <expected>
#include <type_traits>

#include <xer/bits/arithmetic_concepts.h>
#include <xer/bits/in_range.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Converts an arithmetic value to long double for mixed-type comparison.
 *
 * @tparam T Arithmetic type.
 * @param value Source value.
 * @return Converted value.
 */
template<typename T>
    requires arithmetic<T>
[[nodiscard]] constexpr auto to_clamp_long_double(T value) noexcept
    -> long double
{
    return static_cast<long double>(value);
}

/**
 * @brief Returns whether the given arithmetic value is NaN.
 *
 * Non-floating-point values always return false.
 *
 * @tparam T Arithmetic type.
 * @param value Source value.
 * @return true if the value is NaN.
 */
template<typename T>
    requires arithmetic<T>
[[nodiscard]] constexpr auto is_nan_value(T value) noexcept -> bool
{
    if constexpr (std::floating_point<std::remove_cvref_t<T>>) {
        return std::isnan(value);
    } else {
        return false;
    }
}

/**
 * @brief Returns whether lhs is less than rhs using mixed-type comparison.
 *
 * @tparam L Left-hand side arithmetic type.
 * @tparam R Right-hand side arithmetic type.
 * @param lhs Left-hand side value.
 * @param rhs Right-hand side value.
 * @return true if lhs is less than rhs.
 */
template<typename L, typename R>
    requires arithmetic<L> && arithmetic<R>
[[nodiscard]] constexpr auto clamp_less(L lhs, R rhs) noexcept -> bool
{
    return to_clamp_long_double(lhs) < to_clamp_long_double(rhs);
}

/**
 * @brief Validates clamp arguments.
 *
 * This rejects NaN values and low > high.
 *
 * @tparam T Value type.
 * @tparam L Low bound type.
 * @tparam H High bound type.
 * @param value Input value.
 * @param low Low bound.
 * @param high High bound.
 * @return Success or invalid_argument.
 */
template<typename T, typename L, typename H>
    requires arithmetic<T> && arithmetic<L> && arithmetic<H>
[[nodiscard]] constexpr auto validate_clamp_arguments(
    T value,
    L low,
    H high) -> result<void>
{
    if (is_nan_value(value) || is_nan_value(low) || is_nan_value(high)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (clamp_less(high, low)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return {};
}

/**
 * @brief Converts a bound value to the result type after range checking.
 *
 * @tparam T Result type.
 * @tparam U Source bound type.
 * @param value Source bound value.
 * @return Converted value or invalid_argument.
 */
template<typename T, typename U>
    requires non_bool_arithmetic<T> && arithmetic<U>
[[nodiscard]] constexpr auto clamp_cast(U value) -> result<T>
{
    if (!in_range<T>(value)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return static_cast<T>(value);
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Clamps a value to the inclusive range [low, high].
 *
 * The return type is the type of the first argument.
 * If low > high, this function returns invalid_argument.
 * If a bound must be returned and it is outside the range of T,
 * this function returns invalid_argument.
 *
 * @tparam T Value type and result type.
 * @tparam L Low bound type.
 * @tparam H High bound type.
 * @param value Input value.
 * @param low Low bound.
 * @param high High bound.
 * @return Clamped value.
 */
template<typename T, typename L, typename H>
    requires non_bool_arithmetic<T> && arithmetic<L> && arithmetic<H>
[[nodiscard]] constexpr auto clamp(T value, L low, H high) -> result<T>
{
    const auto validation = detail::validate_clamp_arguments(value, low, high);
    if (!validation.has_value()) {
        return std::unexpected(validation.error());
    }

    if (detail::clamp_less(value, low)) {
        return detail::clamp_cast<T>(low);
    }

    if (detail::clamp_less(high, value)) {
        return detail::clamp_cast<T>(high);
    }

    return value;
}

/**
 * @brief Clamps a successful result value to the inclusive range [low, high].
 *
 * Errors in the first argument are propagated unchanged.
 *
 * @tparam T Value type and result type.
 * @tparam L Low bound type.
 * @tparam H High bound type.
 * @param value Input result value.
 * @param low Low bound.
 * @param high High bound.
 * @return Clamped value.
 */
template<typename T, typename L, typename H>
    requires non_bool_arithmetic<T> && arithmetic<L> && arithmetic<H>
[[nodiscard]] constexpr auto clamp(
    const result<T>& value,
    L low,
    H high) -> result<T>
{
    if (!value.has_value()) {
        return std::unexpected(value.error());
    }

    return clamp(*value, low, high);
}

} // namespace xer

#endif /* XER_BITS_CLAMP_H_INCLUDED_ */

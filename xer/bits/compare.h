/**
 * @file xer/bits/compare.h
 * @brief Internal comparison function implementations.
 */

#pragma once

#ifndef XER_BITS_COMPARE_H_INCLUDED_
#define XER_BITS_COMPARE_H_INCLUDED_

#include <cmath>
#include <concepts>
#include <expected>
#include <type_traits>

#include <xer/bits/arithmetic_concepts.h>
#include <xer/bits/common.h>
#include <xer/error.h>
#include <xer/stdint.h>

namespace xer::detail {

/**
 * @brief Concept for integral types except `bool`.
 *
 * @tparam T Type to test.
 */
template<typename T>
concept compare_integral =
    std::integral<std::remove_cvref_t<T>> &&
    !std::same_as<std::remove_cvref_t<T>, bool>;

/**
 * @brief Mathematical integer represented by sign and magnitude.
 */
struct compare_integer_value {
    bool negative;
    xer::uint128_t magnitude;
};

/**
 * @brief Converts an integer operand to sign-magnitude form.
 *
 * @tparam T Integral type except `bool`.
 * @param value Integer operand.
 * @return Sign-magnitude representation.
 */
template<typename T>
    requires compare_integral<T>
[[nodiscard]] constexpr auto to_compare_integer_value(T value) noexcept
    -> compare_integer_value
{
    using raw_t = std::remove_cvref_t<T>;

    if constexpr (std::unsigned_integral<raw_t>) {
        return compare_integer_value{
            false,
            static_cast<xer::uint128_t>(value),
        };
    } else {
        if (value >= 0) {
            return compare_integer_value{
                false,
                static_cast<xer::uint128_t>(value),
            };
        }

        return compare_integer_value{
            true,
            static_cast<xer::uint128_t>(-(value + 1)) + 1,
        };
    }
}

/**
 * @brief Compares two sign-magnitude integers.
 *
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Negative if `lhs < rhs`, zero if equal, positive if `lhs > rhs`.
 */
[[nodiscard]] constexpr auto compare_integer_value_impl(
    compare_integer_value lhs,
    compare_integer_value rhs) noexcept -> int
{
    if (lhs.negative != rhs.negative) {
        return lhs.negative ? -1 : 1;
    }

    if (lhs.magnitude == rhs.magnitude) {
        return 0;
    }

    if (!lhs.negative) {
        return lhs.magnitude < rhs.magnitude ? -1 : 1;
    }

    return lhs.magnitude < rhs.magnitude ? 1 : -1;
}

/**
 * @brief Compares two integral operands mathematically.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Negative if `lhs < rhs`, zero if equal, positive if `lhs > rhs`.
 */
template<typename A, typename B>
    requires compare_integral<A> && compare_integral<B>
[[nodiscard]] constexpr auto compare_integral_impl(A lhs, B rhs) noexcept
    -> int
{
    return compare_integer_value_impl(
        to_compare_integer_value(lhs),
        to_compare_integer_value(rhs));
}

/**
 * @brief Checks whether a floating-point operand is NaN.
 *
 * For non-floating operands, this always returns false.
 *
 * @tparam T Operand type.
 * @param value Operand.
 * @return true if the operand is NaN.
 */
template<typename T>
[[nodiscard]] constexpr auto is_nan_operand(T value) noexcept -> bool
{
    if constexpr (std::floating_point<std::remove_cvref_t<T>>) {
        return std::isnan(value);
    } else {
        (void)value;
        return false;
    }
}

/**
 * @brief Converts an arithmetic operand to `long double`.
 *
 * @tparam T Operand type.
 * @param value Operand.
 * @return Converted value.
 */
template<typename T>
    requires non_bool_arithmetic<T>
[[nodiscard]] constexpr auto to_compare_long_double(T value) noexcept
    -> long double
{
    return static_cast<long double>(value);
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Tests whether two integral operands are equal.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Comparison result.
 */
template<typename A, typename B>
    requires detail::compare_integral<A> && detail::compare_integral<B>
[[nodiscard]] constexpr auto eq(A lhs, B rhs) noexcept -> bool
{
    return detail::compare_integral_impl(lhs, rhs) == 0;
}

/**
 * @brief Tests whether two operands are equal.
 *
 * If either operand is NaN, this function returns false.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Comparison result.
 */
template<typename A, typename B>
    requires non_bool_arithmetic<A> && non_bool_arithmetic<B> &&
             (std::floating_point<std::remove_cvref_t<A>> ||
              std::floating_point<std::remove_cvref_t<B>>)
[[nodiscard]] constexpr auto eq(A lhs, B rhs) noexcept -> bool
{
    if (detail::is_nan_operand(lhs) || detail::is_nan_operand(rhs)) {
        return false;
    }

    return detail::to_compare_long_double(lhs) ==
           detail::to_compare_long_double(rhs);
}

/**
 * @brief Tests whether two integral operands are not equal.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Comparison result.
 */
template<typename A, typename B>
    requires detail::compare_integral<A> && detail::compare_integral<B>
[[nodiscard]] constexpr auto ne(A lhs, B rhs) noexcept -> bool
{
    return detail::compare_integral_impl(lhs, rhs) != 0;
}

/**
 * @brief Tests whether two operands are not equal.
 *
 * If either operand is NaN, this function returns true.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Comparison result.
 */
template<typename A, typename B>
    requires non_bool_arithmetic<A> && non_bool_arithmetic<B> &&
             (std::floating_point<std::remove_cvref_t<A>> ||
              std::floating_point<std::remove_cvref_t<B>>)
[[nodiscard]] constexpr auto ne(A lhs, B rhs) noexcept -> bool
{
    if (detail::is_nan_operand(lhs) || detail::is_nan_operand(rhs)) {
        return true;
    }

    return detail::to_compare_long_double(lhs) !=
           detail::to_compare_long_double(rhs);
}

/**
 * @brief Tests whether `lhs < rhs` for integral operands.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Comparison result.
 */
template<typename A, typename B>
    requires detail::compare_integral<A> && detail::compare_integral<B>
[[nodiscard]] constexpr auto lt(A lhs, B rhs) noexcept -> bool
{
    return detail::compare_integral_impl(lhs, rhs) < 0;
}

/**
 * @brief Tests whether `lhs < rhs`.
 *
 * If either operand is NaN, this function returns false.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Comparison result.
 */
template<typename A, typename B>
    requires non_bool_arithmetic<A> && non_bool_arithmetic<B> &&
             (std::floating_point<std::remove_cvref_t<A>> ||
              std::floating_point<std::remove_cvref_t<B>>)
[[nodiscard]] constexpr auto lt(A lhs, B rhs) noexcept -> bool
{
    if (detail::is_nan_operand(lhs) || detail::is_nan_operand(rhs)) {
        return false;
    }

    return detail::to_compare_long_double(lhs) <
           detail::to_compare_long_double(rhs);
}

/**
 * @brief Tests whether `lhs <= rhs` for integral operands.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Comparison result.
 */
template<typename A, typename B>
    requires detail::compare_integral<A> && detail::compare_integral<B>
[[nodiscard]] constexpr auto le(A lhs, B rhs) noexcept -> bool
{
    return detail::compare_integral_impl(lhs, rhs) <= 0;
}

/**
 * @brief Tests whether `lhs <= rhs`.
 *
 * If either operand is NaN, this function returns false.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Comparison result.
 */
template<typename A, typename B>
    requires non_bool_arithmetic<A> && non_bool_arithmetic<B> &&
             (std::floating_point<std::remove_cvref_t<A>> ||
              std::floating_point<std::remove_cvref_t<B>>)
[[nodiscard]] constexpr auto le(A lhs, B rhs) noexcept -> bool
{
    if (detail::is_nan_operand(lhs) || detail::is_nan_operand(rhs)) {
        return false;
    }

    return detail::to_compare_long_double(lhs) <=
           detail::to_compare_long_double(rhs);
}

/**
 * @brief Tests whether `lhs > rhs` for integral operands.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Comparison result.
 */
template<typename A, typename B>
    requires detail::compare_integral<A> && detail::compare_integral<B>
[[nodiscard]] constexpr auto gt(A lhs, B rhs) noexcept -> bool
{
    return detail::compare_integral_impl(lhs, rhs) > 0;
}

/**
 * @brief Tests whether `lhs > rhs`.
 *
 * If either operand is NaN, this function returns false.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Comparison result.
 */
template<typename A, typename B>
    requires non_bool_arithmetic<A> && non_bool_arithmetic<B> &&
             (std::floating_point<std::remove_cvref_t<A>> ||
              std::floating_point<std::remove_cvref_t<B>>)
[[nodiscard]] constexpr auto gt(A lhs, B rhs) noexcept -> bool
{
    if (detail::is_nan_operand(lhs) || detail::is_nan_operand(rhs)) {
        return false;
    }

    return detail::to_compare_long_double(lhs) >
           detail::to_compare_long_double(rhs);
}

/**
 * @brief Tests whether `lhs >= rhs` for integral operands.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Comparison result.
 */
template<typename A, typename B>
    requires detail::compare_integral<A> && detail::compare_integral<B>
[[nodiscard]] constexpr auto ge(A lhs, B rhs) noexcept -> bool
{
    return detail::compare_integral_impl(lhs, rhs) >= 0;
}

/**
 * @brief Tests whether `lhs >= rhs`.
 *
 * If either operand is NaN, this function returns false.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Comparison result.
 */
template<typename A, typename B>
    requires non_bool_arithmetic<A> && non_bool_arithmetic<B> &&
             (std::floating_point<std::remove_cvref_t<A>> ||
              std::floating_point<std::remove_cvref_t<B>>)
[[nodiscard]] constexpr auto ge(A lhs, B rhs) noexcept -> bool
{
    if (detail::is_nan_operand(lhs) || detail::is_nan_operand(rhs)) {
        return false;
    }

    return detail::to_compare_long_double(lhs) >=
           detail::to_compare_long_double(rhs);
}

template<typename T, typename U>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<U>
[[nodiscard]] constexpr auto eq(
    const result<T>& lhs,
    U rhs) noexcept -> result<bool>
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    return eq(*lhs, rhs);
}

template<typename T, typename U>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<U>
[[nodiscard]] constexpr auto eq(
    T lhs,
    const result<U>& rhs) noexcept -> result<bool>
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return eq(lhs, *rhs);
}

template<typename T, typename U>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<U>
[[nodiscard]] constexpr auto eq(
    const result<T>& lhs,
    const result<U>& rhs) noexcept -> result<bool>
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return eq(*lhs, *rhs);
}

template<typename T, typename U>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<U>
[[nodiscard]] constexpr auto ne(
    const result<T>& lhs,
    U rhs) noexcept -> result<bool>
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    return ne(*lhs, rhs);
}

template<typename T, typename U>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<U>
[[nodiscard]] constexpr auto ne(
    T lhs,
    const result<U>& rhs) noexcept -> result<bool>
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return ne(lhs, *rhs);
}

template<typename T, typename U>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<U>
[[nodiscard]] constexpr auto ne(
    const result<T>& lhs,
    const result<U>& rhs) noexcept -> result<bool>
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return ne(*lhs, *rhs);
}

template<typename T, typename U>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<U>
[[nodiscard]] constexpr auto lt(
    const result<T>& lhs,
    U rhs) noexcept -> result<bool>
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    return lt(*lhs, rhs);
}

template<typename T, typename U>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<U>
[[nodiscard]] constexpr auto lt(
    T lhs,
    const result<U>& rhs) noexcept -> result<bool>
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return lt(lhs, *rhs);
}

template<typename T, typename U>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<U>
[[nodiscard]] constexpr auto lt(
    const result<T>& lhs,
    const result<U>& rhs) noexcept -> result<bool>
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return lt(*lhs, *rhs);
}

template<typename T, typename U>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<U>
[[nodiscard]] constexpr auto le(
    const result<T>& lhs,
    U rhs) noexcept -> result<bool>
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    return le(*lhs, rhs);
}

template<typename T, typename U>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<U>
[[nodiscard]] constexpr auto le(
    T lhs,
    const result<U>& rhs) noexcept -> result<bool>
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return le(lhs, *rhs);
}

template<typename T, typename U>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<U>
[[nodiscard]] constexpr auto le(
    const result<T>& lhs,
    const result<U>& rhs) noexcept -> result<bool>
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return le(*lhs, *rhs);
}

template<typename T, typename U>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<U>
[[nodiscard]] constexpr auto gt(
    const result<T>& lhs,
    U rhs) noexcept -> result<bool>
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    return gt(*lhs, rhs);
}

template<typename T, typename U>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<U>
[[nodiscard]] constexpr auto gt(
    T lhs,
    const result<U>& rhs) noexcept -> result<bool>
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return gt(lhs, *rhs);
}

template<typename T, typename U>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<U>
[[nodiscard]] constexpr auto gt(
    const result<T>& lhs,
    const result<U>& rhs) noexcept -> result<bool>
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return gt(*lhs, *rhs);
}

template<typename T, typename U>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<U>
[[nodiscard]] constexpr auto ge(
    const result<T>& lhs,
    U rhs) noexcept -> result<bool>
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    return ge(*lhs, rhs);
}

template<typename T, typename U>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<U>
[[nodiscard]] constexpr auto ge(
    T lhs,
    const result<U>& rhs) noexcept -> result<bool>
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return ge(lhs, *rhs);
}

template<typename T, typename U>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<U>
[[nodiscard]] constexpr auto ge(
    const result<T>& lhs,
    const result<U>& rhs) noexcept -> result<bool>
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return ge(*lhs, *rhs);
}

} // namespace xer

#endif /* XER_BITS_COMPARE_H_INCLUDED_ */

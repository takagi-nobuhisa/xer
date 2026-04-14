/**
 * @file xer/bits/minmax.h
 * @brief Internal min/max function implementations.
 */

#pragma once

#ifndef XER_BITS_MINMAX_H_INCLUDED_
#define XER_BITS_MINMAX_H_INCLUDED_

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include <cmath>
#include <concepts>
#include <expected>
#include <type_traits>

#include <xer/bits/arithmetic_concepts.h>
#include <xer/bits/in_range.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Common result type for max/min.
 *
 * @tparam T Left operand type.
 * @tparam U Right operand type.
 */
template<typename T, typename U>
using minmax_common_t = std::common_type_t<T, U>;

/**
 * @brief Unsigned common result type for umax/umin.
 *
 * @tparam T Left operand type.
 * @tparam U Right operand type.
 */
template<typename T, typename U>
using uminmax_common_t =
    std::make_unsigned_t<std::common_type_t<T, U>>;

/**
 * @brief Converts an arithmetic value to long double for mixed-type comparison.
 *
 * @tparam T Arithmetic type.
 * @param value Source value.
 * @return Converted value.
 */
template<typename T>
    requires arithmetic<T>
[[nodiscard]] constexpr auto to_minmax_long_double(T value) noexcept
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
[[nodiscard]] constexpr auto minmax_is_nan_value(T value) noexcept -> bool
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
[[nodiscard]] constexpr auto minmax_less(L lhs, R rhs) noexcept -> bool
{
    return to_minmax_long_double(lhs) < to_minmax_long_double(rhs);
}

/**
 * @brief Converts a selected value to the result type after range checking.
 *
 * @tparam R Result type.
 * @tparam V Source value type.
 * @param value Selected value.
 * @return Converted result or out_of_range.
 */
template<typename R, typename V>
    requires arithmetic<R> && arithmetic<V>
[[nodiscard]] constexpr auto minmax_cast(V value) -> result<R>
{
    if (!in_range<R>(value)) {
        return std::unexpected(make_error(error_t::out_of_range));
    }

    return static_cast<R>(value);
}

/**
 * @brief Validates min/max arguments.
 *
 * This rejects NaN operands.
 *
 * @tparam T Left operand type.
 * @tparam U Right operand type.
 * @param lhs Left operand.
 * @param rhs Right operand.
 * @return Success or invalid_argument.
 */
template<typename T, typename U>
    requires arithmetic<T> && arithmetic<U>
[[nodiscard]] constexpr auto validate_minmax_arguments(T lhs, U rhs)
    -> result<void>
{
    if (minmax_is_nan_value(lhs) || minmax_is_nan_value(rhs)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return {};
}

/**
 * @brief Selects the larger of two arithmetic values.
 *
 * @tparam T Left operand type.
 * @tparam U Right operand type.
 * @param lhs Left operand.
 * @param rhs Right operand.
 * @return Selected value converted to common type or error.
 */
template<typename T, typename U>
    requires arithmetic<T> && arithmetic<U>
[[nodiscard]] constexpr auto max_impl(T lhs, U rhs)
    -> result<minmax_common_t<T, U>>
{
    using result_type = minmax_common_t<T, U>;

    const auto validation = validate_minmax_arguments(lhs, rhs);
    if (!validation.has_value()) {
        return std::unexpected(validation.error());
    }

    if (minmax_less(lhs, rhs)) {
        return minmax_cast<result_type>(rhs);
    }

    return minmax_cast<result_type>(lhs);
}

/**
 * @brief Selects the smaller of two arithmetic values.
 *
 * @tparam T Left operand type.
 * @tparam U Right operand type.
 * @param lhs Left operand.
 * @param rhs Right operand.
 * @return Selected value converted to common type or error.
 */
template<typename T, typename U>
    requires arithmetic<T> && arithmetic<U>
[[nodiscard]] constexpr auto min_impl(T lhs, U rhs)
    -> result<minmax_common_t<T, U>>
{
    using result_type = minmax_common_t<T, U>;

    const auto validation = validate_minmax_arguments(lhs, rhs);
    if (!validation.has_value()) {
        return std::unexpected(validation.error());
    }

    if (minmax_less(rhs, lhs)) {
        return minmax_cast<result_type>(rhs);
    }

    return minmax_cast<result_type>(lhs);
}

/**
 * @brief Selects the larger of two integer values and returns an unsigned type.
 *
 * @tparam T Left operand type.
 * @tparam U Right operand type.
 * @param lhs Left operand.
 * @param rhs Right operand.
 * @return Selected value converted to unsigned common type or error.
 */
template<typename T, typename U>
    requires std::integral<T> && std::integral<U>
[[nodiscard]] constexpr auto umax_impl(T lhs, U rhs)
    -> result<uminmax_common_t<T, U>>
{
    using result_type = uminmax_common_t<T, U>;

    if (minmax_less(lhs, rhs)) {
        return minmax_cast<result_type>(rhs);
    }

    return minmax_cast<result_type>(lhs);
}

/**
 * @brief Selects the smaller of two integer values and returns an unsigned type.
 *
 * @tparam T Left operand type.
 * @tparam U Right operand type.
 * @param lhs Left operand.
 * @param rhs Right operand.
 * @return Selected value converted to unsigned common type or error.
 */
template<typename T, typename U>
    requires std::integral<T> && std::integral<U>
[[nodiscard]] constexpr auto umin_impl(T lhs, U rhs)
    -> result<uminmax_common_t<T, U>>
{
    using result_type = uminmax_common_t<T, U>;

    if (minmax_less(rhs, lhs)) {
        return minmax_cast<result_type>(rhs);
    }

    return minmax_cast<result_type>(lhs);
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Returns the larger of two arithmetic values.
 *
 * The return type is `std::common_type_t<T, U>`.
 * If the selected value is not representable by that type,
 * this function returns out_of_range.
 *
 * @tparam T Left operand type.
 * @tparam U Right operand type.
 * @param lhs Left operand.
 * @param rhs Right operand.
 * @return Larger value or error.
 */
template<typename T, typename U>
    requires arithmetic<T> && arithmetic<U>
[[nodiscard]] constexpr auto max(T lhs, U rhs)
    -> result<std::common_type_t<T, U>>
{
    return detail::max_impl(lhs, rhs);
}

/**
 * @brief Returns the larger of two arithmetic values.
 *
 * Errors in either operand are propagated unchanged.
 *
 * @tparam T Left operand type.
 * @tparam U Right operand type.
 * @param lhs Left operand result.
 * @param rhs Right operand result.
 * @return Larger value or error.
 */
template<typename T, typename U>
    requires arithmetic<T> && arithmetic<U>
[[nodiscard]] constexpr auto max(const result<T>& lhs, const result<U>& rhs)
    -> result<std::common_type_t<T, U>>
{
    if (!lhs.has_value()) {
        return std::unexpected(lhs.error());
    }

    if (!rhs.has_value()) {
        return std::unexpected(rhs.error());
    }

    return max(*lhs, *rhs);
}

/**
 * @brief Returns the larger of two arithmetic values.
 *
 * Errors in the left operand are propagated unchanged.
 *
 * @tparam T Left operand type.
 * @tparam U Right operand type.
 * @param lhs Left operand result.
 * @param rhs Right operand.
 * @return Larger value or error.
 */
template<typename T, typename U>
    requires arithmetic<T> && arithmetic<U>
[[nodiscard]] constexpr auto max(const result<T>& lhs, U rhs)
    -> result<std::common_type_t<T, U>>
{
    if (!lhs.has_value()) {
        return std::unexpected(lhs.error());
    }

    return max(*lhs, rhs);
}

/**
 * @brief Returns the larger of two arithmetic values.
 *
 * Errors in the right operand are propagated unchanged.
 *
 * @tparam T Left operand type.
 * @tparam U Right operand type.
 * @param lhs Left operand.
 * @param rhs Right operand result.
 * @return Larger value or error.
 */
template<typename T, typename U>
    requires arithmetic<T> && arithmetic<U>
[[nodiscard]] constexpr auto max(T lhs, const result<U>& rhs)
    -> result<std::common_type_t<T, U>>
{
    if (!rhs.has_value()) {
        return std::unexpected(rhs.error());
    }

    return max(lhs, *rhs);
}

/**
 * @brief Returns the smaller of two arithmetic values.
 *
 * The return type is `std::common_type_t<T, U>`.
 * If the selected value is not representable by that type,
 * this function returns out_of_range.
 *
 * @tparam T Left operand type.
 * @tparam U Right operand type.
 * @param lhs Left operand.
 * @param rhs Right operand.
 * @return Smaller value or error.
 */
template<typename T, typename U>
    requires arithmetic<T> && arithmetic<U>
[[nodiscard]] constexpr auto min(T lhs, U rhs)
    -> result<std::common_type_t<T, U>>
{
    return detail::min_impl(lhs, rhs);
}

/**
 * @brief Returns the smaller of two arithmetic values.
 *
 * Errors in either operand are propagated unchanged.
 *
 * @tparam T Left operand type.
 * @tparam U Right operand type.
 * @param lhs Left operand result.
 * @param rhs Right operand result.
 * @return Smaller value or error.
 */
template<typename T, typename U>
    requires arithmetic<T> && arithmetic<U>
[[nodiscard]] constexpr auto min(const result<T>& lhs, const result<U>& rhs)
    -> result<std::common_type_t<T, U>>
{
    if (!lhs.has_value()) {
        return std::unexpected(lhs.error());
    }

    if (!rhs.has_value()) {
        return std::unexpected(rhs.error());
    }

    return min(*lhs, *rhs);
}

/**
 * @brief Returns the smaller of two arithmetic values.
 *
 * Errors in the left operand are propagated unchanged.
 *
 * @tparam T Left operand type.
 * @tparam U Right operand type.
 * @param lhs Left operand result.
 * @param rhs Right operand.
 * @return Smaller value or error.
 */
template<typename T, typename U>
    requires arithmetic<T> && arithmetic<U>
[[nodiscard]] constexpr auto min(const result<T>& lhs, U rhs)
    -> result<std::common_type_t<T, U>>
{
    if (!lhs.has_value()) {
        return std::unexpected(lhs.error());
    }

    return min(*lhs, rhs);
}

/**
 * @brief Returns the smaller of two arithmetic values.
 *
 * Errors in the right operand are propagated unchanged.
 *
 * @tparam T Left operand type.
 * @tparam U Right operand type.
 * @param lhs Left operand.
 * @param rhs Right operand result.
 * @return Smaller value or error.
 */
template<typename T, typename U>
    requires arithmetic<T> && arithmetic<U>
[[nodiscard]] constexpr auto min(T lhs, const result<U>& rhs)
    -> result<std::common_type_t<T, U>>
{
    if (!rhs.has_value()) {
        return std::unexpected(rhs.error());
    }

    return min(lhs, *rhs);
}

/**
 * @brief Returns the larger of two integer values as an unsigned common type.
 *
 * The return type is `std::make_unsigned_t<std::common_type_t<T, U>>`.
 * If the selected value is not representable by that type,
 * this function returns out_of_range.
 *
 * @tparam T Left operand type.
 * @tparam U Right operand type.
 * @param lhs Left operand.
 * @param rhs Right operand.
 * @return Larger value or error.
 */
template<typename T, typename U>
    requires std::integral<T> && std::integral<U>
[[nodiscard]] constexpr auto umax(T lhs, U rhs)
    -> result<std::make_unsigned_t<std::common_type_t<T, U>>>
{
    return detail::umax_impl(lhs, rhs);
}

/**
 * @brief Returns the larger of two integer values as an unsigned common type.
 *
 * Errors in either operand are propagated unchanged.
 *
 * @tparam T Left operand type.
 * @tparam U Right operand type.
 * @param lhs Left operand result.
 * @param rhs Right operand result.
 * @return Larger value or error.
 */
template<typename T, typename U>
    requires std::integral<T> && std::integral<U>
[[nodiscard]] constexpr auto umax(
    const result<T>& lhs,
    const result<U>& rhs)
    -> result<std::make_unsigned_t<std::common_type_t<T, U>>>
{
    if (!lhs.has_value()) {
        return std::unexpected(lhs.error());
    }

    if (!rhs.has_value()) {
        return std::unexpected(rhs.error());
    }

    return umax(*lhs, *rhs);
}

/**
 * @brief Returns the larger of two integer values as an unsigned common type.
 *
 * Errors in the left operand are propagated unchanged.
 *
 * @tparam T Left operand type.
 * @tparam U Right operand type.
 * @param lhs Left operand result.
 * @param rhs Right operand.
 * @return Larger value or error.
 */
template<typename T, typename U>
    requires std::integral<T> && std::integral<U>
[[nodiscard]] constexpr auto umax(const result<T>& lhs, U rhs)
    -> result<std::make_unsigned_t<std::common_type_t<T, U>>>
{
    if (!lhs.has_value()) {
        return std::unexpected(lhs.error());
    }

    return umax(*lhs, rhs);
}

/**
 * @brief Returns the larger of two integer values as an unsigned common type.
 *
 * Errors in the right operand are propagated unchanged.
 *
 * @tparam T Left operand type.
 * @tparam U Right operand type.
 * @param lhs Left operand.
 * @param rhs Right operand result.
 * @return Larger value or error.
 */
template<typename T, typename U>
    requires std::integral<T> && std::integral<U>
[[nodiscard]] constexpr auto umax(T lhs, const result<U>& rhs)
    -> result<std::make_unsigned_t<std::common_type_t<T, U>>>
{
    if (!rhs.has_value()) {
        return std::unexpected(rhs.error());
    }

    return umax(lhs, *rhs);
}

/**
 * @brief Returns the smaller of two integer values as an unsigned common type.
 *
 * The return type is `std::make_unsigned_t<std::common_type_t<T, U>>`.
 * If the selected value is not representable by that type,
 * this function returns out_of_range.
 *
 * @tparam T Left operand type.
 * @tparam U Right operand type.
 * @param lhs Left operand.
 * @param rhs Right operand.
 * @return Smaller value or error.
 */
template<typename T, typename U>
    requires std::integral<T> && std::integral<U>
[[nodiscard]] constexpr auto umin(T lhs, U rhs)
    -> result<std::make_unsigned_t<std::common_type_t<T, U>>>
{
    return detail::umin_impl(lhs, rhs);
}

/**
 * @brief Returns the smaller of two integer values as an unsigned common type.
 *
 * Errors in either operand are propagated unchanged.
 *
 * @tparam T Left operand type.
 * @tparam U Right operand type.
 * @param lhs Left operand result.
 * @param rhs Right operand result.
 * @return Smaller value or error.
 */
template<typename T, typename U>
    requires std::integral<T> && std::integral<U>
[[nodiscard]] constexpr auto umin(
    const result<T>& lhs,
    const result<U>& rhs)
    -> result<std::make_unsigned_t<std::common_type_t<T, U>>>
{
    if (!lhs.has_value()) {
        return std::unexpected(lhs.error());
    }

    if (!rhs.has_value()) {
        return std::unexpected(rhs.error());
    }

    return umin(*lhs, *rhs);
}

/**
 * @brief Returns the smaller of two integer values as an unsigned common type.
 *
 * Errors in the left operand are propagated unchanged.
 *
 * @tparam T Left operand type.
 * @tparam U Right operand type.
 * @param lhs Left operand result.
 * @param rhs Right operand.
 * @return Smaller value or error.
 */
template<typename T, typename U>
    requires std::integral<T> && std::integral<U>
[[nodiscard]] constexpr auto umin(const result<T>& lhs, U rhs)
    -> result<std::make_unsigned_t<std::common_type_t<T, U>>>
{
    if (!lhs.has_value()) {
        return std::unexpected(lhs.error());
    }

    return umin(*lhs, rhs);
}

/**
 * @brief Returns the smaller of two integer values as an unsigned common type.
 *
 * Errors in the right operand are propagated unchanged.
 *
 * @tparam T Left operand type.
 * @tparam U Right operand type.
 * @param lhs Left operand.
 * @param rhs Right operand result.
 * @return Smaller value or error.
 */
template<typename T, typename U>
    requires std::integral<T> && std::integral<U>
[[nodiscard]] constexpr auto umin(T lhs, const result<U>& rhs)
    -> result<std::make_unsigned_t<std::common_type_t<T, U>>>
{
    if (!rhs.has_value()) {
        return std::unexpected(rhs.error());
    }

    return umin(lhs, *rhs);
}

} // namespace xer

#endif /* XER_BITS_MINMAX_H_INCLUDED_ */

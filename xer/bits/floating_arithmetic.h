/**
 * @file xer/bits/floating_arithmetic.h
 * @brief Internal floating-point arithmetic function implementations.
 */

#pragma once

#ifndef XER_BITS_FLOATING_ARITHMETIC_H_INCLUDED_
#define XER_BITS_FLOATING_ARITHMETIC_H_INCLUDED_

#include <cmath>
#include <concepts>
#include <expected>
#include <limits>
#include <type_traits>
#include <utility>

#include <xer/bits/common.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Reports whether `T` is an arithmetic operand accepted by floating-point arithmetic functions.
 *
 * @tparam T Type to test.
 */
template<typename T>
inline constexpr bool is_floating_arithmetic_operand_v =
    (std::integral<std::remove_cvref_t<T>> &&
     !std::same_as<std::remove_cvref_t<T>, bool>) ||
    std::floating_point<std::remove_cvref_t<T>>;

/**
 * @brief Reports whether a binary arithmetic operation should be handled by floating-point arithmetic functions.
 *
 * At least one operand must be a floating-point type.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 */
template<typename A, typename B>
inline constexpr bool is_floating_arithmetic_pair_v =
    is_floating_arithmetic_operand_v<A> &&
    is_floating_arithmetic_operand_v<B> &&
    (std::floating_point<std::remove_cvref_t<A>> ||
     std::floating_point<std::remove_cvref_t<B>>);

/**
 * @brief Converts an arithmetic operand to `long double`.
 *
 * @tparam T Operand type.
 * @param value Operand value.
 * @return Converted value.
 */
template<typename T>
    requires(is_floating_arithmetic_operand_v<T>)
[[nodiscard]] constexpr long double to_long_double(T value) noexcept
{
    return static_cast<long double>(value);
}

/**
 * @brief Reports whether a floating-point value is finite.
 *
 * @param value Value to test.
 * @return `true` if finite.
 */
[[nodiscard]] inline bool is_finite(long double value) noexcept
{
    return std::isfinite(value);
}

/**
 * @brief Creates a domain error for floating-point arithmetic.
 *
 * @return Domain error.
 */
[[nodiscard]] constexpr error<void> make_floating_domain_error() noexcept
{
    return make_error(error_t::dom);
}

/**
 * @brief Validates a floating-point operand value.
 *
 * @param value Value to validate.
 * @return Success if finite, otherwise an error.
 */
[[nodiscard]] inline result<long double>
validate_operand(long double value) noexcept
{
    if (!is_finite(value)) {
        return std::unexpected(make_floating_domain_error());
    }

    return value;
}

/**
 * @brief Validates a floating-point result value.
 *
 * @param value Value to validate.
 * @return Success if finite, otherwise an error.
 */
[[nodiscard]] inline result<long double>
validate_result(long double value) noexcept
{
    if (!is_finite(value)) {
        return std::unexpected(make_floating_domain_error());
    }

    return value;
}

/**
 * @brief Implements floating-point addition.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Sum as `long double`.
 */
template<typename A, typename B>
    requires(is_floating_arithmetic_pair_v<A, B>)
[[nodiscard]] inline result<long double> add_floating(
    A lhs,
    B rhs) noexcept
{
    const auto left = validate_operand(to_long_double(lhs));
    if (!left) {
        return std::unexpected(left.error());
    }

    const auto right = validate_operand(to_long_double(rhs));
    if (!right) {
        return std::unexpected(right.error());
    }

    return validate_result(*left + *right);
}

/**
 * @brief Implements floating-point subtraction.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Difference as `long double`.
 */
template<typename A, typename B>
    requires(is_floating_arithmetic_pair_v<A, B>)
[[nodiscard]] inline result<long double> sub_floating(
    A lhs,
    B rhs) noexcept
{
    const auto left = validate_operand(to_long_double(lhs));
    if (!left) {
        return std::unexpected(left.error());
    }

    const auto right = validate_operand(to_long_double(rhs));
    if (!right) {
        return std::unexpected(right.error());
    }

    return validate_result(*left - *right);
}

/**
 * @brief Implements floating-point multiplication.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Product as `long double`.
 */
template<typename A, typename B>
    requires(is_floating_arithmetic_pair_v<A, B>)
[[nodiscard]] inline result<long double> mul_floating(
    A lhs,
    B rhs) noexcept
{
    const auto left = validate_operand(to_long_double(lhs));
    if (!left) {
        return std::unexpected(left.error());
    }

    const auto right = validate_operand(to_long_double(rhs));
    if (!right) {
        return std::unexpected(right.error());
    }

    return validate_result(*left * *right);
}

/**
 * @brief Implements floating-point division without remainder.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Quotient as `long double`.
 */
template<typename A, typename B>
    requires(is_floating_arithmetic_pair_v<A, B>)
[[nodiscard]] inline result<long double> div_floating(
    A lhs,
    B rhs) noexcept
{
    const auto left = validate_operand(to_long_double(lhs));
    if (!left) {
        return std::unexpected(left.error());
    }

    const auto right = validate_operand(to_long_double(rhs));
    if (!right) {
        return std::unexpected(right.error());
    }

    if (*right == 0.0L) {
        return std::unexpected(make_floating_domain_error());
    }

    return validate_result(*left / *right);
}

/**
 * @brief Implements floating-point division with remainder.
 *
 * The quotient is `trunc(lhs / rhs)`, and the remainder is `lhs - rhs * quotient`.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @param rem Remainder output pointer. May be null.
 * @return Quotient as `long double`.
 */
template<typename A, typename B>
    requires(is_floating_arithmetic_pair_v<A, B>)
[[nodiscard]] inline result<long double> div_floating(
    A lhs,
    B rhs,
    long double* rem) noexcept
{
    const auto left = validate_operand(to_long_double(lhs));
    if (!left) {
        return std::unexpected(left.error());
    }

    const auto right = validate_operand(to_long_double(rhs));
    if (!right) {
        return std::unexpected(right.error());
    }

    if (*right == 0.0L) {
        return std::unexpected(make_floating_domain_error());
    }

    const long double raw_quotient = *left / *right;
    if (!is_finite(raw_quotient)) {
        return std::unexpected(make_floating_domain_error());
    }

    const long double quotient = std::trunc(raw_quotient);
    if (!is_finite(quotient)) {
        return std::unexpected(make_floating_domain_error());
    }

    const long double remainder = *left - *right * quotient;
    if (!is_finite(remainder)) {
        return std::unexpected(make_floating_domain_error());
    }

    if (rem != nullptr) {
        *rem = remainder;
    }

    return quotient;
}

/**
 * @brief Implements floating-point modulo.
 *
 * The result is `lhs - rhs * trunc(lhs / rhs)`.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Remainder as `long double`.
 */
template<typename A, typename B>
    requires(is_floating_arithmetic_pair_v<A, B>)
[[nodiscard]] inline result<long double> mod_floating(
    A lhs,
    B rhs) noexcept
{
    long double remainder = 0.0L;
    const auto quotient = div_floating(lhs, rhs, &remainder);
    if (!quotient) {
        return std::unexpected(quotient.error());
    }

    return remainder;
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Adds two operands using floating-point arithmetic rules.
 *
 * This overload participates only when at least one operand is a floating-point type.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Sum as `long double`.
 */
template<typename A, typename B>
    requires(detail::is_floating_arithmetic_pair_v<A, B>)
[[nodiscard]] inline result<long double> add(
    A lhs,
    B rhs) noexcept
{
    return detail::add_floating(lhs, rhs);
}

/**
 * @brief Subtracts two operands using floating-point arithmetic rules.
 *
 * This overload participates only when at least one operand is a floating-point type.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Difference as `long double`.
 */
template<typename A, typename B>
    requires(detail::is_floating_arithmetic_pair_v<A, B>)
[[nodiscard]] inline result<long double> sub(
    A lhs,
    B rhs) noexcept
{
    return detail::sub_floating(lhs, rhs);
}

/**
 * @brief Multiplies two operands using floating-point arithmetic rules.
 *
 * This overload participates only when at least one operand is a floating-point type.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Product as `long double`.
 */
template<typename A, typename B>
    requires(detail::is_floating_arithmetic_pair_v<A, B>)
[[nodiscard]] inline result<long double> mul(
    A lhs,
    B rhs) noexcept
{
    return detail::mul_floating(lhs, rhs);
}

/**
 * @brief Divides two operands using floating-point arithmetic rules.
 *
 * This overload participates only when at least one operand is a floating-point type.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Quotient as `long double`.
 */
template<typename A, typename B>
    requires(detail::is_floating_arithmetic_pair_v<A, B>)
[[nodiscard]] inline result<long double> div(
    A lhs,
    B rhs) noexcept
{
    return detail::div_floating(lhs, rhs);
}

/**
 * @brief Divides two operands using floating-point arithmetic rules and stores the remainder.
 *
 * This overload participates only when at least one operand is a floating-point type.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @param rem Remainder output pointer. May be null.
 * @return Quotient as `long double`.
 */
template<typename A, typename B>
    requires(detail::is_floating_arithmetic_pair_v<A, B>)
[[nodiscard]] inline result<long double> div(
    A lhs,
    B rhs,
    long double* rem) noexcept
{
    return detail::div_floating(lhs, rhs, rem);
}

/**
 * @brief Returns the floating-point remainder.
 *
 * This overload participates only when at least one operand is a floating-point type.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Remainder as `long double`.
 */
template<typename A, typename B>
    requires(detail::is_floating_arithmetic_pair_v<A, B>)
[[nodiscard]] inline result<long double> mod(
    A lhs,
    B rhs) noexcept
{
    return detail::mod_floating(lhs, rhs);
}

/**
 * @brief Propagates an error from the left operand if present.
 *
 * @tparam T Expected value type.
 * @tparam U Right-hand side type.
 * @param lhs Left-hand side expected value.
 * @param rhs Right-hand side operand.
 * @return Result of `add`.
 */
template<typename T, typename U>
    requires(detail::is_floating_arithmetic_pair_v<T, U>)
[[nodiscard]] inline auto add(
    const result<T>& lhs,
    U rhs) noexcept -> decltype(add(std::declval<T>(), rhs))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    return add(*lhs, rhs);
}

template<typename T, typename U>
    requires(detail::is_floating_arithmetic_pair_v<T, U>)
[[nodiscard]] inline auto add(
    T lhs,
    const result<U>& rhs) noexcept -> decltype(add(lhs, std::declval<U>()))
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return add(lhs, *rhs);
}

template<typename T, typename U>
    requires(detail::is_floating_arithmetic_pair_v<T, U>)
[[nodiscard]] inline auto add(
    const result<T>& lhs,
    const result<U>& rhs) noexcept -> decltype(add(std::declval<T>(), std::declval<U>()))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return add(*lhs, *rhs);
}

template<typename T, typename U>
    requires(detail::is_floating_arithmetic_pair_v<T, U>)
[[nodiscard]] inline auto sub(
    const result<T>& lhs,
    U rhs) noexcept -> decltype(sub(std::declval<T>(), rhs))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    return sub(*lhs, rhs);
}

template<typename T, typename U>
    requires(detail::is_floating_arithmetic_pair_v<T, U>)
[[nodiscard]] inline auto sub(
    T lhs,
    const result<U>& rhs) noexcept -> decltype(sub(lhs, std::declval<U>()))
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return sub(lhs, *rhs);
}

template<typename T, typename U>
    requires(detail::is_floating_arithmetic_pair_v<T, U>)
[[nodiscard]] inline auto sub(
    const result<T>& lhs,
    const result<U>& rhs) noexcept -> decltype(sub(std::declval<T>(), std::declval<U>()))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return sub(*lhs, *rhs);
}

template<typename T, typename U>
    requires(detail::is_floating_arithmetic_pair_v<T, U>)
[[nodiscard]] inline auto mul(
    const result<T>& lhs,
    U rhs) noexcept -> decltype(mul(std::declval<T>(), rhs))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    return mul(*lhs, rhs);
}

template<typename T, typename U>
    requires(detail::is_floating_arithmetic_pair_v<T, U>)
[[nodiscard]] inline auto mul(
    T lhs,
    const result<U>& rhs) noexcept -> decltype(mul(lhs, std::declval<U>()))
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return mul(lhs, *rhs);
}

template<typename T, typename U>
    requires(detail::is_floating_arithmetic_pair_v<T, U>)
[[nodiscard]] inline auto mul(
    const result<T>& lhs,
    const result<U>& rhs) noexcept -> decltype(mul(std::declval<T>(), std::declval<U>()))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return mul(*lhs, *rhs);
}

template<typename T, typename U>
    requires(detail::is_floating_arithmetic_pair_v<T, U>)
[[nodiscard]] inline auto div(
    const result<T>& lhs,
    U rhs) noexcept -> decltype(div(std::declval<T>(), rhs))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    return div(*lhs, rhs);
}

template<typename T, typename U>
    requires(detail::is_floating_arithmetic_pair_v<T, U>)
[[nodiscard]] inline auto div(
    T lhs,
    const result<U>& rhs) noexcept -> decltype(div(lhs, std::declval<U>()))
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return div(lhs, *rhs);
}

template<typename T, typename U>
    requires(detail::is_floating_arithmetic_pair_v<T, U>)
[[nodiscard]] inline auto div(
    const result<T>& lhs,
    const result<U>& rhs) noexcept -> decltype(div(std::declval<T>(), std::declval<U>()))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return div(*lhs, *rhs);
}

template<typename T, typename U>
    requires(detail::is_floating_arithmetic_pair_v<T, U>)
[[nodiscard]] inline auto div(
    const result<T>& lhs,
    U rhs,
    long double* rem) noexcept -> decltype(div(std::declval<T>(), rhs, rem))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    return div(*lhs, rhs, rem);
}

template<typename T, typename U>
    requires(detail::is_floating_arithmetic_pair_v<T, U>)
[[nodiscard]] inline auto div(
    T lhs,
    const result<U>& rhs,
    long double* rem) noexcept -> decltype(div(lhs, std::declval<U>(), rem))
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return div(lhs, *rhs, rem);
}

template<typename T, typename U>
    requires(detail::is_floating_arithmetic_pair_v<T, U>)
[[nodiscard]] inline auto div(
    const result<T>& lhs,
    const result<U>& rhs,
    long double* rem) noexcept -> decltype(div(std::declval<T>(), std::declval<U>(), rem))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return div(*lhs, *rhs, rem);
}

template<typename T, typename U>
    requires(detail::is_floating_arithmetic_pair_v<T, U>)
[[nodiscard]] inline auto mod(
    const result<T>& lhs,
    U rhs) noexcept -> decltype(mod(std::declval<T>(), rhs))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    return mod(*lhs, rhs);
}

template<typename T, typename U>
    requires(detail::is_floating_arithmetic_pair_v<T, U>)
[[nodiscard]] inline auto mod(
    T lhs,
    const result<U>& rhs) noexcept -> decltype(mod(lhs, std::declval<U>()))
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return mod(lhs, *rhs);
}

template<typename T, typename U>
    requires(detail::is_floating_arithmetic_pair_v<T, U>)
[[nodiscard]] inline auto mod(
    const result<T>& lhs,
    const result<U>& rhs) noexcept -> decltype(mod(std::declval<T>(), std::declval<U>()))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return mod(*lhs, *rhs);
}

} // namespace xer

#endif

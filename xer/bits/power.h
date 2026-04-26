/**
 * @file xer/bits/power.h
 * @brief Internal square and cube arithmetic helpers.
 */

#pragma once

#ifndef XER_BITS_POWER_H_INCLUDED_
#define XER_BITS_POWER_H_INCLUDED_

#include <concepts>
#include <expected>
#include <type_traits>

#include <xer/bits/arithmetic_concepts.h>
#include <xer/error.h>
#include <xer/stdint.h>

namespace xer {

/**
 * @brief Squares an integer value.
 *
 * This function uses XER multiplication rules instead of the built-in `*`
 * operator directly. Overflow and range errors are reported through
 * `xer::result`.
 *
 * @tparam T Integer operand type except `bool`.
 * @param value Operand to square.
 * @return Squared value as `xer::int64_t`, or an error.
 */
template<typename T>
    requires(std::integral<std::remove_cvref_t<T>> &&
             !std::same_as<std::remove_cvref_t<T>, bool>)
[[nodiscard]] constexpr auto sq(T value) noexcept -> result<xer::int64_t>
{
    return mul(value, value);
}

/**
 * @brief Squares a floating-point value.
 *
 * This function follows XER floating-point arithmetic rules. Non-finite
 * operands or non-finite results are reported as domain errors by the
 * underlying multiplication function.
 *
 * @tparam T Floating-point operand type.
 * @param value Operand to square.
 * @return Squared value as `long double`, or an error.
 */
template<typename T>
    requires(std::floating_point<std::remove_cvref_t<T>>)
[[nodiscard]] inline auto sq(T value) noexcept -> result<long double>
{
    return mul(value, value);
}

/**
 * @brief Cubes an integer value.
 *
 * The calculation is performed as two XER multiplications. If the first
 * multiplication fails, that error is propagated. Otherwise, the second
 * multiplication is performed with the squared value.
 *
 * @tparam T Integer operand type except `bool`.
 * @param value Operand to cube.
 * @return Cubed value as `xer::int64_t`, or an error.
 */
template<typename T>
    requires(std::integral<std::remove_cvref_t<T>> &&
             !std::same_as<std::remove_cvref_t<T>, bool>)
[[nodiscard]] constexpr auto cb(T value) noexcept -> result<xer::int64_t>
{
    const auto squared = sq(value);
    if (!squared) {
        return std::unexpected(squared.error());
    }

    return mul(*squared, value);
}

/**
 * @brief Cubes a floating-point value.
 *
 * The calculation is performed as two XER multiplications and follows the
 * same finite-value validation policy as the other floating-point arithmetic
 * helpers.
 *
 * @tparam T Floating-point operand type.
 * @param value Operand to cube.
 * @return Cubed value as `long double`, or an error.
 */
template<typename T>
    requires(std::floating_point<std::remove_cvref_t<T>>)
[[nodiscard]] inline auto cb(T value) noexcept -> result<long double>
{
    const auto squared = sq(value);
    if (!squared) {
        return std::unexpected(squared.error());
    }

    return mul(*squared, value);
}

/**
 * @brief Squares a successful integer result or propagates its error.
 *
 * @tparam T Integer value type except `bool`.
 * @param value Result value to square.
 * @return Squared value, or the original error.
 */
template<typename T>
    requires(std::integral<T> && !std::same_as<T, bool>)
[[nodiscard]] constexpr auto sq(const result<T>& value) noexcept
    -> result<xer::int64_t>
{
    if (!value) {
        return std::unexpected(value.error());
    }

    return sq(*value);
}

/**
 * @brief Squares a successful floating-point result or propagates its error.
 *
 * @tparam T Floating-point value type.
 * @param value Result value to square.
 * @return Squared value, or the original error.
 */
template<typename T>
    requires(std::floating_point<T>)
[[nodiscard]] inline auto sq(const result<T>& value) noexcept
    -> result<long double>
{
    if (!value) {
        return std::unexpected(value.error());
    }

    return sq(*value);
}

/**
 * @brief Cubes a successful integer result or propagates its error.
 *
 * @tparam T Integer value type except `bool`.
 * @param value Result value to cube.
 * @return Cubed value, or the original error.
 */
template<typename T>
    requires(std::integral<T> && !std::same_as<T, bool>)
[[nodiscard]] constexpr auto cb(const result<T>& value) noexcept
    -> result<xer::int64_t>
{
    if (!value) {
        return std::unexpected(value.error());
    }

    return cb(*value);
}

/**
 * @brief Cubes a successful floating-point result or propagates its error.
 *
 * @tparam T Floating-point value type.
 * @param value Result value to cube.
 * @return Cubed value, or the original error.
 */
template<typename T>
    requires(std::floating_point<T>)
[[nodiscard]] inline auto cb(const result<T>& value) noexcept
    -> result<long double>
{
    if (!value) {
        return std::unexpected(value.error());
    }

    return cb(*value);
}

} // namespace xer

#endif /* XER_BITS_POWER_H_INCLUDED_ */

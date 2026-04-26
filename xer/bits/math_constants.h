/**
 * @file xer/bits/math_constants.h
 * @brief Internal mathematical constants.
 */

#pragma once

#ifndef XER_BITS_MATH_CONSTANTS_H_INCLUDED_
#define XER_BITS_MATH_CONSTANTS_H_INCLUDED_

#include <complex>
#include <concepts>
#include <numbers>

namespace xer {

/**
 * @brief Template variable for the mathematical constant pi.
 *
 * @tparam T Floating-point type.
 */
template<std::floating_point T>
inline constexpr T pi_v = std::numbers::pi_v<T>;

/**
 * @brief Unicode alias for @ref pi_v.
 *
 * @tparam T Floating-point type.
 */
template<std::floating_point T>
inline constexpr T 𝜋 = pi_v<T>;

/**
 * @brief Template variable for the mathematical constant tau.
 *
 * Tau is defined as 2 * pi.
 *
 * @tparam T Floating-point type.
 */
template<std::floating_point T>
inline constexpr T tau_v = std::numbers::pi_v<T> * static_cast<T>(2);

/**
 * @brief Unicode alias for @ref tau_v.
 *
 * @tparam T Floating-point type.
 */
template<std::floating_point T>
inline constexpr T τ = tau_v<T>;

/**
 * @brief Template variable for the square root of 2.
 *
 * @tparam T Floating-point type.
 */
template<std::floating_point T>
inline constexpr T sqrt_2_v = std::numbers::sqrt2_v<T>;

/**
 * @brief Short alias for @ref sqrt_2_v.
 *
 * @tparam T Floating-point type.
 */
template<std::floating_point T>
inline constexpr T sqrt_2 = sqrt_2_v<T>;

/**
 * @brief Template variable for the square root of 3.
 *
 * @tparam T Floating-point type.
 */
template<std::floating_point T>
inline constexpr T sqrt_3_v = static_cast<T>(
    1.732050807568877293527446341505872366942805253810380628055806979L);

/**
 * @brief Short alias for @ref sqrt_3_v.
 *
 * @tparam T Floating-point type.
 */
template<std::floating_point T>
inline constexpr T sqrt_3 = sqrt_3_v<T>;

/**
 * @brief Template variable for the square root of 5.
 *
 * @tparam T Floating-point type.
 */
template<std::floating_point T>
inline constexpr T sqrt_5_v = static_cast<T>(
    2.236067977499789696409173668731276235440618359611525724270897245L);

/**
 * @brief Short alias for @ref sqrt_5_v.
 *
 * @tparam T Floating-point type.
 */
template<std::floating_point T>
inline constexpr T sqrt_5 = sqrt_5_v<T>;

/**
 * @brief Template variable for the square root of 7.
 *
 * @tparam T Floating-point type.
 */
template<std::floating_point T>
inline constexpr T sqrt_7_v = static_cast<T>(
    2.645751311064590590501615753639260425710259183082450180368334459L);

/**
 * @brief Short alias for @ref sqrt_7_v.
 *
 * @tparam T Floating-point type.
 */
template<std::floating_point T>
inline constexpr T sqrt_7 = sqrt_7_v<T>;

/**
 * @brief Template variable for a primitive cube root of unity.
 *
 * This constant is defined as:
 *
 * @code
 * -1 / 2 + sqrt(3) / 2 * i
 * @endcode
 *
 * It satisfies:
 *
 * @code
 * omega_v<T> * omega_v<T> * omega_v<T> == 1
 * 1 + omega_v<T> + omega2_v<T> == 0
 * @endcode
 *
 * @tparam T Floating-point type.
 */
template<std::floating_point T>
inline constexpr std::complex<T> omega_v{
    static_cast<T>(-0.5L),
    sqrt_3_v<T> / static_cast<T>(2),
};

/**
 * @brief Template variable for the square of @ref omega_v.
 *
 * This constant is the other non-real cube root of unity.
 *
 * @tparam T Floating-point type.
 */
template<std::floating_point T>
inline constexpr std::complex<T> omega2_v{
    static_cast<T>(-0.5L),
    -sqrt_3_v<T> / static_cast<T>(2),
};

/**
 * @brief Unicode alias for @ref omega_v.
 *
 * @tparam T Floating-point type.
 */
template<std::floating_point T>
inline constexpr std::complex<T> ω = omega_v<T>;

/**
 * @brief Unicode alias for @ref omega2_v.
 *
 * @tparam T Floating-point type.
 */
template<std::floating_point T>
inline constexpr std::complex<T> ω² = omega2_v<T>;

} // namespace xer

#endif /* XER_BITS_MATH_CONSTANTS_H_INCLUDED_ */

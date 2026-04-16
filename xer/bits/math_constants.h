/**
 * @file xer/bits/math_constants.h
 * @brief Internal mathematical constants.
 */

#pragma once

#ifndef XER_BITS_MATH_CONSTANTS_H_INCLUDED_
#define XER_BITS_MATH_CONSTANTS_H_INCLUDED_

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

} // namespace xer

#endif /* XER_BITS_MATH_CONSTANTS_H_INCLUDED_ */

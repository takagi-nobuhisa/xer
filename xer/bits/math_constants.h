/**
 * @file xer/bits/math_constants.h
 * @brief Internal mathematical constants.
 */

#pragma once

#ifndef XER_BITS_MATH_CONSTANTS_H_INCLUDED_
#define XER_BITS_MATH_CONSTANTS_H_INCLUDED_

#include <concepts>

namespace xer {

/**
 * @brief Template variable for the mathematical constant pi.
 *
 * @tparam T Floating-point type.
 */
template<std::floating_point T>
inline constexpr T pi_v = static_cast<T>(3.141592653589793238462643383279502884L);

/**
 * @brief Unicode alias for @ref pi_v.
 *
 * @tparam T Floating-point type.
 */
template<std::floating_point T>
inline constexpr T 𝜋 = pi_v<T>;

} // namespace xer

#endif /* XER_BITS_MATH_CONSTANTS_H_INCLUDED_ */

/**
 * @file xer/bits/math_constants_non_standard_identifiers.h
 * @brief Optional non-standard identifier aliases for mathematical constants.
 */

#pragma once

#ifndef XER_BITS_MATH_CONSTANTS_NON_STANDARD_IDENTIFIERS_H_INCLUDED_
#define XER_BITS_MATH_CONSTANTS_NON_STANDARD_IDENTIFIERS_H_INCLUDED_

/**
 * @brief Non-standard Unicode alias for @ref omega2_v.
 *
 * This alias contains a superscript digit, which is not a standard C++
 * identifier character. This header is included only when
 * XER_ENABLE_NON_STANDARD_IDENTIFIERS is defined.
 *
 * @tparam T Floating-point type.
 */
template<std::floating_point T>
inline constexpr std::complex<T> ω² = omega2_v<T>;

#endif /* XER_BITS_MATH_CONSTANTS_NON_STANDARD_IDENTIFIERS_H_INCLUDED_ */

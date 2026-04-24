/**
 * @file xer/bits/numeric_limits.h
 * @brief Numeric limits helper constants.
 */

#pragma once

#ifndef XER_BITS_NUMERIC_LIMITS_H_INCLUDED_
#define XER_BITS_NUMERIC_LIMITS_H_INCLUDED_

#include <limits>

namespace xer {

/**
 * @brief Minimum finite value representable by `T`.
 *
 * This uses `std::numeric_limits<T>::lowest()` rather than `min()` so that
 * floating-point types return the most negative finite value.
 *
 * @tparam T Numeric type.
 */
template<typename T>
inline constexpr T min_of = std::numeric_limits<T>::lowest();

/**
 * @brief Maximum finite value representable by `T`.
 *
 * @tparam T Numeric type.
 */
template<typename T>
inline constexpr T max_of = std::numeric_limits<T>::max();

} // namespace xer

#endif /* XER_BITS_NUMERIC_LIMITS_H_INCLUDED_ */

/**
 * @file xer/bits/in_range.h
 * @brief Internal in_range function implementations.
 */

#pragma once

#ifndef XER_BITS_IN_RANGE_H_INCLUDED_
#define XER_BITS_IN_RANGE_H_INCLUDED_

#include <cmath>
#include <limits>
#include <type_traits>

#include <xer/bits/arithmetic_concepts.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Converts an arithmetic value to long double for range checks.
 *
 * @tparam T Arithmetic source type.
 * @param value Source value.
 * @return Converted value.
 */
template<typename T>
    requires arithmetic<T>
[[nodiscard]] constexpr auto to_in_range_long_double(T value) noexcept
    -> long double
{
    return static_cast<long double>(value);
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Checks whether an arithmetic value is within the numeric range of T.
 *
 * This function checks only whether @p value lies between
 * `std::numeric_limits<T>::lowest()` and `std::numeric_limits<T>::max()`.
 *
 * `T = bool` is intentionally rejected.
 *
 * @tparam T Destination arithmetic type whose range is tested.
 * @tparam U Source arithmetic type.
 * @param value Source value.
 * @return true if the value is within the range of T.
 */
template<typename T, typename U>
    requires non_bool_arithmetic<T> && arithmetic<U>
[[nodiscard]] constexpr auto in_range(U value) noexcept -> bool
{
    if constexpr (std::floating_point<std::remove_cvref_t<U>>) {
        if (std::isnan(value)) {
            return false;
        }
    }

    const long double numeric_value = detail::to_in_range_long_double(value);
    const long double lowest =
        detail::to_in_range_long_double(std::numeric_limits<T>::lowest());
    const long double highest =
        detail::to_in_range_long_double(std::numeric_limits<T>::max());

    return lowest <= numeric_value && numeric_value <= highest;
}

/**
 * @brief Checks whether a successful result value is within the numeric range of T.
 *
 * If @p value is an error, this function returns `false`.
 *
 * `T = bool` is intentionally rejected.
 *
 * @tparam T Destination arithmetic type whose range is tested.
 * @tparam U Source arithmetic type.
 * @param value Source result.
 * @return true if the successful value is within the range of T.
 */
template<typename T, typename U>
    requires non_bool_arithmetic<T> && arithmetic<U>
[[nodiscard]] constexpr auto in_range(const result<U>& value) noexcept -> bool
{
    return value.has_value() && in_range<T>(*value);
}

} // namespace xer

#endif /* XER_BITS_IN_RANGE_H_INCLUDED_ */

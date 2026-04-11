/**
 * @file xer/bits/make_unsigned_ex.h
 * @brief Internal unsigned-integer mapping utilities.
 */

#pragma once

#ifndef XER_BITS_MAKE_UNSIGNED_EX_H_INCLUDED_
#define XER_BITS_MAKE_UNSIGNED_EX_H_INCLUDED_

#include <concepts>
#include <type_traits>

namespace xer::detail {

/**
 * @brief Maps an integer type to its unsigned counterpart.
 *
 * `std::make_unsigned_t` is not reliably available for `__int128`
 * on all target standard library implementations, so XER provides
 * its own trait for that case.
 *
 * @tparam T Source integer type.
 */
template<typename T>
struct make_unsigned_ex {
    using type = std::make_unsigned_t<T>;
};

#if defined(__SIZEOF_INT128__)
template<>
struct make_unsigned_ex<__int128> {
    using type = unsigned __int128;
};

template<>
struct make_unsigned_ex<unsigned __int128> {
    using type = unsigned __int128;
};
#endif

/**
 * @brief Convenience alias for make_unsigned_ex.
 *
 * @tparam T Source integer type.
 */
template<typename T>
using make_unsigned_ex_t = typename make_unsigned_ex<T>::type;

/**
 * @brief Returns whether T is treated as a signed integer by XER internals.
 *
 * This intentionally includes `__int128` even on implementations where
 * the standard library concepts do not classify it as `std::signed_integral`.
 *
 * @tparam T Type to test.
 */
template<typename T>
inline constexpr bool is_signed_integer_ex_v =
    std::same_as<std::remove_cv_t<T>, signed char> ||
    std::same_as<std::remove_cv_t<T>, short> ||
    std::same_as<std::remove_cv_t<T>, int> ||
    std::same_as<std::remove_cv_t<T>, long> ||
    std::same_as<std::remove_cv_t<T>, long long>
#if defined(__SIZEOF_INT128__)
    || std::same_as<std::remove_cv_t<T>, __int128>
#endif
    ;

/**
 * @brief Returns whether T is treated as an unsigned integer by XER internals.
 *
 * This intentionally includes `unsigned __int128` even on implementations where
 * the standard library concepts do not classify it as `std::unsigned_integral`.
 *
 * @tparam T Type to test.
 */
template<typename T>
inline constexpr bool is_unsigned_integer_ex_v =
    std::same_as<std::remove_cv_t<T>, unsigned char> ||
    std::same_as<std::remove_cv_t<T>, unsigned short> ||
    std::same_as<std::remove_cv_t<T>, unsigned int> ||
    std::same_as<std::remove_cv_t<T>, unsigned long> ||
    std::same_as<std::remove_cv_t<T>, unsigned long long>
#if defined(__SIZEOF_INT128__)
    || std::same_as<std::remove_cv_t<T>, unsigned __int128>
#endif
    ;

} // namespace xer::detail

#endif /* XER_BITS_MAKE_UNSIGNED_EX_H_INCLUDED_ */

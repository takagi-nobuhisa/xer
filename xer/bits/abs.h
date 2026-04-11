/**
 * @file xer/bits/abs.h
 * @brief Internal absolute value function implementations.
 */

#pragma once

#ifndef XER_BITS_ABS_H_INCLUDED_
#define XER_BITS_ABS_H_INCLUDED_

#include <expected>
#include <limits>
#include <type_traits>
#include <utility>

#include <xer/bits/common.h>
#include <xer/bits/error.h>
#include <xer/bits/make_unsigned_ex.h>

namespace xer {
namespace detail {

/**
 * @brief Converts an error_t value to error<void>.
 *
 * @param value Source error code.
 * @return Converted error<void> object.
 */
[[nodiscard]] constexpr error<void> to_error_void(error_t value) noexcept
{
    return make_error(value);
}

/**
 * @brief Returns an error<void> object unchanged.
 *
 * @param value Source error object.
 * @return Same error object.
 */
[[nodiscard]] constexpr error<void> to_error_void(const error<void>& value) noexcept
{
    return value;
}

/**
 * @brief Converts an error with detail to error<void>.
 *
 * @tparam Detail Detail type.
 * @param value Source error object.
 * @return Converted error<void> object preserving code and location.
 */
template<class Detail>
[[nodiscard]] constexpr error<void> to_error_void(const error<Detail>& value) noexcept
{
    return error<void>(value.code, value.location);
}

/**
 * @brief Converts an arbitrary error object to error<void>.
 *
 * @tparam E Source error type.
 * @param value Source error object.
 * @return Converted error<void> object.
 */
template<class E>
[[nodiscard]] constexpr error<void> to_error_void(const E&) noexcept
{
    return make_error(error_t::runtime_error);
}

/**
 * @brief Computes the unsigned absolute value of a signed integer.
 *
 * @tparam SignedInteger Signed integer type.
 * @param value Source value.
 * @return Unsigned absolute value.
 */
template<class SignedInteger>
[[nodiscard]] constexpr make_unsigned_ex_t<SignedInteger> signed_uabs(
    SignedInteger value) noexcept
{
    using unsigned_type = make_unsigned_ex_t<SignedInteger>;

    if (value >= 0) {
        return static_cast<unsigned_type>(value);
    }

    return static_cast<unsigned_type>(0) - static_cast<unsigned_type>(value);
}

/**
 * @brief Computes the signed absolute value of a signed integer.
 *
 * @tparam SignedInteger Signed integer type.
 * @param value Source value.
 * @return Absolute value on success, or overflow_error on failure.
 */
template<class SignedInteger>
[[nodiscard]] constexpr std::expected<SignedInteger, error<void>> signed_abs(
    SignedInteger value) noexcept
{
    if (value == std::numeric_limits<SignedInteger>::min()) {
        return std::unexpected(make_error(error_t::overflow_error));
    }

    if (value < 0) {
        return static_cast<SignedInteger>(-value);
    }

    return value;
}

/**
 * @brief Applies abs() to an expected value and normalizes the error type.
 *
 * @tparam T Integer type.
 * @tparam E Source error type.
 * @param value Source expected object.
 * @return Normalized expected absolute value.
 */
template<class T, class E>
[[nodiscard]] constexpr std::expected<T, error<void>> abs_expected(
    const std::expected<T, E>& value) noexcept
{
    if (!value) {
        return std::unexpected(to_error_void(value.error()));
    }

    return signed_abs(*value);
}

/**
 * @brief Applies uabs() to an expected value and normalizes the error type.
 *
 * @tparam T Integer type.
 * @tparam U Result unsigned type.
 * @tparam E Source error type.
 * @param value Source expected object.
 * @return Normalized expected unsigned absolute value.
 */
template<class T, class U, class E>
[[nodiscard]] constexpr std::expected<U, error<void>> uabs_expected(
    const std::expected<T, E>& value) noexcept
{
    if (!value) {
        return std::unexpected(to_error_void(value.error()));
    }

    if constexpr (std::is_signed_v<T>) {
        return signed_uabs(*value);
    } else {
        return static_cast<U>(*value);
    }
}

} // namespace detail

/**
 * @brief Returns the absolute value of an int.
 *
 * @param value Source value.
 * @return Absolute value on success, or overflow_error on failure.
 */
[[nodiscard]] constexpr std::expected<int, error<void>> abs(int value) noexcept
{
    return detail::signed_abs(value);
}

/**
 * @brief Returns the absolute value of a long.
 *
 * @param value Source value.
 * @return Absolute value on success, or overflow_error on failure.
 */
[[nodiscard]] constexpr std::expected<long, error<void>> abs(long value) noexcept
{
    return detail::signed_abs(value);
}

/**
 * @brief Returns the absolute value of a long long.
 *
 * @param value Source value.
 * @return Absolute value on success, or overflow_error on failure.
 */
[[nodiscard]] constexpr std::expected<long long, error<void>> abs(long long value) noexcept
{
    return detail::signed_abs(value);
}

#if defined(__SIZEOF_INT128__)
/**
 * @brief Returns the absolute value of a signed __int128.
 *
 * @param value Source value.
 * @return Absolute value on success, or overflow_error on failure.
 */
[[nodiscard]] constexpr std::expected<__int128, error<void>> abs(__int128 value) noexcept
{
    return detail::signed_abs(value);
}
#endif

/**
 * @brief Returns the absolute value of an expected int.
 *
 * @tparam E Source error type.
 * @param value Source expected object.
 * @return Absolute value on success, or normalized error on failure.
 */
template<class E>
[[nodiscard]] constexpr std::expected<int, error<void>> abs(
    const std::expected<int, E>& value) noexcept
{
    return detail::abs_expected(value);
}

/**
 * @brief Returns the absolute value of an expected long.
 *
 * @tparam E Source error type.
 * @param value Source expected object.
 * @return Absolute value on success, or normalized error on failure.
 */
template<class E>
[[nodiscard]] constexpr std::expected<long, error<void>> abs(
    const std::expected<long, E>& value) noexcept
{
    return detail::abs_expected(value);
}

/**
 * @brief Returns the absolute value of an expected long long.
 *
 * @tparam E Source error type.
 * @param value Source expected object.
 * @return Absolute value on success, or normalized error on failure.
 */
template<class E>
[[nodiscard]] constexpr std::expected<long long, error<void>> abs(
    const std::expected<long long, E>& value) noexcept
{
    return detail::abs_expected(value);
}

#if defined(__SIZEOF_INT128__)
/**
 * @brief Returns the absolute value of an expected signed __int128.
 *
 * @tparam E Source error type.
 * @param value Source expected object.
 * @return Absolute value on success, or normalized error on failure.
 */
template<class E>
[[nodiscard]] constexpr std::expected<__int128, error<void>> abs(
    const std::expected<__int128, E>& value) noexcept
{
    return detail::abs_expected(value);
}
#endif

/**
 * @brief Returns the unsigned absolute value of an int.
 *
 * @param value Source value.
 * @return Unsigned absolute value.
 */
[[nodiscard]] constexpr std::expected<unsigned int, error<void>> uabs(int value) noexcept
{
    return detail::signed_uabs(value);
}

/**
 * @brief Returns the unsigned absolute value of an unsigned int.
 *
 * @param value Source value.
 * @return Same value.
 */
[[nodiscard]] constexpr std::expected<unsigned int, error<void>> uabs(
    unsigned int value) noexcept
{
    return value;
}

/**
 * @brief Returns the unsigned absolute value of a long.
 *
 * @param value Source value.
 * @return Unsigned absolute value.
 */
[[nodiscard]] constexpr std::expected<unsigned long, error<void>> uabs(long value) noexcept
{
    return detail::signed_uabs(value);
}

/**
 * @brief Returns the unsigned absolute value of an unsigned long.
 *
 * @param value Source value.
 * @return Same value.
 */
[[nodiscard]] constexpr std::expected<unsigned long, error<void>> uabs(
    unsigned long value) noexcept
{
    return value;
}

/**
 * @brief Returns the unsigned absolute value of a long long.
 *
 * @param value Source value.
 * @return Unsigned absolute value.
 */
[[nodiscard]] constexpr std::expected<unsigned long long, error<void>> uabs(
    long long value) noexcept
{
    return detail::signed_uabs(value);
}

/**
 * @brief Returns the unsigned absolute value of an unsigned long long.
 *
 * @param value Source value.
 * @return Same value.
 */
[[nodiscard]] constexpr std::expected<unsigned long long, error<void>> uabs(
    unsigned long long value) noexcept
{
    return value;
}

#if defined(__SIZEOF_INT128__)
/**
 * @brief Returns the unsigned absolute value of a signed __int128.
 *
 * @param value Source value.
 * @return Unsigned absolute value.
 */
[[nodiscard]] constexpr std::expected<unsigned __int128, error<void>> uabs(
    __int128 value) noexcept
{
    return detail::signed_uabs(value);
}

/**
 * @brief Returns the unsigned absolute value of an unsigned __int128.
 *
 * @param value Source value.
 * @return Same value.
 */
[[nodiscard]] constexpr std::expected<unsigned __int128, error<void>> uabs(
    unsigned __int128 value) noexcept
{
    return value;
}
#endif

/**
 * @brief Returns the unsigned absolute value of an expected int.
 *
 * @tparam E Source error type.
 * @param value Source expected object.
 * @return Unsigned absolute value on success, or normalized error on failure.
 */
template<class E>
[[nodiscard]] constexpr std::expected<unsigned int, error<void>> uabs(
    const std::expected<int, E>& value) noexcept
{
    return detail::uabs_expected<int, unsigned int>(value);
}

/**
 * @brief Returns the unsigned absolute value of an expected unsigned int.
 *
 * @tparam E Source error type.
 * @param value Source expected object.
 * @return Same value on success, or normalized error on failure.
 */
template<class E>
[[nodiscard]] constexpr std::expected<unsigned int, error<void>> uabs(
    const std::expected<unsigned int, E>& value) noexcept
{
    return detail::uabs_expected<unsigned int, unsigned int>(value);
}

/**
 * @brief Returns the unsigned absolute value of an expected long.
 *
 * @tparam E Source error type.
 * @param value Source expected object.
 * @return Unsigned absolute value on success, or normalized error on failure.
 */
template<class E>
[[nodiscard]] constexpr std::expected<unsigned long, error<void>> uabs(
    const std::expected<long, E>& value) noexcept
{
    return detail::uabs_expected<long, unsigned long>(value);
}

/**
 * @brief Returns the unsigned absolute value of an expected unsigned long.
 *
 * @tparam E Source error type.
 * @param value Source expected object.
 * @return Same value on success, or normalized error on failure.
 */
template<class E>
[[nodiscard]] constexpr std::expected<unsigned long, error<void>> uabs(
    const std::expected<unsigned long, E>& value) noexcept
{
    return detail::uabs_expected<unsigned long, unsigned long>(value);
}

/**
 * @brief Returns the unsigned absolute value of an expected long long.
 *
 * @tparam E Source error type.
 * @param value Source expected object.
 * @return Unsigned absolute value on success, or normalized error on failure.
 */
template<class E>
[[nodiscard]] constexpr std::expected<unsigned long long, error<void>> uabs(
    const std::expected<long long, E>& value) noexcept
{
    return detail::uabs_expected<long long, unsigned long long>(value);
}

/**
 * @brief Returns the unsigned absolute value of an expected unsigned long long.
 *
 * @tparam E Source error type.
 * @param value Source expected object.
 * @return Same value on success, or normalized error on failure.
 */
template<class E>
[[nodiscard]] constexpr std::expected<unsigned long long, error<void>> uabs(
    const std::expected<unsigned long long, E>& value) noexcept
{
    return detail::uabs_expected<unsigned long long, unsigned long long>(value);
}

#if defined(__SIZEOF_INT128__)
/**
 * @brief Returns the unsigned absolute value of an expected signed __int128.
 *
 * @tparam E Source error type.
 * @param value Source expected object.
 * @return Unsigned absolute value on success, or normalized error on failure.
 */
template<class E>
[[nodiscard]] constexpr std::expected<unsigned __int128, error<void>> uabs(
    const std::expected<__int128, E>& value) noexcept
{
    return detail::uabs_expected<__int128, unsigned __int128>(value);
}

/**
 * @brief Returns the unsigned absolute value of an expected unsigned __int128.
 *
 * @tparam E Source error type.
 * @param value Source expected object.
 * @return Same value on success, or normalized error on failure.
 */
template<class E>
[[nodiscard]] constexpr std::expected<unsigned __int128, error<void>> uabs(
    const std::expected<unsigned __int128, E>& value) noexcept
{
    return detail::uabs_expected<unsigned __int128, unsigned __int128>(value);
}
#endif

} // namespace xer

#endif /* XER_BITS_ABS_H_INCLUDED_ */

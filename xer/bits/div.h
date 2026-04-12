/**
 * @file xer/bits/div.h
 * @brief Internal integer division function implementations.
 */

#pragma once

#ifndef XER_BITS_DIV_H_INCLUDED_
#define XER_BITS_DIV_H_INCLUDED_

#include <concepts>
#include <expected>
#include <limits>
#include <type_traits>
#include <utility>

#include <xer/bits/common.h>
#include <xer/bits/make_unsigned_ex.h>
#include <xer/error.h>
#include <xer/stdint.h>

namespace xer {

/**
 * @brief Quotient and remainder pair.
 *
 * @tparam T Integer value type.
 */
template<typename T>
struct rem_quot {
    T rem;
    T quot;
};

using div_t = rem_quot<int>;
using ldiv_t = rem_quot<long>;
using lldiv_t = rem_quot<long long>;

using i8div_t = rem_quot<xer::int8_t>;
using i16div_t = rem_quot<xer::int16_t>;
using i32div_t = rem_quot<xer::int32_t>;
using i64div_t = rem_quot<xer::int64_t>;

using u8div_t = rem_quot<xer::uint8_t>;
using u16div_t = rem_quot<xer::uint16_t>;
using u32div_t = rem_quot<xer::uint32_t>;
using u64div_t = rem_quot<xer::uint64_t>;

} // namespace xer

namespace xer::detail {

/**
 * @brief Canonical integer argument types accepted directly by public overloads.
 *
 * @tparam T Type to test.
 */
template<typename T>
inline constexpr bool is_canonical_div_integer_v =
    std::same_as<T, int> ||
    std::same_as<T, unsigned int> ||
    std::same_as<T, long> ||
    std::same_as<T, unsigned long> ||
    std::same_as<T, long long> ||
    std::same_as<T, unsigned long long> ||
    std::same_as<T, xer::int128_t> ||
    std::same_as<T, xer::uint128_t>;

/**
 * @brief Integer types forwarded to canonical public overloads.
 *
 * `bool` is intentionally excluded.
 *
 * @tparam T Type to test.
 */
template<typename T>
inline constexpr bool is_forwardable_div_integer_v =
    std::integral<std::remove_cvref_t<T>> &&
    !std::same_as<std::remove_cvref_t<T>, bool> &&
    !is_canonical_div_integer_v<std::remove_cvref_t<T>>;

/**
 * @brief Canonical forwarding target for a non-canonical integer type.
 *
 * Signed narrow integers are forwarded to `int`.
 * Unsigned narrow integers are forwarded to `unsigned int`.
 *
 * @tparam T Source integer type.
 */
template<typename T>
using forwarded_div_integer_t = std::conditional_t<
    std::is_signed_v<std::remove_cvref_t<T>>,
    int,
    unsigned int>;

/**
 * @brief Common signed integer type used for mixed signed arithmetic.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 */
template<typename A, typename B>
using signed_div_result_t = std::conditional_t<
    std::same_as<A, xer::int128_t> || std::same_as<B, xer::int128_t> ||
        std::same_as<A, xer::uint128_t> || std::same_as<B, xer::uint128_t>,
    xer::int128_t,
    std::conditional_t<
        std::same_as<A, long long> || std::same_as<B, long long> ||
            std::same_as<A, unsigned long long> || std::same_as<B, unsigned long long>,
        long long,
        std::conditional_t<
            std::same_as<A, long> || std::same_as<B, long> ||
                std::same_as<A, unsigned long> || std::same_as<B, unsigned long>,
            long,
            int>>>;

/**
 * @brief Common unsigned integer type used for mixed unsigned arithmetic.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 */
template<typename A, typename B>
using unsigned_div_result_t = make_unsigned_ex_t<signed_div_result_t<A, B>>;

/**
 * @brief Creates a divide-by-zero error.
 *
 * @return Divide-by-zero error.
 */
[[nodiscard]] constexpr error<void> make_divide_by_zero_error() noexcept
{
    return make_error(error_t::divide_by_zero);
}

/**
 * @brief Creates an out-of-range error.
 *
 * @return Out-of-range error.
 */
[[nodiscard]] constexpr error<void> make_div_out_of_range_error() noexcept
{
    return make_error(error_t::out_of_range);
}

/**
 * @brief Creates a domain error.
 *
 * @return Domain error.
 */
[[nodiscard]] constexpr error<void> make_div_domain_error() noexcept
{
    return make_error(error_t::dom);
}

/**
 * @brief Converts an integer value to sign and magnitude.
 *
 * @tparam T Integer type.
 * @param value Source value.
 * @return Sign-magnitude representation.
 */
template<typename T>
    requires(is_canonical_div_integer_v<T>)
[[nodiscard]] constexpr auto to_sign_magnitude(T value) noexcept -> std::pair<bool, xer::uint128_t> {
    if constexpr (is_unsigned_integer_ex_v<T>) {
        return {false, static_cast<xer::uint128_t>(value)};
    } else {
        if (value >= 0) {
            return {false, static_cast<xer::uint128_t>(value)};
        }

        return {
            true,
            static_cast<xer::uint128_t>(-(value + 1)) + 1,
        };
    }
}

/**
 * @brief Converts sign and magnitude to a signed result type.
 *
 * @tparam T Signed integer result type.
 * @param negative Sign bit.
 * @param magnitude Magnitude.
 * @return Converted value, or an error if out of range.
 */
template<typename T>
    requires(is_signed_integer_ex_v<T>)
[[nodiscard]] constexpr auto from_sign_magnitude_signed(
    bool negative,
    xer::uint128_t magnitude) noexcept -> result<T> {
    constexpr xer::uint128_t positive_limit =
        static_cast<xer::uint128_t>(std::numeric_limits<T>::max());
    constexpr xer::uint128_t negative_limit = positive_limit + 1;

    if (!negative) {
        if (magnitude > positive_limit) {
            return std::unexpected(make_div_out_of_range_error());
        }

        return static_cast<T>(magnitude);
    }

    if (magnitude > negative_limit) {
        return std::unexpected(make_div_out_of_range_error());
    }

    if (magnitude == negative_limit) {
        return std::numeric_limits<T>::min();
    }

    return -static_cast<T>(magnitude);
}

/**
 * @brief Converts a non-negative magnitude to an unsigned result type.
 *
 * @tparam T Unsigned integer result type.
 * @param magnitude Magnitude.
 * @return Converted value, or an error if out of range.
 */
template<typename T>
    requires(is_unsigned_integer_ex_v<T>)
[[nodiscard]] constexpr auto from_magnitude_unsigned(
    xer::uint128_t magnitude) noexcept -> result<T> {
    constexpr xer::uint128_t positive_limit =
        static_cast<xer::uint128_t>(std::numeric_limits<T>::max());

    if (magnitude > positive_limit) {
        return std::unexpected(make_div_out_of_range_error());
    }

    return static_cast<T>(magnitude);
}

/**
 * @brief Performs truncating integer division on magnitudes.
 *
 * @param lhs_mag Dividend magnitude.
 * @param rhs_mag Divisor magnitude.
 * @return Quotient and remainder magnitudes.
 */
[[nodiscard]] constexpr auto divide_magnitude(
    xer::uint128_t lhs_mag,
    xer::uint128_t rhs_mag) noexcept -> std::pair<xer::uint128_t, xer::uint128_t> {
    return {lhs_mag / rhs_mag, lhs_mag % rhs_mag};
}

/**
 * @brief Implements signed division for canonical integer operand types.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Dividend.
 * @param rhs Divisor.
 * @return Quotient and remainder.
 */
template<typename A, typename B>
    requires(is_canonical_div_integer_v<A> && is_canonical_div_integer_v<B>)
[[nodiscard]] constexpr auto div_canonical(
    A lhs,
    B rhs) noexcept
    -> result<xer::rem_quot<signed_div_result_t<A, B>>>
{
    using result_t = signed_div_result_t<A, B>;

    const auto [lhs_negative, lhs_mag] = to_sign_magnitude(lhs);
    const auto [rhs_negative, rhs_mag] = to_sign_magnitude(rhs);

    if (rhs_mag == 0) {
        return std::unexpected(make_divide_by_zero_error());
    }

    const auto [quot_mag, rem_mag] = divide_magnitude(lhs_mag, rhs_mag);

    const auto quot = from_sign_magnitude_signed<result_t>(
        lhs_negative != rhs_negative && quot_mag != 0,
        quot_mag);
    if (!quot) {
        return std::unexpected(quot.error());
    }

    const auto rem = from_sign_magnitude_signed<result_t>(
        lhs_negative && rem_mag != 0,
        rem_mag);
    if (!rem) {
        return std::unexpected(rem.error());
    }

    return xer::rem_quot<result_t>{
        .rem = *rem,
        .quot = *quot,
    };
}

/**
 * @brief Implements unsigned-style division for canonical integer operand types.
 *
 * The operation succeeds only when the dividend and divisor have the same sign.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Dividend.
 * @param rhs Divisor.
 * @return Quotient and remainder.
 */
template<typename A, typename B>
    requires(is_canonical_div_integer_v<A> && is_canonical_div_integer_v<B>)
[[nodiscard]] constexpr auto udiv_canonical(
    A lhs,
    B rhs) noexcept
    -> result<xer::rem_quot<unsigned_div_result_t<A, B>>>
{
    using result_t = unsigned_div_result_t<A, B>;

    const auto [lhs_negative, lhs_mag] = to_sign_magnitude(lhs);
    const auto [rhs_negative, rhs_mag] = to_sign_magnitude(rhs);

    if (rhs_mag == 0) {
        return std::unexpected(make_divide_by_zero_error());
    }

    if (lhs_negative != rhs_negative) {
        return std::unexpected(make_div_domain_error());
    }

    const auto [quot_mag, rem_mag] = divide_magnitude(lhs_mag, rhs_mag);

    const auto quot = from_magnitude_unsigned<result_t>(quot_mag);
    if (!quot) {
        return std::unexpected(quot.error());
    }

    const auto rem = from_magnitude_unsigned<result_t>(rem_mag);
    if (!rem) {
        return std::unexpected(rem.error());
    }

    return xer::rem_quot<result_t>{
        .rem = *rem,
        .quot = *quot,
    };
}

} // namespace xer::detail

namespace xer {

#define XER_DETAIL_DECLARE_DIV_PAIR(A, B)                                            \
    [[nodiscard]] constexpr auto div(A lhs, B rhs) noexcept                          \
        -> result<rem_quot<detail::signed_div_result_t<A, B>>>   \
    {                                                                                \
        return detail::div_canonical(lhs, rhs);                                      \
    }                                                                                \
                                                                                     \
    [[nodiscard]] constexpr auto udiv(A lhs, B rhs) noexcept                         \
        -> result<rem_quot<detail::unsigned_div_result_t<A, B>>> \
    {                                                                                \
        return detail::udiv_canonical(lhs, rhs);                                     \
    }

XER_DETAIL_DECLARE_DIV_PAIR(int, int)
XER_DETAIL_DECLARE_DIV_PAIR(int, unsigned int)
XER_DETAIL_DECLARE_DIV_PAIR(int, long)
XER_DETAIL_DECLARE_DIV_PAIR(int, unsigned long)
XER_DETAIL_DECLARE_DIV_PAIR(int, long long)
XER_DETAIL_DECLARE_DIV_PAIR(int, unsigned long long)
XER_DETAIL_DECLARE_DIV_PAIR(int, xer::int128_t)
XER_DETAIL_DECLARE_DIV_PAIR(int, xer::uint128_t)

XER_DETAIL_DECLARE_DIV_PAIR(unsigned int, int)
XER_DETAIL_DECLARE_DIV_PAIR(unsigned int, unsigned int)
XER_DETAIL_DECLARE_DIV_PAIR(unsigned int, long)
XER_DETAIL_DECLARE_DIV_PAIR(unsigned int, unsigned long)
XER_DETAIL_DECLARE_DIV_PAIR(unsigned int, long long)
XER_DETAIL_DECLARE_DIV_PAIR(unsigned int, unsigned long long)
XER_DETAIL_DECLARE_DIV_PAIR(unsigned int, xer::int128_t)
XER_DETAIL_DECLARE_DIV_PAIR(unsigned int, xer::uint128_t)

XER_DETAIL_DECLARE_DIV_PAIR(long, int)
XER_DETAIL_DECLARE_DIV_PAIR(long, unsigned int)
XER_DETAIL_DECLARE_DIV_PAIR(long, long)
XER_DETAIL_DECLARE_DIV_PAIR(long, unsigned long)
XER_DETAIL_DECLARE_DIV_PAIR(long, long long)
XER_DETAIL_DECLARE_DIV_PAIR(long, unsigned long long)
XER_DETAIL_DECLARE_DIV_PAIR(long, xer::int128_t)
XER_DETAIL_DECLARE_DIV_PAIR(long, xer::uint128_t)

XER_DETAIL_DECLARE_DIV_PAIR(unsigned long, int)
XER_DETAIL_DECLARE_DIV_PAIR(unsigned long, unsigned int)
XER_DETAIL_DECLARE_DIV_PAIR(unsigned long, long)
XER_DETAIL_DECLARE_DIV_PAIR(unsigned long, unsigned long)
XER_DETAIL_DECLARE_DIV_PAIR(unsigned long, long long)
XER_DETAIL_DECLARE_DIV_PAIR(unsigned long, unsigned long long)
XER_DETAIL_DECLARE_DIV_PAIR(unsigned long, xer::int128_t)
XER_DETAIL_DECLARE_DIV_PAIR(unsigned long, xer::uint128_t)

XER_DETAIL_DECLARE_DIV_PAIR(long long, int)
XER_DETAIL_DECLARE_DIV_PAIR(long long, unsigned int)
XER_DETAIL_DECLARE_DIV_PAIR(long long, long)
XER_DETAIL_DECLARE_DIV_PAIR(long long, unsigned long)
XER_DETAIL_DECLARE_DIV_PAIR(long long, long long)
XER_DETAIL_DECLARE_DIV_PAIR(long long, unsigned long long)
XER_DETAIL_DECLARE_DIV_PAIR(long long, xer::int128_t)
XER_DETAIL_DECLARE_DIV_PAIR(long long, xer::uint128_t)

XER_DETAIL_DECLARE_DIV_PAIR(unsigned long long, int)
XER_DETAIL_DECLARE_DIV_PAIR(unsigned long long, unsigned int)
XER_DETAIL_DECLARE_DIV_PAIR(unsigned long long, long)
XER_DETAIL_DECLARE_DIV_PAIR(unsigned long long, unsigned long)
XER_DETAIL_DECLARE_DIV_PAIR(unsigned long long, long long)
XER_DETAIL_DECLARE_DIV_PAIR(unsigned long long, unsigned long long)
XER_DETAIL_DECLARE_DIV_PAIR(unsigned long long, xer::int128_t)
XER_DETAIL_DECLARE_DIV_PAIR(unsigned long long, xer::uint128_t)

XER_DETAIL_DECLARE_DIV_PAIR(xer::int128_t, int)
XER_DETAIL_DECLARE_DIV_PAIR(xer::int128_t, unsigned int)
XER_DETAIL_DECLARE_DIV_PAIR(xer::int128_t, long)
XER_DETAIL_DECLARE_DIV_PAIR(xer::int128_t, unsigned long)
XER_DETAIL_DECLARE_DIV_PAIR(xer::int128_t, long long)
XER_DETAIL_DECLARE_DIV_PAIR(xer::int128_t, unsigned long long)
XER_DETAIL_DECLARE_DIV_PAIR(xer::int128_t, xer::int128_t)
XER_DETAIL_DECLARE_DIV_PAIR(xer::int128_t, xer::uint128_t)

XER_DETAIL_DECLARE_DIV_PAIR(xer::uint128_t, int)
XER_DETAIL_DECLARE_DIV_PAIR(xer::uint128_t, unsigned int)
XER_DETAIL_DECLARE_DIV_PAIR(xer::uint128_t, long)
XER_DETAIL_DECLARE_DIV_PAIR(xer::uint128_t, unsigned long)
XER_DETAIL_DECLARE_DIV_PAIR(xer::uint128_t, long long)
XER_DETAIL_DECLARE_DIV_PAIR(xer::uint128_t, unsigned long long)
XER_DETAIL_DECLARE_DIV_PAIR(xer::uint128_t, xer::int128_t)
XER_DETAIL_DECLARE_DIV_PAIR(xer::uint128_t, xer::uint128_t)

#undef XER_DETAIL_DECLARE_DIV_PAIR

template<typename A, typename B>
    requires(detail::is_forwardable_div_integer_v<A> ||
             detail::is_forwardable_div_integer_v<B>)
[[nodiscard]] constexpr auto div(A lhs, B rhs) noexcept
    -> decltype(div(
        static_cast<detail::forwarded_div_integer_t<A>>(lhs),
        static_cast<detail::forwarded_div_integer_t<B>>(rhs)))
{
    return div(
        static_cast<detail::forwarded_div_integer_t<A>>(lhs),
        static_cast<detail::forwarded_div_integer_t<B>>(rhs));
}

template<typename A, typename B>
    requires(detail::is_forwardable_div_integer_v<A> ||
             detail::is_forwardable_div_integer_v<B>)
[[nodiscard]] constexpr auto udiv(A lhs, B rhs) noexcept
    -> decltype(udiv(
        static_cast<detail::forwarded_div_integer_t<A>>(lhs),
        static_cast<detail::forwarded_div_integer_t<B>>(rhs)))
{
    return udiv(
        static_cast<detail::forwarded_div_integer_t<A>>(lhs),
        static_cast<detail::forwarded_div_integer_t<B>>(rhs));
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto div(
    const result<T>& lhs,
    U rhs) noexcept -> decltype(div(std::declval<T>(), rhs))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    return div(*lhs, rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto div(
    T lhs,
    const result<U>& rhs) noexcept
    -> decltype(div(lhs, std::declval<U>()))
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return div(lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto div(
    const result<T>& lhs,
    const result<U>& rhs) noexcept
    -> decltype(div(std::declval<T>(), std::declval<U>()))
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
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto udiv(
    const result<T>& lhs,
    U rhs) noexcept -> decltype(udiv(std::declval<T>(), rhs))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    return udiv(*lhs, rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto udiv(
    T lhs,
    const result<U>& rhs) noexcept
    -> decltype(udiv(lhs, std::declval<U>()))
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return udiv(lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto udiv(
    const result<T>& lhs,
    const result<U>& rhs) noexcept
    -> decltype(udiv(std::declval<T>(), std::declval<U>()))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return udiv(*lhs, *rhs);
}

} // namespace xer

#endif

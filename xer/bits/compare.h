/**
 * @file xer/bits/compare.h
 * @brief Internal integer comparison function implementations.
 */

#pragma once

#ifndef XER_BITS_COMPARE_H_INCLUDED_
#define XER_BITS_COMPARE_H_INCLUDED_

#include <concepts>
#include <expected>
#include <type_traits>
#include <utility>

#include <xer/bits/common.h>
#include <xer/error.h>
#include <xer/stdint.h>

namespace xer::detail {

/**
 * @brief Canonical integer argument types accepted directly by public overloads.
 *
 * @tparam T Type to test.
 */
template<typename T>
inline constexpr bool is_canonical_compare_integer_v =
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
inline constexpr bool is_forwardable_compare_integer_v =
    std::integral<std::remove_cvref_t<T>> &&
    !std::same_as<std::remove_cvref_t<T>, bool> &&
    !is_canonical_compare_integer_v<std::remove_cvref_t<T>>;

/**
 * @brief Canonical forwarding target for a non-canonical integer type.
 *
 * Signed narrow integers are forwarded to `int`.
 * Unsigned narrow integers are forwarded to `unsigned int`.
 *
 * @tparam T Source integer type.
 */
template<typename T>
using forwarded_compare_integer_t = std::conditional_t<
    std::signed_integral<std::remove_cvref_t<T>>,
    int,
    unsigned int>;

/**
 * @brief Mathematical integer represented by sign and magnitude.
 */
struct compare_integer_value {
    bool negative;
    xer::uint128_t magnitude;
};

/**
 * @brief Converts an integer operand to sign-magnitude form.
 *
 * @tparam T Canonical integer type.
 * @param value Integer operand.
 * @return Sign-magnitude representation.
 */
template<typename T>
    requires(is_canonical_compare_integer_v<T>)
[[nodiscard]] constexpr compare_integer_value to_compare_integer_value(T value) noexcept
{
    if constexpr (std::unsigned_integral<T>) {
        return compare_integer_value{
            false,
            static_cast<xer::uint128_t>(value),
        };
    } else {
        if (value >= 0) {
            return compare_integer_value{
                false,
                static_cast<xer::uint128_t>(value),
            };
        }

        return compare_integer_value{
            true,
            static_cast<xer::uint128_t>(-(value + 1)) + 1,
        };
    }
}

/**
 * @brief Compares two sign-magnitude integers.
 *
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Negative if `lhs < rhs`, zero if equal, positive if `lhs > rhs`.
 */
[[nodiscard]] constexpr int compare_integer_value_impl(
    compare_integer_value lhs,
    compare_integer_value rhs) noexcept
{
    if (lhs.negative != rhs.negative) {
        return lhs.negative ? -1 : 1;
    }

    if (lhs.magnitude == rhs.magnitude) {
        return 0;
    }

    if (!lhs.negative) {
        return lhs.magnitude < rhs.magnitude ? -1 : 1;
    }

    return lhs.magnitude < rhs.magnitude ? 1 : -1;
}

/**
 * @brief Implements integer comparison for canonical operand types.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Comparison result.
 */
template<typename A, typename B>
    requires(is_canonical_compare_integer_v<A> && is_canonical_compare_integer_v<B>)
[[nodiscard]] constexpr int compare_canonical(A lhs, B rhs) noexcept
{
    return compare_integer_value_impl(
        to_compare_integer_value(lhs),
        to_compare_integer_value(rhs));
}

} // namespace xer::detail

namespace xer {

#define XER_DETAIL_DECLARE_COMPARE_PAIR(A, B)         \
    [[nodiscard]] constexpr bool eq(A lhs, B rhs) noexcept \
    {                                                 \
        return detail::compare_canonical(lhs, rhs) == 0; \
    }                                                 \
                                                      \
    [[nodiscard]] constexpr bool ne(A lhs, B rhs) noexcept \
    {                                                 \
        return detail::compare_canonical(lhs, rhs) != 0; \
    }                                                 \
                                                      \
    [[nodiscard]] constexpr bool lt(A lhs, B rhs) noexcept \
    {                                                 \
        return detail::compare_canonical(lhs, rhs) < 0; \
    }                                                 \
                                                      \
    [[nodiscard]] constexpr bool le(A lhs, B rhs) noexcept \
    {                                                 \
        return detail::compare_canonical(lhs, rhs) <= 0; \
    }                                                 \
                                                      \
    [[nodiscard]] constexpr bool gt(A lhs, B rhs) noexcept \
    {                                                 \
        return detail::compare_canonical(lhs, rhs) > 0; \
    }                                                 \
                                                      \
    [[nodiscard]] constexpr bool ge(A lhs, B rhs) noexcept \
    {                                                 \
        return detail::compare_canonical(lhs, rhs) >= 0; \
    }

XER_DETAIL_DECLARE_COMPARE_PAIR(int, int)
XER_DETAIL_DECLARE_COMPARE_PAIR(int, unsigned int)
XER_DETAIL_DECLARE_COMPARE_PAIR(int, long)
XER_DETAIL_DECLARE_COMPARE_PAIR(int, unsigned long)
XER_DETAIL_DECLARE_COMPARE_PAIR(int, long long)
XER_DETAIL_DECLARE_COMPARE_PAIR(int, unsigned long long)
XER_DETAIL_DECLARE_COMPARE_PAIR(int, xer::int128_t)
XER_DETAIL_DECLARE_COMPARE_PAIR(int, xer::uint128_t)

XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned int, int)
XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned int, unsigned int)
XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned int, long)
XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned int, unsigned long)
XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned int, long long)
XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned int, unsigned long long)
XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned int, xer::int128_t)
XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned int, xer::uint128_t)

XER_DETAIL_DECLARE_COMPARE_PAIR(long, int)
XER_DETAIL_DECLARE_COMPARE_PAIR(long, unsigned int)
XER_DETAIL_DECLARE_COMPARE_PAIR(long, long)
XER_DETAIL_DECLARE_COMPARE_PAIR(long, unsigned long)
XER_DETAIL_DECLARE_COMPARE_PAIR(long, long long)
XER_DETAIL_DECLARE_COMPARE_PAIR(long, unsigned long long)
XER_DETAIL_DECLARE_COMPARE_PAIR(long, xer::int128_t)
XER_DETAIL_DECLARE_COMPARE_PAIR(long, xer::uint128_t)

XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned long, int)
XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned long, unsigned int)
XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned long, long)
XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned long, unsigned long)
XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned long, long long)
XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned long, unsigned long long)
XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned long, xer::int128_t)
XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned long, xer::uint128_t)

XER_DETAIL_DECLARE_COMPARE_PAIR(long long, int)
XER_DETAIL_DECLARE_COMPARE_PAIR(long long, unsigned int)
XER_DETAIL_DECLARE_COMPARE_PAIR(long long, long)
XER_DETAIL_DECLARE_COMPARE_PAIR(long long, unsigned long)
XER_DETAIL_DECLARE_COMPARE_PAIR(long long, long long)
XER_DETAIL_DECLARE_COMPARE_PAIR(long long, unsigned long long)
XER_DETAIL_DECLARE_COMPARE_PAIR(long long, xer::int128_t)
XER_DETAIL_DECLARE_COMPARE_PAIR(long long, xer::uint128_t)

XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned long long, int)
XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned long long, unsigned int)
XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned long long, long)
XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned long long, unsigned long)
XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned long long, long long)
XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned long long, unsigned long long)
XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned long long, xer::int128_t)
XER_DETAIL_DECLARE_COMPARE_PAIR(unsigned long long, xer::uint128_t)

XER_DETAIL_DECLARE_COMPARE_PAIR(xer::int128_t, int)
XER_DETAIL_DECLARE_COMPARE_PAIR(xer::int128_t, unsigned int)
XER_DETAIL_DECLARE_COMPARE_PAIR(xer::int128_t, long)
XER_DETAIL_DECLARE_COMPARE_PAIR(xer::int128_t, unsigned long)
XER_DETAIL_DECLARE_COMPARE_PAIR(xer::int128_t, long long)
XER_DETAIL_DECLARE_COMPARE_PAIR(xer::int128_t, unsigned long long)
XER_DETAIL_DECLARE_COMPARE_PAIR(xer::int128_t, xer::int128_t)
XER_DETAIL_DECLARE_COMPARE_PAIR(xer::int128_t, xer::uint128_t)

XER_DETAIL_DECLARE_COMPARE_PAIR(xer::uint128_t, int)
XER_DETAIL_DECLARE_COMPARE_PAIR(xer::uint128_t, unsigned int)
XER_DETAIL_DECLARE_COMPARE_PAIR(xer::uint128_t, long)
XER_DETAIL_DECLARE_COMPARE_PAIR(xer::uint128_t, unsigned long)
XER_DETAIL_DECLARE_COMPARE_PAIR(xer::uint128_t, long long)
XER_DETAIL_DECLARE_COMPARE_PAIR(xer::uint128_t, unsigned long long)
XER_DETAIL_DECLARE_COMPARE_PAIR(xer::uint128_t, xer::int128_t)
XER_DETAIL_DECLARE_COMPARE_PAIR(xer::uint128_t, xer::uint128_t)

#undef XER_DETAIL_DECLARE_COMPARE_PAIR

template<typename A, typename B>
    requires(detail::is_forwardable_compare_integer_v<A> ||
             detail::is_forwardable_compare_integer_v<B>)
[[nodiscard]] constexpr auto eq(A lhs, B rhs) noexcept
    -> decltype(eq(
        static_cast<detail::forwarded_compare_integer_t<A>>(lhs),
        static_cast<detail::forwarded_compare_integer_t<B>>(rhs)))
{
    return eq(
        static_cast<detail::forwarded_compare_integer_t<A>>(lhs),
        static_cast<detail::forwarded_compare_integer_t<B>>(rhs));
}

template<typename A, typename B>
    requires(detail::is_forwardable_compare_integer_v<A> ||
             detail::is_forwardable_compare_integer_v<B>)
[[nodiscard]] constexpr auto ne(A lhs, B rhs) noexcept
    -> decltype(ne(
        static_cast<detail::forwarded_compare_integer_t<A>>(lhs),
        static_cast<detail::forwarded_compare_integer_t<B>>(rhs)))
{
    return ne(
        static_cast<detail::forwarded_compare_integer_t<A>>(lhs),
        static_cast<detail::forwarded_compare_integer_t<B>>(rhs));
}

template<typename A, typename B>
    requires(detail::is_forwardable_compare_integer_v<A> ||
             detail::is_forwardable_compare_integer_v<B>)
[[nodiscard]] constexpr auto lt(A lhs, B rhs) noexcept
    -> decltype(lt(
        static_cast<detail::forwarded_compare_integer_t<A>>(lhs),
        static_cast<detail::forwarded_compare_integer_t<B>>(rhs)))
{
    return lt(
        static_cast<detail::forwarded_compare_integer_t<A>>(lhs),
        static_cast<detail::forwarded_compare_integer_t<B>>(rhs));
}

template<typename A, typename B>
    requires(detail::is_forwardable_compare_integer_v<A> ||
             detail::is_forwardable_compare_integer_v<B>)
[[nodiscard]] constexpr auto le(A lhs, B rhs) noexcept
    -> decltype(le(
        static_cast<detail::forwarded_compare_integer_t<A>>(lhs),
        static_cast<detail::forwarded_compare_integer_t<B>>(rhs)))
{
    return le(
        static_cast<detail::forwarded_compare_integer_t<A>>(lhs),
        static_cast<detail::forwarded_compare_integer_t<B>>(rhs));
}

template<typename A, typename B>
    requires(detail::is_forwardable_compare_integer_v<A> ||
             detail::is_forwardable_compare_integer_v<B>)
[[nodiscard]] constexpr auto gt(A lhs, B rhs) noexcept
    -> decltype(gt(
        static_cast<detail::forwarded_compare_integer_t<A>>(lhs),
        static_cast<detail::forwarded_compare_integer_t<B>>(rhs)))
{
    return gt(
        static_cast<detail::forwarded_compare_integer_t<A>>(lhs),
        static_cast<detail::forwarded_compare_integer_t<B>>(rhs));
}

template<typename A, typename B>
    requires(detail::is_forwardable_compare_integer_v<A> ||
             detail::is_forwardable_compare_integer_v<B>)
[[nodiscard]] constexpr auto ge(A lhs, B rhs) noexcept
    -> decltype(ge(
        static_cast<detail::forwarded_compare_integer_t<A>>(lhs),
        static_cast<detail::forwarded_compare_integer_t<B>>(rhs)))
{
    return ge(
        static_cast<detail::forwarded_compare_integer_t<A>>(lhs),
        static_cast<detail::forwarded_compare_integer_t<B>>(rhs));
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr result<bool> eq(
    const result<T>& lhs,
    U rhs) noexcept
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    return eq(*lhs, rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr result<bool> eq(
    T lhs,
    const result<U>& rhs) noexcept
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return eq(lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr result<bool> eq(
    const result<T>& lhs,
    const result<U>& rhs) noexcept
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return eq(*lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr result<bool> ne(
    const result<T>& lhs,
    U rhs) noexcept
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    return ne(*lhs, rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr result<bool> ne(
    T lhs,
    const result<U>& rhs) noexcept
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return ne(lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr result<bool> ne(
    const result<T>& lhs,
    const result<U>& rhs) noexcept
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return ne(*lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr result<bool> lt(
    const result<T>& lhs,
    U rhs) noexcept
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    return lt(*lhs, rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr result<bool> lt(
    T lhs,
    const result<U>& rhs) noexcept
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return lt(lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr result<bool> lt(
    const result<T>& lhs,
    const result<U>& rhs) noexcept
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return lt(*lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr result<bool> le(
    const result<T>& lhs,
    U rhs) noexcept
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    return le(*lhs, rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr result<bool> le(
    T lhs,
    const result<U>& rhs) noexcept
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return le(lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr result<bool> le(
    const result<T>& lhs,
    const result<U>& rhs) noexcept
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return le(*lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr result<bool> gt(
    const result<T>& lhs,
    U rhs) noexcept
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    return gt(*lhs, rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr result<bool> gt(
    T lhs,
    const result<U>& rhs) noexcept
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return gt(lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr result<bool> gt(
    const result<T>& lhs,
    const result<U>& rhs) noexcept
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return gt(*lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr result<bool> ge(
    const result<T>& lhs,
    U rhs) noexcept
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    return ge(*lhs, rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr result<bool> ge(
    T lhs,
    const result<U>& rhs) noexcept
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return ge(lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr result<bool> ge(
    const result<T>& lhs,
    const result<U>& rhs) noexcept
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }
    if (!rhs) {
        return std::unexpected(rhs.error());
    }
    return ge(*lhs, *rhs);
}

} // namespace xer

#endif

/**
 * @file xer/bits/basic_arithmetic.h
 * @brief Internal integer basic arithmetic function implementations.
 */

#pragma once

#ifndef XER_BITS_BASIC_ARITHMETIC_H_INCLUDED_
#define XER_BITS_BASIC_ARITHMETIC_H_INCLUDED_

#include <concepts>
#include <expected>
#include <limits>
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
inline constexpr bool is_canonical_integer_v =
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
inline constexpr bool is_forwardable_integer_v =
    std::integral<std::remove_cvref_t<T>> &&
    !std::same_as<std::remove_cvref_t<T>, bool> &&
    !is_canonical_integer_v<std::remove_cvref_t<T>>;

/**
 * @brief Canonical forwarding target for a non-canonical integer type.
 *
 * Signed narrow integers are forwarded to `int`.
 * Unsigned narrow integers are forwarded to `unsigned int`.
 *
 * @tparam T Source integer type.
 */
template<typename T>
using forwarded_integer_t = std::conditional_t<
    std::signed_integral<std::remove_cvref_t<T>>,
    int,
    unsigned int>;

/**
 * @brief Mathematical integer represented by sign and magnitude.
 */
struct integer_value {
    bool negative;
    xer::uint128_t magnitude;
};

/**
 * @brief Creates an arithmetic overflow error.
 *
 * @return Overflow error.
 */
[[nodiscard]] constexpr error<void> make_arithmetic_overflow_error() noexcept
{
    return make_error(error_t::overflow_error);
}

/**
 * @brief Creates an arithmetic out-of-range error.
 *
 * @return Out-of-range error.
 */
[[nodiscard]] constexpr error<void> make_arithmetic_out_of_range_error() noexcept
{
    return make_error(error_t::out_of_range);
}

/**
 * @brief Converts an integer operand to sign-magnitude form.
 *
 * @tparam T Canonical integer type.
 * @param value Integer operand.
 * @return Sign-magnitude representation.
 */
template<typename T>
    requires(is_canonical_integer_v<T>)
[[nodiscard]] constexpr integer_value to_integer_value(T value) noexcept
{
    if constexpr (std::unsigned_integral<T>) {
        return integer_value{
            false,
            static_cast<xer::uint128_t>(value),
        };
    } else {
        if (value >= 0) {
            return integer_value{
                false,
                static_cast<xer::uint128_t>(value),
            };
        }

        return integer_value{
            true,
            static_cast<xer::uint128_t>(-(value + 1)) + 1,
        };
    }
}

/**
 * @brief Negates a sign-magnitude integer.
 *
 * @param value Source value.
 * @return Negated value.
 */
[[nodiscard]] constexpr integer_value negate_integer_value(integer_value value) noexcept
{
    if (value.magnitude == 0) {
        return integer_value{false, 0};
    }

    value.negative = !value.negative;
    return value;
}

/**
 * @brief Adds two sign-magnitude integers.
 *
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Sum, or an error if the magnitude overflows `uint128_t`.
 */
[[nodiscard]] constexpr std::expected<integer_value, error<void>> add_integer_value(
    integer_value lhs,
    integer_value rhs) noexcept
{
    if (lhs.negative == rhs.negative) {
        const xer::uint128_t max_value =
            std::numeric_limits<xer::uint128_t>::max();

        if (max_value - lhs.magnitude < rhs.magnitude) {
            return std::unexpected(make_arithmetic_overflow_error());
        }

        return integer_value{
            lhs.negative,
            lhs.magnitude + rhs.magnitude,
        };
    }

    if (lhs.magnitude == rhs.magnitude) {
        return integer_value{false, 0};
    }

    if (lhs.magnitude > rhs.magnitude) {
        return integer_value{
            lhs.negative,
            lhs.magnitude - rhs.magnitude,
        };
    }

    return integer_value{
        rhs.negative,
        rhs.magnitude - lhs.magnitude,
    };
}

/**
 * @brief Subtracts two sign-magnitude integers.
 *
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Difference, or an error if the magnitude overflows `uint128_t`.
 */
[[nodiscard]] constexpr std::expected<integer_value, error<void>> sub_integer_value(
    integer_value lhs,
    integer_value rhs) noexcept
{
    return add_integer_value(lhs, negate_integer_value(rhs));
}

/**
 * @brief Multiplies two sign-magnitude integers.
 *
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Product, or an error if the magnitude overflows `uint128_t`.
 */
[[nodiscard]] constexpr std::expected<integer_value, error<void>> mul_integer_value(
    integer_value lhs,
    integer_value rhs) noexcept
{
    if (lhs.magnitude == 0 || rhs.magnitude == 0) {
        return integer_value{false, 0};
    }

    const xer::uint128_t max_value =
        std::numeric_limits<xer::uint128_t>::max();

    if (lhs.magnitude > max_value / rhs.magnitude) {
        return std::unexpected(make_arithmetic_overflow_error());
    }

    return integer_value{
        lhs.negative != rhs.negative,
        lhs.magnitude * rhs.magnitude,
    };
}

/**
 * @brief Converts a mathematical integer to the signed public result type.
 *
 * @param value Source value.
 * @return Signed result, or an error if out of range.
 */
[[nodiscard]] constexpr std::expected<xer::int64_t, error<void>>
to_signed_result(integer_value value) noexcept
{
    constexpr xer::uint128_t positive_limit =
        static_cast<xer::uint128_t>(std::numeric_limits<xer::int64_t>::max());
    constexpr xer::uint128_t negative_limit = positive_limit + 1;

    if (!value.negative) {
        if (value.magnitude > positive_limit) {
            return std::unexpected(make_arithmetic_out_of_range_error());
        }

        return static_cast<xer::int64_t>(value.magnitude);
    }

    if (value.magnitude > negative_limit) {
        return std::unexpected(make_arithmetic_out_of_range_error());
    }

    if (value.magnitude == negative_limit) {
        return std::numeric_limits<xer::int64_t>::min();
    }

    return -static_cast<xer::int64_t>(value.magnitude);
}

/**
 * @brief Converts a mathematical integer to the unsigned public result type.
 *
 * @param value Source value.
 * @return Unsigned result, or an error if out of range.
 */
[[nodiscard]] constexpr std::expected<xer::uint64_t, error<void>>
to_unsigned_result(integer_value value) noexcept
{
    if (value.negative) {
        return std::unexpected(make_arithmetic_out_of_range_error());
    }

    constexpr xer::uint128_t positive_limit =
        static_cast<xer::uint128_t>(std::numeric_limits<xer::uint64_t>::max());

    if (value.magnitude > positive_limit) {
        return std::unexpected(make_arithmetic_out_of_range_error());
    }

    return static_cast<xer::uint64_t>(value.magnitude);
}

/**
 * @brief Implements signed addition for canonical integer operand types.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Sum in the public signed result type.
 */
template<typename A, typename B>
    requires(is_canonical_integer_v<A> && is_canonical_integer_v<B>)
[[nodiscard]] constexpr std::expected<xer::int64_t, error<void>> add_canonical(
    A lhs,
    B rhs) noexcept
{
    const auto result =
        add_integer_value(to_integer_value(lhs), to_integer_value(rhs));
    if (!result) {
        return std::unexpected(result.error());
    }

    return to_signed_result(*result);
}

/**
 * @brief Implements unsigned addition for canonical integer operand types.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Sum in the public unsigned result type.
 */
template<typename A, typename B>
    requires(is_canonical_integer_v<A> && is_canonical_integer_v<B>)
[[nodiscard]] constexpr std::expected<xer::uint64_t, error<void>> uadd_canonical(
    A lhs,
    B rhs) noexcept
{
    const auto result =
        add_integer_value(to_integer_value(lhs), to_integer_value(rhs));
    if (!result) {
        return std::unexpected(result.error());
    }

    return to_unsigned_result(*result);
}

/**
 * @brief Implements signed subtraction for canonical integer operand types.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Difference in the public signed result type.
 */
template<typename A, typename B>
    requires(is_canonical_integer_v<A> && is_canonical_integer_v<B>)
[[nodiscard]] constexpr std::expected<xer::int64_t, error<void>> sub_canonical(
    A lhs,
    B rhs) noexcept
{
    const auto result =
        sub_integer_value(to_integer_value(lhs), to_integer_value(rhs));
    if (!result) {
        return std::unexpected(result.error());
    }

    return to_signed_result(*result);
}

/**
 * @brief Implements unsigned subtraction for canonical integer operand types.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Difference in the public unsigned result type.
 */
template<typename A, typename B>
    requires(is_canonical_integer_v<A> && is_canonical_integer_v<B>)
[[nodiscard]] constexpr std::expected<xer::uint64_t, error<void>> usub_canonical(
    A lhs,
    B rhs) noexcept
{
    const auto result =
        sub_integer_value(to_integer_value(lhs), to_integer_value(rhs));
    if (!result) {
        return std::unexpected(result.error());
    }

    return to_unsigned_result(*result);
}

/**
 * @brief Implements signed multiplication for canonical integer operand types.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Product in the public signed result type.
 */
template<typename A, typename B>
    requires(is_canonical_integer_v<A> && is_canonical_integer_v<B>)
[[nodiscard]] constexpr std::expected<xer::int64_t, error<void>> mul_canonical(
    A lhs,
    B rhs) noexcept
{
    const auto result =
        mul_integer_value(to_integer_value(lhs), to_integer_value(rhs));
    if (!result) {
        return std::unexpected(result.error());
    }

    return to_signed_result(*result);
}

/**
 * @brief Implements unsigned multiplication for canonical integer operand types.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Product in the public unsigned result type.
 */
template<typename A, typename B>
    requires(is_canonical_integer_v<A> && is_canonical_integer_v<B>)
[[nodiscard]] constexpr std::expected<xer::uint64_t, error<void>> umul_canonical(
    A lhs,
    B rhs) noexcept
{
    const auto result =
        mul_integer_value(to_integer_value(lhs), to_integer_value(rhs));
    if (!result) {
        return std::unexpected(result.error());
    }

    return to_unsigned_result(*result);
}

} // namespace xer::detail

namespace xer {

#define XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(A, B)                              \
    [[nodiscard]] constexpr std::expected<xer::int64_t, error<void>> add(          \
        A lhs,                                                                      \
        B rhs) noexcept                                                             \
    {                                                                               \
        return detail::add_canonical(lhs, rhs);                                     \
    }                                                                               \
                                                                                    \
    [[nodiscard]] constexpr std::expected<xer::uint64_t, error<void>> uadd(        \
        A lhs,                                                                      \
        B rhs) noexcept                                                             \
    {                                                                               \
        return detail::uadd_canonical(lhs, rhs);                                    \
    }                                                                               \
                                                                                    \
    [[nodiscard]] constexpr std::expected<xer::int64_t, error<void>> sub(          \
        A lhs,                                                                      \
        B rhs) noexcept                                                             \
    {                                                                               \
        return detail::sub_canonical(lhs, rhs);                                     \
    }                                                                               \
                                                                                    \
    [[nodiscard]] constexpr std::expected<xer::uint64_t, error<void>> usub(        \
        A lhs,                                                                      \
        B rhs) noexcept                                                             \
    {                                                                               \
        return detail::usub_canonical(lhs, rhs);                                    \
    }                                                                               \
                                                                                    \
    [[nodiscard]] constexpr std::expected<xer::int64_t, error<void>> mul(          \
        A lhs,                                                                      \
        B rhs) noexcept                                                             \
    {                                                                               \
        return detail::mul_canonical(lhs, rhs);                                     \
    }                                                                               \
                                                                                    \
    [[nodiscard]] constexpr std::expected<xer::uint64_t, error<void>> umul(        \
        A lhs,                                                                      \
        B rhs) noexcept                                                             \
    {                                                                               \
        return detail::umul_canonical(lhs, rhs);                                    \
    }

XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(int, int)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(int, unsigned int)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(int, long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(int, unsigned long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(int, long long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(int, unsigned long long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(int, xer::int128_t)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(int, xer::uint128_t)

XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned int, int)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned int, unsigned int)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned int, long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned int, unsigned long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned int, long long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned int, unsigned long long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned int, xer::int128_t)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned int, xer::uint128_t)

XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(long, int)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(long, unsigned int)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(long, long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(long, unsigned long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(long, long long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(long, unsigned long long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(long, xer::int128_t)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(long, xer::uint128_t)

XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned long, int)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned long, unsigned int)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned long, long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned long, unsigned long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned long, long long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned long, unsigned long long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned long, xer::int128_t)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned long, xer::uint128_t)

XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(long long, int)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(long long, unsigned int)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(long long, long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(long long, unsigned long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(long long, long long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(long long, unsigned long long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(long long, xer::int128_t)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(long long, xer::uint128_t)

XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned long long, int)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned long long, unsigned int)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned long long, long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned long long, unsigned long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned long long, long long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned long long, unsigned long long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned long long, xer::int128_t)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(unsigned long long, xer::uint128_t)

XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(xer::int128_t, int)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(xer::int128_t, unsigned int)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(xer::int128_t, long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(xer::int128_t, unsigned long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(xer::int128_t, long long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(xer::int128_t, unsigned long long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(xer::int128_t, xer::int128_t)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(xer::int128_t, xer::uint128_t)

XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(xer::uint128_t, int)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(xer::uint128_t, unsigned int)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(xer::uint128_t, long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(xer::uint128_t, unsigned long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(xer::uint128_t, long long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(xer::uint128_t, unsigned long long)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(xer::uint128_t, xer::int128_t)
XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR(xer::uint128_t, xer::uint128_t)

#undef XER_DETAIL_DECLARE_BASIC_ARITHMETIC_PAIR

/**
 * @brief Forwards narrow integer operands to the canonical overload set.
 *
 * @tparam A Left-hand side type.
 * @tparam B Right-hand side type.
 * @param lhs Left-hand side.
 * @param rhs Right-hand side.
 * @return Result of `add`.
 */
template<typename A, typename B>
    requires(detail::is_forwardable_integer_v<A> || detail::is_forwardable_integer_v<B>)
[[nodiscard]] constexpr auto add(A lhs, B rhs) noexcept
    -> decltype(add(
        static_cast<detail::forwarded_integer_t<A>>(lhs),
        static_cast<detail::forwarded_integer_t<B>>(rhs)))
{
    return add(
        static_cast<detail::forwarded_integer_t<A>>(lhs),
        static_cast<detail::forwarded_integer_t<B>>(rhs));
}

template<typename A, typename B>
    requires(detail::is_forwardable_integer_v<A> || detail::is_forwardable_integer_v<B>)
[[nodiscard]] constexpr auto uadd(A lhs, B rhs) noexcept
    -> decltype(uadd(
        static_cast<detail::forwarded_integer_t<A>>(lhs),
        static_cast<detail::forwarded_integer_t<B>>(rhs)))
{
    return uadd(
        static_cast<detail::forwarded_integer_t<A>>(lhs),
        static_cast<detail::forwarded_integer_t<B>>(rhs));
}

template<typename A, typename B>
    requires(detail::is_forwardable_integer_v<A> || detail::is_forwardable_integer_v<B>)
[[nodiscard]] constexpr auto sub(A lhs, B rhs) noexcept
    -> decltype(sub(
        static_cast<detail::forwarded_integer_t<A>>(lhs),
        static_cast<detail::forwarded_integer_t<B>>(rhs)))
{
    return sub(
        static_cast<detail::forwarded_integer_t<A>>(lhs),
        static_cast<detail::forwarded_integer_t<B>>(rhs));
}

template<typename A, typename B>
    requires(detail::is_forwardable_integer_v<A> || detail::is_forwardable_integer_v<B>)
[[nodiscard]] constexpr auto usub(A lhs, B rhs) noexcept
    -> decltype(usub(
        static_cast<detail::forwarded_integer_t<A>>(lhs),
        static_cast<detail::forwarded_integer_t<B>>(rhs)))
{
    return usub(
        static_cast<detail::forwarded_integer_t<A>>(lhs),
        static_cast<detail::forwarded_integer_t<B>>(rhs));
}

template<typename A, typename B>
    requires(detail::is_forwardable_integer_v<A> || detail::is_forwardable_integer_v<B>)
[[nodiscard]] constexpr auto mul(A lhs, B rhs) noexcept
    -> decltype(mul(
        static_cast<detail::forwarded_integer_t<A>>(lhs),
        static_cast<detail::forwarded_integer_t<B>>(rhs)))
{
    return mul(
        static_cast<detail::forwarded_integer_t<A>>(lhs),
        static_cast<detail::forwarded_integer_t<B>>(rhs));
}

template<typename A, typename B>
    requires(detail::is_forwardable_integer_v<A> || detail::is_forwardable_integer_v<B>)
[[nodiscard]] constexpr auto umul(A lhs, B rhs) noexcept
    -> decltype(umul(
        static_cast<detail::forwarded_integer_t<A>>(lhs),
        static_cast<detail::forwarded_integer_t<B>>(rhs)))
{
    return umul(
        static_cast<detail::forwarded_integer_t<A>>(lhs),
        static_cast<detail::forwarded_integer_t<B>>(rhs));
}

/**
 * @brief Propagates an error from the left operand if present.
 *
 * @tparam T Expected value type.
 * @tparam U Right-hand side type.
 * @param lhs Left-hand side expected value.
 * @param rhs Right-hand side operand.
 * @return Result of `add`.
 */
template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto add(
    const std::expected<T, error<void>>& lhs,
    U rhs) noexcept -> decltype(add(std::declval<T>(), rhs))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    return add(*lhs, rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto add(
    T lhs,
    const std::expected<U, error<void>>& rhs) noexcept -> decltype(add(lhs, std::declval<U>()))
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return add(lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto add(
    const std::expected<T, error<void>>& lhs,
    const std::expected<U, error<void>>& rhs) noexcept -> decltype(add(std::declval<T>(), std::declval<U>()))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return add(*lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto uadd(
    const std::expected<T, error<void>>& lhs,
    U rhs) noexcept -> decltype(uadd(std::declval<T>(), rhs))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    return uadd(*lhs, rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto uadd(
    T lhs,
    const std::expected<U, error<void>>& rhs) noexcept -> decltype(uadd(lhs, std::declval<U>()))
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return uadd(lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto uadd(
    const std::expected<T, error<void>>& lhs,
    const std::expected<U, error<void>>& rhs) noexcept -> decltype(uadd(std::declval<T>(), std::declval<U>()))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return uadd(*lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto sub(
    const std::expected<T, error<void>>& lhs,
    U rhs) noexcept -> decltype(sub(std::declval<T>(), rhs))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    return sub(*lhs, rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto sub(
    T lhs,
    const std::expected<U, error<void>>& rhs) noexcept -> decltype(sub(lhs, std::declval<U>()))
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return sub(lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto sub(
    const std::expected<T, error<void>>& lhs,
    const std::expected<U, error<void>>& rhs) noexcept -> decltype(sub(std::declval<T>(), std::declval<U>()))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return sub(*lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto usub(
    const std::expected<T, error<void>>& lhs,
    U rhs) noexcept -> decltype(usub(std::declval<T>(), rhs))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    return usub(*lhs, rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto usub(
    T lhs,
    const std::expected<U, error<void>>& rhs) noexcept -> decltype(usub(lhs, std::declval<U>()))
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return usub(lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto usub(
    const std::expected<T, error<void>>& lhs,
    const std::expected<U, error<void>>& rhs) noexcept -> decltype(usub(std::declval<T>(), std::declval<U>()))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return usub(*lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto mul(
    const std::expected<T, error<void>>& lhs,
    U rhs) noexcept -> decltype(mul(std::declval<T>(), rhs))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    return mul(*lhs, rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto mul(
    T lhs,
    const std::expected<U, error<void>>& rhs) noexcept -> decltype(mul(lhs, std::declval<U>()))
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return mul(lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto mul(
    const std::expected<T, error<void>>& lhs,
    const std::expected<U, error<void>>& rhs) noexcept -> decltype(mul(std::declval<T>(), std::declval<U>()))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return mul(*lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto umul(
    const std::expected<T, error<void>>& lhs,
    U rhs) noexcept -> decltype(umul(std::declval<T>(), rhs))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    return umul(*lhs, rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto umul(
    T lhs,
    const std::expected<U, error<void>>& rhs) noexcept -> decltype(umul(lhs, std::declval<U>()))
{
    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return umul(lhs, *rhs);
}

template<typename T, typename U>
    requires(std::integral<T> && !std::same_as<T, bool> &&
             std::integral<U> && !std::same_as<U, bool>)
[[nodiscard]] constexpr auto umul(
    const std::expected<T, error<void>>& lhs,
    const std::expected<U, error<void>>& rhs) noexcept -> decltype(umul(std::declval<T>(), std::declval<U>()))
{
    if (!lhs) {
        return std::unexpected(lhs.error());
    }

    if (!rhs) {
        return std::unexpected(rhs.error());
    }

    return umul(*lhs, *rhs);
}

} // namespace xer

#endif

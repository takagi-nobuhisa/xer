/**
 * @file xer/bits/minmax.h
 * @brief Internal min/max/clamp function implementations.
 */

#pragma once

#ifndef XER_BITS_MINMAX_H_INCLUDED_
#define XER_BITS_MINMAX_H_INCLUDED_

#include <expected>
#include <type_traits>
#include <utility>

#include <xer/bits/arithmetic_concepts.h>
#include <xer/bits/compare.h>
#include <xer/bits/in_range.h>
#include <xer/error.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace xer::detail {

/**
 * @brief Common arithmetic result type for min/max/clamp.
 *
 * @tparam A First operand type.
 * @tparam B Second operand type.
 */
template<typename A, typename B>
using minmax_common_t =
    std::common_type_t<std::remove_cvref_t<A>, std::remove_cvref_t<B>>;

/**
 * @brief Common arithmetic result type for clamp.
 *
 * @tparam A Value type.
 * @tparam B Boundary type.
 * @tparam C Boundary type.
 */
template<typename A, typename B, typename C>
using clamp_common_t = std::common_type_t<
    std::remove_cvref_t<A>,
    std::remove_cvref_t<B>,
    std::remove_cvref_t<C>>;

/**
 * @brief Converts a selected min/max operand to the common result type.
 *
 * @tparam R Result type.
 * @tparam T Source type.
 * @param value Selected source value.
 * @return Converted value or out_of_range.
 */
template<typename R, typename T>
    requires non_bool_arithmetic<R> && non_bool_arithmetic<T>
[[nodiscard]] constexpr auto convert_minmax_result(T value) -> result<R>
{
    if (!in_range<R>(value)) {
        return std::unexpected(make_error(error_t::out_of_range));
    }

    return static_cast<R>(value);
}

/**
 * @brief Validates clamp bounds.
 *
 * @tparam Lo Lower-bound type.
 * @tparam Hi Upper-bound type.
 * @param lo Lower bound.
 * @param hi Upper bound.
 * @return true if bounds are valid.
 */
template<typename Lo, typename Hi>
    requires non_bool_arithmetic<Lo> && non_bool_arithmetic<Hi>
[[nodiscard]] constexpr auto valid_clamp_bounds(Lo lo, Hi hi) noexcept -> bool
{
    return !lt(hi, lo);
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Returns the smaller of two arithmetic values.
 *
 * The return type is `std::common_type_t<A, B>`.
 * If the selected value is not representable in that type, this function
 * returns `error_t::out_of_range`.
 *
 * @tparam A First operand type.
 * @tparam B Second operand type.
 * @param lhs First operand.
 * @param rhs Second operand.
 * @return Smaller value.
 */
template<typename A, typename B>
    requires non_bool_arithmetic<A> && non_bool_arithmetic<B> &&
             requires { typename detail::minmax_common_t<A, B>; } &&
             non_bool_arithmetic<detail::minmax_common_t<A, B>>
[[nodiscard]] constexpr auto min(A lhs, B rhs)
    -> result<detail::minmax_common_t<A, B>>
{
    using result_t = detail::minmax_common_t<A, B>;

    if (lt(rhs, lhs)) {
        return detail::convert_minmax_result<result_t>(rhs);
    }

    return detail::convert_minmax_result<result_t>(lhs);
}

/**
 * @brief Returns the larger of two arithmetic values.
 *
 * The return type is `std::common_type_t<A, B>`.
 * If the selected value is not representable in that type, this function
 * returns `error_t::out_of_range`.
 *
 * @tparam A First operand type.
 * @tparam B Second operand type.
 * @param lhs First operand.
 * @param rhs Second operand.
 * @return Larger value.
 */
template<typename A, typename B>
    requires non_bool_arithmetic<A> && non_bool_arithmetic<B> &&
             requires { typename detail::minmax_common_t<A, B>; } &&
             non_bool_arithmetic<detail::minmax_common_t<A, B>>
[[nodiscard]] constexpr auto max(A lhs, B rhs)
    -> result<detail::minmax_common_t<A, B>>
{
    using result_t = detail::minmax_common_t<A, B>;

    if (lt(lhs, rhs)) {
        return detail::convert_minmax_result<result_t>(rhs);
    }

    return detail::convert_minmax_result<result_t>(lhs);
}

/**
 * @brief Clamps a value to the closed interval [lo, hi].
 *
 * The return type is `std::common_type_t<T, Lo, Hi>`.
 * If `hi < lo`, this function returns `error_t::invalid_argument`.
 * If the selected value is not representable in the result type, this function
 * returns `error_t::out_of_range`.
 *
 * @tparam T Value type.
 * @tparam Lo Lower-bound type.
 * @tparam Hi Upper-bound type.
 * @param value Value to clamp.
 * @param lo Lower bound.
 * @param hi Upper bound.
 * @return Clamped value.
 */
template<typename T, typename Lo, typename Hi>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<Lo> &&
             non_bool_arithmetic<Hi> &&
             requires { typename detail::clamp_common_t<T, Lo, Hi>; } &&
             non_bool_arithmetic<detail::clamp_common_t<T, Lo, Hi>>
[[nodiscard]] constexpr auto clamp(T value, Lo lo, Hi hi)
    -> result<detail::clamp_common_t<T, Lo, Hi>>
{
    using result_t = detail::clamp_common_t<T, Lo, Hi>;

    if (!detail::valid_clamp_bounds(lo, hi)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (lt(value, lo)) {
        return detail::convert_minmax_result<result_t>(lo);
    }

    if (lt(hi, value)) {
        return detail::convert_minmax_result<result_t>(hi);
    }

    return detail::convert_minmax_result<result_t>(value);
}

/**
 * @brief Returns the smaller of a successful result value and a raw value.
 *
 * Errors are propagated unchanged.
 *
 * @tparam A First operand type.
 * @tparam B Second operand type.
 * @tparam Detail Error detail type.
 * @param lhs First operand.
 * @param rhs Second operand.
 * @return Smaller value.
 */
template<typename A, typename B, typename Detail>
    requires non_bool_arithmetic<A> && non_bool_arithmetic<B> &&
             requires { typename detail::minmax_common_t<A, B>; } &&
             non_bool_arithmetic<detail::minmax_common_t<A, B>>
[[nodiscard]] constexpr auto min(const result<A, Detail>& lhs, B rhs)
    -> result<detail::minmax_common_t<A, B>, Detail>
{
    if (!lhs.has_value()) {
        return std::unexpected(lhs.error());
    }

    const auto r = min(*lhs, rhs);
    if (!r.has_value()) {
        return std::unexpected(r.error());
    }

    return *r;
}

/**
 * @brief Returns the smaller of a raw value and a successful result value.
 *
 * Errors are propagated unchanged.
 *
 * @tparam A First operand type.
 * @tparam B Second operand type.
 * @tparam Detail Error detail type.
 * @param lhs First operand.
 * @param rhs Second operand.
 * @return Smaller value.
 */
template<typename A, typename B, typename Detail>
    requires non_bool_arithmetic<A> && non_bool_arithmetic<B> &&
             requires { typename detail::minmax_common_t<A, B>; } &&
             non_bool_arithmetic<detail::minmax_common_t<A, B>>
[[nodiscard]] constexpr auto min(A lhs, const result<B, Detail>& rhs)
    -> result<detail::minmax_common_t<A, B>, Detail>
{
    if (!rhs.has_value()) {
        return std::unexpected(rhs.error());
    }

    const auto r = min(lhs, *rhs);
    if (!r.has_value()) {
        return std::unexpected(r.error());
    }

    return *r;
}

/**
 * @brief Returns the smaller of two successful result values.
 *
 * Errors are propagated unchanged.
 *
 * @tparam A First operand type.
 * @tparam B Second operand type.
 * @tparam Detail Error detail type.
 * @param lhs First operand.
 * @param rhs Second operand.
 * @return Smaller value.
 */
template<typename A, typename B, typename Detail>
    requires non_bool_arithmetic<A> && non_bool_arithmetic<B> &&
             requires { typename detail::minmax_common_t<A, B>; } &&
             non_bool_arithmetic<detail::minmax_common_t<A, B>>
[[nodiscard]] constexpr auto min(
    const result<A, Detail>& lhs,
    const result<B, Detail>& rhs)
    -> result<detail::minmax_common_t<A, B>, Detail>
{
    if (!lhs.has_value()) {
        return std::unexpected(lhs.error());
    }

    if (!rhs.has_value()) {
        return std::unexpected(rhs.error());
    }

    const auto r = min(*lhs, *rhs);
    if (!r.has_value()) {
        return std::unexpected(r.error());
    }

    return *r;
}

/**
 * @brief Returns the larger of a successful result value and a raw value.
 *
 * Errors are propagated unchanged.
 *
 * @tparam A First operand type.
 * @tparam B Second operand type.
 * @tparam Detail Error detail type.
 * @param lhs First operand.
 * @param rhs Second operand.
 * @return Larger value.
 */
template<typename A, typename B, typename Detail>
    requires non_bool_arithmetic<A> && non_bool_arithmetic<B> &&
             requires { typename detail::minmax_common_t<A, B>; } &&
             non_bool_arithmetic<detail::minmax_common_t<A, B>>
[[nodiscard]] constexpr auto max(const result<A, Detail>& lhs, B rhs)
    -> result<detail::minmax_common_t<A, B>, Detail>
{
    if (!lhs.has_value()) {
        return std::unexpected(lhs.error());
    }

    const auto r = max(*lhs, rhs);
    if (!r.has_value()) {
        return std::unexpected(r.error());
    }

    return *r;
}

/**
 * @brief Returns the larger of a raw value and a successful result value.
 *
 * Errors are propagated unchanged.
 *
 * @tparam A First operand type.
 * @tparam B Second operand type.
 * @tparam Detail Error detail type.
 * @param lhs First operand.
 * @param rhs Second operand.
 * @return Larger value.
 */
template<typename A, typename B, typename Detail>
    requires non_bool_arithmetic<A> && non_bool_arithmetic<B> &&
             requires { typename detail::minmax_common_t<A, B>; } &&
             non_bool_arithmetic<detail::minmax_common_t<A, B>>
[[nodiscard]] constexpr auto max(A lhs, const result<B, Detail>& rhs)
    -> result<detail::minmax_common_t<A, B>, Detail>
{
    if (!rhs.has_value()) {
        return std::unexpected(rhs.error());
    }

    const auto r = max(lhs, *rhs);
    if (!r.has_value()) {
        return std::unexpected(r.error());
    }

    return *r;
}

/**
 * @brief Returns the larger of two successful result values.
 *
 * Errors are propagated unchanged.
 *
 * @tparam A First operand type.
 * @tparam B Second operand type.
 * @tparam Detail Error detail type.
 * @param lhs First operand.
 * @param rhs Second operand.
 * @return Larger value.
 */
template<typename A, typename B, typename Detail>
    requires non_bool_arithmetic<A> && non_bool_arithmetic<B> &&
             requires { typename detail::minmax_common_t<A, B>; } &&
             non_bool_arithmetic<detail::minmax_common_t<A, B>>
[[nodiscard]] constexpr auto max(
    const result<A, Detail>& lhs,
    const result<B, Detail>& rhs)
    -> result<detail::minmax_common_t<A, B>, Detail>
{
    if (!lhs.has_value()) {
        return std::unexpected(lhs.error());
    }

    if (!rhs.has_value()) {
        return std::unexpected(rhs.error());
    }

    const auto r = max(*lhs, *rhs);
    if (!r.has_value()) {
        return std::unexpected(r.error());
    }

    return *r;
}

/**
 * @brief Clamps a successful result value to the closed interval [lo, hi].
 *
 * Errors are propagated unchanged.
 *
 * @tparam T Value type.
 * @tparam Lo Lower-bound type.
 * @tparam Hi Upper-bound type.
 * @tparam Detail Error detail type.
 * @param value Value to clamp.
 * @param lo Lower bound.
 * @param hi Upper bound.
 * @return Clamped value.
 */
template<typename T, typename Lo, typename Hi, typename Detail>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<Lo> &&
             non_bool_arithmetic<Hi> &&
             requires { typename detail::clamp_common_t<T, Lo, Hi>; } &&
             non_bool_arithmetic<detail::clamp_common_t<T, Lo, Hi>>
[[nodiscard]] constexpr auto clamp(
    const result<T, Detail>& value,
    Lo lo,
    Hi hi) -> result<detail::clamp_common_t<T, Lo, Hi>, Detail>
{
    if (!value.has_value()) {
        return std::unexpected(value.error());
    }

    const auto r = clamp(*value, lo, hi);
    if (!r.has_value()) {
        return std::unexpected(r.error());
    }

    return *r;
}

/**
 * @brief Clamps a raw value using a successful lower bound and a raw upper bound.
 *
 * Errors are propagated unchanged.
 *
 * @tparam T Value type.
 * @tparam Lo Lower-bound type.
 * @tparam Hi Upper-bound type.
 * @tparam Detail Error detail type.
 * @param value Value to clamp.
 * @param lo Lower bound.
 * @param hi Upper bound.
 * @return Clamped value.
 */
template<typename T, typename Lo, typename Hi, typename Detail>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<Lo> &&
             non_bool_arithmetic<Hi> &&
             requires { typename detail::clamp_common_t<T, Lo, Hi>; } &&
             non_bool_arithmetic<detail::clamp_common_t<T, Lo, Hi>>
[[nodiscard]] constexpr auto clamp(
    T value,
    const result<Lo, Detail>& lo,
    Hi hi) -> result<detail::clamp_common_t<T, Lo, Hi>, Detail>
{
    if (!lo.has_value()) {
        return std::unexpected(lo.error());
    }

    const auto r = clamp(value, *lo, hi);
    if (!r.has_value()) {
        return std::unexpected(r.error());
    }

    return *r;
}

/**
 * @brief Clamps a raw value using a raw lower bound and a successful upper bound.
 *
 * Errors are propagated unchanged.
 *
 * @tparam T Value type.
 * @tparam Lo Lower-bound type.
 * @tparam Hi Upper-bound type.
 * @tparam Detail Error detail type.
 * @param value Value to clamp.
 * @param lo Lower bound.
 * @param hi Upper bound.
 * @return Clamped value.
 */
template<typename T, typename Lo, typename Hi, typename Detail>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<Lo> &&
             non_bool_arithmetic<Hi> &&
             requires { typename detail::clamp_common_t<T, Lo, Hi>; } &&
             non_bool_arithmetic<detail::clamp_common_t<T, Lo, Hi>>
[[nodiscard]] constexpr auto clamp(
    T value,
    Lo lo,
    const result<Hi, Detail>& hi)
    -> result<detail::clamp_common_t<T, Lo, Hi>, Detail>
{
    if (!hi.has_value()) {
        return std::unexpected(hi.error());
    }

    const auto r = clamp(value, lo, *hi);
    if (!r.has_value()) {
        return std::unexpected(r.error());
    }

    return *r;
}

/**
 * @brief Clamps a successful result value using a successful lower bound and a raw upper bound.
 *
 * Errors are propagated unchanged.
 *
 * @tparam T Value type.
 * @tparam Lo Lower-bound type.
 * @tparam Hi Upper-bound type.
 * @tparam Detail Error detail type.
 * @param value Value to clamp.
 * @param lo Lower bound.
 * @param hi Upper bound.
 * @return Clamped value.
 */
template<typename T, typename Lo, typename Hi, typename Detail>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<Lo> &&
             non_bool_arithmetic<Hi> &&
             requires { typename detail::clamp_common_t<T, Lo, Hi>; } &&
             non_bool_arithmetic<detail::clamp_common_t<T, Lo, Hi>>
[[nodiscard]] constexpr auto clamp(
    const result<T, Detail>& value,
    const result<Lo, Detail>& lo,
    Hi hi) -> result<detail::clamp_common_t<T, Lo, Hi>, Detail>
{
    if (!value.has_value()) {
        return std::unexpected(value.error());
    }

    if (!lo.has_value()) {
        return std::unexpected(lo.error());
    }

    const auto r = clamp(*value, *lo, hi);
    if (!r.has_value()) {
        return std::unexpected(r.error());
    }

    return *r;
}

/**
 * @brief Clamps a successful result value using a raw lower bound and a successful upper bound.
 *
 * Errors are propagated unchanged.
 *
 * @tparam T Value type.
 * @tparam Lo Lower-bound type.
 * @tparam Hi Upper-bound type.
 * @tparam Detail Error detail type.
 * @param value Value to clamp.
 * @param lo Lower bound.
 * @param hi Upper bound.
 * @return Clamped value.
 */
template<typename T, typename Lo, typename Hi, typename Detail>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<Lo> &&
             non_bool_arithmetic<Hi> &&
             requires { typename detail::clamp_common_t<T, Lo, Hi>; } &&
             non_bool_arithmetic<detail::clamp_common_t<T, Lo, Hi>>
[[nodiscard]] constexpr auto clamp(
    const result<T, Detail>& value,
    Lo lo,
    const result<Hi, Detail>& hi)
    -> result<detail::clamp_common_t<T, Lo, Hi>, Detail>
{
    if (!value.has_value()) {
        return std::unexpected(value.error());
    }

    if (!hi.has_value()) {
        return std::unexpected(hi.error());
    }

    const auto r = clamp(*value, lo, *hi);
    if (!r.has_value()) {
        return std::unexpected(r.error());
    }

    return *r;
}

/**
 * @brief Clamps a raw value using successful lower and upper bounds.
 *
 * Errors are propagated unchanged.
 *
 * @tparam T Value type.
 * @tparam Lo Lower-bound type.
 * @tparam Hi Upper-bound type.
 * @tparam Detail Error detail type.
 * @param value Value to clamp.
 * @param lo Lower bound.
 * @param hi Upper bound.
 * @return Clamped value.
 */
template<typename T, typename Lo, typename Hi, typename Detail>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<Lo> &&
             non_bool_arithmetic<Hi> &&
             requires { typename detail::clamp_common_t<T, Lo, Hi>; } &&
             non_bool_arithmetic<detail::clamp_common_t<T, Lo, Hi>>
[[nodiscard]] constexpr auto clamp(
    T value,
    const result<Lo, Detail>& lo,
    const result<Hi, Detail>& hi)
    -> result<detail::clamp_common_t<T, Lo, Hi>, Detail>
{
    if (!lo.has_value()) {
        return std::unexpected(lo.error());
    }

    if (!hi.has_value()) {
        return std::unexpected(hi.error());
    }

    const auto r = clamp(value, *lo, *hi);
    if (!r.has_value()) {
        return std::unexpected(r.error());
    }

    return *r;
}

/**
 * @brief Clamps a successful result value using successful lower and upper bounds.
 *
 * Errors are propagated unchanged.
 *
 * @tparam T Value type.
 * @tparam Lo Lower-bound type.
 * @tparam Hi Upper-bound type.
 * @tparam Detail Error detail type.
 * @param value Value to clamp.
 * @param lo Lower bound.
 * @param hi Upper bound.
 * @return Clamped value.
 */
template<typename T, typename Lo, typename Hi, typename Detail>
    requires non_bool_arithmetic<T> && non_bool_arithmetic<Lo> &&
             non_bool_arithmetic<Hi> &&
             requires { typename detail::clamp_common_t<T, Lo, Hi>; } &&
             non_bool_arithmetic<detail::clamp_common_t<T, Lo, Hi>>
[[nodiscard]] constexpr auto clamp(
    const result<T, Detail>& value,
    const result<Lo, Detail>& lo,
    const result<Hi, Detail>& hi)
    -> result<detail::clamp_common_t<T, Lo, Hi>, Detail>
{
    if (!value.has_value()) {
        return std::unexpected(value.error());
    }

    if (!lo.has_value()) {
        return std::unexpected(lo.error());
    }

    if (!hi.has_value()) {
        return std::unexpected(hi.error());
    }

    const auto r = clamp(*value, *lo, *hi);
    if (!r.has_value()) {
        return std::unexpected(r.error());
    }

    return *r;
}

} // namespace xer

#endif /* XER_BITS_MINMAX_H_INCLUDED_ */

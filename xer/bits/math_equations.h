/**
 * @file xer/bits/math_equations.h
 * @brief Real elementary equation helpers.
 */

#pragma once

#ifndef XER_BITS_MATH_EQUATIONS_H_INCLUDED_
#define XER_BITS_MATH_EQUATIONS_H_INCLUDED_

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <limits>
#include <numbers>
#include <optional>
#include <expected>

#include <xer/bits/error.h>

namespace xer {

namespace detail {

template<std::floating_point T>
[[nodiscard]] auto equation_tolerance(T lhs, T rhs) noexcept -> T
{
    const T scale = std::max({T{1}, std::abs(lhs), std::abs(rhs)});
    return scale * std::numeric_limits<T>::epsilon() * static_cast<T>(64);
}

template<std::floating_point T>
[[nodiscard]] auto equation_near(T lhs, T rhs) noexcept -> bool
{
    return std::abs(lhs - rhs) <= equation_tolerance(lhs, rhs);
}

template<std::floating_point T, std::size_t N>
auto append_real_root(std::array<std::optional<T>, N>& roots, T value) noexcept -> void
{
    for (const auto& root : roots) {
        if (root && equation_near(*root, value)) {
            return;
        }
    }

    for (auto& root : roots) {
        if (!root) {
            root = value;
            return;
        }
    }
}

template<std::floating_point T, std::size_t N>
auto sort_real_roots(std::array<std::optional<T>, N>& roots) noexcept -> void
{
    std::array<T, N> values{};
    std::size_t count = 0;

    for (const auto& root : roots) {
        if (root) {
            values[count++] = *root;
        }
    }

    std::sort(values.begin(), values.begin() + static_cast<std::ptrdiff_t>(count));

    for (std::size_t i = 0; i < N; ++i) {
        if (i < count) {
            roots[i] = values[i];
        }
        else {
            roots[i] = std::nullopt;
        }
    }
}

} // namespace detail


/**
 * @brief Computes the area of a triangle from its three side lengths.
 *
 * The function uses Heron's formula. The arguments are interpreted as side
 * lengths. Negative side lengths and side lengths that cannot form a triangle
 * are rejected with `error_t::invalid_argument`. Degenerate triangles are
 * accepted and return zero.
 *
 * @tparam T Floating-point type.
 * @param a First side length.
 * @param b Second side length.
 * @param c Third side length.
 * @return Triangle area, or an error if the arguments are invalid.
 */
template<std::floating_point T>
[[nodiscard]] auto heron(T a, T b, T c) -> result<T>
{
    if (a < T{} || b < T{} || c < T{}) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::array<T, 3> sides{a, b, c};
    std::sort(sides.begin(), sides.end());

    const T x = sides[0];
    const T y = sides[1];
    const T z = sides[2];

    if (z - y > x) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (z - y == x) {
        return T{};
    }

    const T term1 = z + (x + y);
    const T term2 = x - (z - y);
    const T term3 = x + (z - y);
    const T term4 = z + (y - x);

    return std::sqrt(term1 * term2 * term3 * term4) / static_cast<T>(4);
}

/**
 * @brief Solves a quadratic equation and returns its real roots.
 *
 * The equation is interpreted as:
 *
 * @code
 * a * x * x + b * x + c == 0
 * @endcode
 *
 * The coefficient @p a must not be zero. If the equation has no real root,
 * the function returns an array containing two empty optional values. If the
 * equation has one real root, including a double root, the first element holds
 * the root and the second element is empty. If the equation has two distinct
 * real roots, both elements hold roots in ascending order.
 *
 * @tparam T Floating-point type.
 * @param a Coefficient of x^2.
 * @param b Coefficient of x.
 * @param c Constant term.
 * @return Real roots, or an error if @p a is zero.
 */
template<std::floating_point T>
[[nodiscard]] auto quadratic(T a, T b, T c)
    -> result<std::array<std::optional<T>, 2>>
{
    if (a == T{}) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const T four = static_cast<T>(4);
    const T two = static_cast<T>(2);
    const T discriminant = b * b - four * a * c;

    if (discriminant < T{}) {
        return std::array<std::optional<T>, 2>{std::nullopt, std::nullopt};
    }

    if (discriminant == T{}) {
        return std::array<std::optional<T>, 2>{-b / (two * a), std::nullopt};
    }

    const T sqrt_d = std::sqrt(discriminant);
    const T q = -static_cast<T>(0.5) * (b + std::copysign(sqrt_d, b));

    T x1{};
    T x2{};

    if (q == T{}) {
        x1 = (-b - sqrt_d) / (two * a);
        x2 = (-b + sqrt_d) / (two * a);
    }
    else {
        x1 = q / a;
        x2 = c / q;
    }

    if (x2 < x1) {
        std::swap(x1, x2);
    }

    return std::array<std::optional<T>, 2>{x1, x2};
}

/**
 * @brief Solves a cubic equation and returns its real roots.
 *
 * The equation is interpreted as:
 *
 * @code
 * a * x * x * x + b * x * x + c * x + d == 0
 * @endcode
 *
 * The coefficient @p a must not be zero. The returned array contains distinct
 * real roots in ascending order, stored from the first element. Empty elements
 * are represented by `std::nullopt`.
 *
 * @tparam T Floating-point type.
 * @param a Coefficient of x^3.
 * @param b Coefficient of x^2.
 * @param c Coefficient of x.
 * @param d Constant term.
 * @return Distinct real roots, or an error if @p a is zero.
 */
template<std::floating_point T>
[[nodiscard]] auto cubic(T a, T b, T c, T d)
    -> result<std::array<std::optional<T>, 3>>
{
    if (a == T{}) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const T three = static_cast<T>(3);
    const T two = static_cast<T>(2);
    const T twenty_seven = static_cast<T>(27);

    const T aa = b / a;
    const T bb = c / a;
    const T cc = d / a;

    const T p = bb - aa * aa / three;
    const T q = two * aa * aa * aa / twenty_seven - aa * bb / three + cc;
    const T half_q = q / two;
    const T third_p = p / three;
    const T discriminant = half_q * half_q + third_p * third_p * third_p;
    const T offset = aa / three;

    std::array<std::optional<T>, 3> roots{std::nullopt, std::nullopt, std::nullopt};

    if (discriminant > T{}) {
        const T sqrt_d = std::sqrt(discriminant);
        const T u = std::cbrt(-half_q + sqrt_d);
        const T v = std::cbrt(-half_q - sqrt_d);
        roots[0] = u + v - offset;
        return roots;
    }

    if (discriminant == T{}) {
        const T u = std::cbrt(-half_q);
        detail::append_real_root(roots, two * u - offset);
        detail::append_real_root(roots, -u - offset);
        detail::sort_real_roots(roots);
        return roots;
    }

    const T r = two * std::sqrt(-p / three);
    T cos_arg = (three * q / (two * p)) * std::sqrt(-three / p);
    cos_arg = std::clamp(cos_arg, static_cast<T>(-1), static_cast<T>(1));

    const T theta = std::acos(cos_arg);
    const T tau = std::numbers::pi_v<T> * two;

    for (int k = 0; k < 3; ++k) {
        const T angle = (theta + tau * static_cast<T>(k)) / three;
        detail::append_real_root(roots, r * std::cos(angle) - offset);
    }

    detail::sort_real_roots(roots);
    return roots;
}

} // namespace xer

#endif /* XER_BITS_MATH_EQUATIONS_H_INCLUDED_ */

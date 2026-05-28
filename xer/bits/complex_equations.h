/**
 * @file xer/bits/complex_equations.h
 * @brief Complex elementary equation helpers.
 */

#pragma once

#ifndef XER_BITS_COMPLEX_EQUATIONS_H_INCLUDED_
#define XER_BITS_COMPLEX_EQUATIONS_H_INCLUDED_

#include <array>
#include <cmath>
#include <complex>
#include <concepts>
#include <expected>

#include <xer/bits/error.h>
#include <xer/bits/math_constants.h>

namespace xer {

namespace detail {

template<std::floating_point T>
[[nodiscard]] auto complex_cuberoot(std::complex<T> value) -> std::complex<T>
{
    return std::pow(value, static_cast<T>(1) / static_cast<T>(3));
}

} // namespace detail

/**
 * @brief Solves a quadratic equation and returns its complex roots.
 *
 * The equation is interpreted as:
 *
 * @code
 * a * x * x + b * x + c == 0
 * @endcode
 *
 * The coefficient @p a must not be zero. The returned array contains two roots
 * with multiplicity.
 *
 * @tparam T Floating-point type.
 * @param a Coefficient of x^2.
 * @param b Coefficient of x.
 * @param c Constant term.
 * @return Complex roots, or an error if @p a is zero.
 */
template<std::floating_point T>
[[nodiscard]] auto cquadratic(T a, T b, T c)
    -> result<std::array<std::complex<T>, 2>>
{
    if (a == T{}) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const std::complex<T> discriminant{b * b - static_cast<T>(4) * a * c, T{}};
    const auto sqrt_d = std::sqrt(discriminant);
    const std::complex<T> divisor{static_cast<T>(2) * a, T{}};

    return std::array<std::complex<T>, 2>{
        (std::complex<T>{-b, T{}} - sqrt_d) / divisor,
        (std::complex<T>{-b, T{}} + sqrt_d) / divisor,
    };
}

/**
 * @brief Solves a cubic equation and returns its complex roots.
 *
 * The equation is interpreted as:
 *
 * @code
 * a * x * x * x + b * x * x + c * x + d == 0
 * @endcode
 *
 * The coefficient @p a must not be zero. The returned array contains three
 * roots with multiplicity.
 *
 * @tparam T Floating-point type.
 * @param a Coefficient of x^3.
 * @param b Coefficient of x^2.
 * @param c Coefficient of x.
 * @param d Constant term.
 * @return Complex roots, or an error if @p a is zero.
 */
template<std::floating_point T>
[[nodiscard]] auto ccubic(T a, T b, T c, T d)
    -> result<std::array<std::complex<T>, 3>>
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
    const std::complex<T> half_q{q / two, T{}};
    const std::complex<T> third_p{p / three, T{}};
    const std::complex<T> delta = half_q * half_q + third_p * third_p * third_p;
    const auto sqrt_delta = std::sqrt(delta);

    std::complex<T> u = detail::complex_cuberoot(-half_q + sqrt_delta);
    std::complex<T> v{};

    if (std::abs(u) == T{}) {
        v = detail::complex_cuberoot(-half_q - sqrt_delta);
    }
    else {
        v = -third_p / u;
    }

    const std::complex<T> offset{aa / three, T{}};
    const auto omega = omega_v<T>;
    const auto omega2 = omega2_v<T>;

    return std::array<std::complex<T>, 3>{
        u + v - offset,
        omega * u + omega2 * v - offset,
        omega2 * u + omega * v - offset,
    };
}

} // namespace xer

#endif /* XER_BITS_COMPLEX_EQUATIONS_H_INCLUDED_ */

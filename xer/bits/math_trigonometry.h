/**
 * @file xer/bits/math_trigonometry.h
 * @brief Trigonometric helpers that use τrad values.
 */

#pragma once

#ifndef XER_BITS_MATH_TRIGONOMETRY_H_INCLUDED_
#define XER_BITS_MATH_TRIGONOMETRY_H_INCLUDED_

#include <cmath>
#include <concepts>

#include <xer/cyclic.h>

namespace xer {

/**
 * @brief Computes the sine of a τrad scalar angle.
 *
 * @tparam T Floating-point type.
 * @param theta Angle in τrad, where 1 is one full turn.
 * @return Sine of @p theta.
 */
template<std::floating_point T>
[[nodiscard]] auto sin(T theta) noexcept -> T
{
    return std::sin(to_rad(theta));
}

/**
 * @brief Computes the sine of a cyclic angle.
 *
 * @tparam T Floating-point type.
 * @param theta Cyclic angle in τrad.
 * @return Sine of @p theta.
 */
template<std::floating_point T>
[[nodiscard]] auto sin(cyclic<T> theta) noexcept -> T
{
    return std::sin(to_rad(theta));
}

/**
 * @brief Computes the cosine of a τrad scalar angle.
 *
 * @tparam T Floating-point type.
 * @param theta Angle in τrad, where 1 is one full turn.
 * @return Cosine of @p theta.
 */
template<std::floating_point T>
[[nodiscard]] auto cos(T theta) noexcept -> T
{
    return std::cos(to_rad(theta));
}

/**
 * @brief Computes the cosine of a cyclic angle.
 *
 * @tparam T Floating-point type.
 * @param theta Cyclic angle in τrad.
 * @return Cosine of @p theta.
 */
template<std::floating_point T>
[[nodiscard]] auto cos(cyclic<T> theta) noexcept -> T
{
    return std::cos(to_rad(theta));
}

/**
 * @brief Computes the tangent of a τrad scalar angle.
 *
 * @tparam T Floating-point type.
 * @param theta Angle in τrad, where 1 is one full turn.
 * @return Tangent of @p theta.
 */
template<std::floating_point T>
[[nodiscard]] auto tan(T theta) noexcept -> T
{
    return std::tan(to_rad(theta));
}

/**
 * @brief Computes the tangent of a cyclic angle.
 *
 * @tparam T Floating-point type.
 * @param theta Cyclic angle in τrad.
 * @return Tangent of @p theta.
 */
template<std::floating_point T>
[[nodiscard]] auto tan(cyclic<T> theta) noexcept -> T
{
    return std::tan(to_rad(theta));
}

/**
 * @brief Computes the arcsine and returns a τrad scalar angle.
 *
 * @tparam T Floating-point type.
 * @param value Sine value.
 * @return Angle in τrad.
 */
template<std::floating_point T>
[[nodiscard]] auto asin(T value) noexcept -> T
{
    return std::asin(value) / tau_v<T>;
}

/**
 * @brief Computes the arccosine and returns a τrad scalar angle.
 *
 * @tparam T Floating-point type.
 * @param value Cosine value.
 * @return Angle in τrad.
 */
template<std::floating_point T>
[[nodiscard]] auto acos(T value) noexcept -> T
{
    return std::acos(value) / tau_v<T>;
}

/**
 * @brief Computes the arctangent and returns a τrad scalar angle.
 *
 * @tparam T Floating-point type.
 * @param value Tangent value.
 * @return Angle in τrad.
 */
template<std::floating_point T>
[[nodiscard]] auto atan(T value) noexcept -> T
{
    return std::atan(value) / tau_v<T>;
}

/**
 * @brief Computes atan2 and returns a τrad scalar angle.
 *
 * @tparam T Floating-point type.
 * @param y Y component.
 * @param x X component.
 * @return Signed angle in τrad.
 */
template<std::floating_point T>
[[nodiscard]] auto atan2(T y, T x) noexcept -> T
{
    return std::atan2(y, x) / tau_v<T>;
}

} // namespace xer

#endif /* XER_BITS_MATH_TRIGONOMETRY_H_INCLUDED_ */

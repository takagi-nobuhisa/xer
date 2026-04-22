/**
 * @file xer/bits/cyclic.h
 * @brief Internal cyclic value type and conversion functions.
 */

#pragma once

#ifndef XER_BITS_CYCLIC_H_INCLUDED_
#define XER_BITS_CYCLIC_H_INCLUDED_

#include <cmath>
#include <concepts>
#include <limits>

#include <xer/bits/math_constants.h>

namespace xer {

/**
 * @brief Cyclic value normalized to the half-open interval [0, 1).
 *
 * @tparam T Floating-point storage type.
 */
template<std::floating_point T>
class cyclic {
public:
    /**
     * @brief Underlying floating-point type.
     */
    using value_type = T;

    /**
     * @brief Default tolerance for practical equality checks.
     */
    static constexpr T default_epsilon =
        std::numeric_limits<T>::epsilon() * static_cast<T>(16);

    /**
     * @brief Constructs the zero position.
     */
    constexpr cyclic() noexcept = default;

    /**
     * @brief Constructs a cyclic value from an arbitrary scalar.
     *
     * Non-finite values are normalized to zero so that the class invariant
     * `0 <= value() < 1` is preserved even for noexcept construction.
     *
     * @param value Source scalar value.
     */
    constexpr explicit cyclic(T value) noexcept : value_(normalize(value)) {}

    /**
     * @brief Returns the normalized internal representation.
     *
     * @return Normalized value in [0, 1).
     */
    [[nodiscard]] constexpr auto value() const noexcept -> T { return value_; }

    /**
     * @brief Returns the clockwise distance from this value to @p to.
     *
     * @param to Destination cyclic value.
     * @return Clockwise distance in [0, 1).
     */
    [[nodiscard]] constexpr auto cw(cyclic to) const noexcept -> T
    {
        return normalize(value_ - to.value_);
    }

    /**
     * @brief Returns the counterclockwise distance from this value to @p to.
     *
     * @param to Destination cyclic value.
     * @return Counterclockwise distance in [0, 1).
     */
    [[nodiscard]] constexpr auto ccw(cyclic to) const noexcept -> T
    {
        return normalize(to.value_ - value_);
    }

    /**
     * @brief Returns the shortest signed difference from this value to @p to.
     *
     * Positive results mean counterclockwise and negative results mean
     * clockwise. The returned value is normalized to the half-open interval
     * [-0.5, 0.5).
     *
     * @param to Destination cyclic value.
     * @return Shortest signed difference.
     */
    [[nodiscard]] constexpr auto diff(cyclic to) const noexcept -> T
    {
        T distance = ccw(to);
        if (distance >= static_cast<T>(0.5)) {
            distance -= static_cast<T>(1);
        }
        return distance;
    }

    /**
     * @brief Tests practical equality against @p to using the default epsilon.
     *
     * @param to Comparison target.
     * @return true if the cyclic values are practically equal.
     */
    [[nodiscard]] constexpr auto eq(cyclic to) const noexcept -> bool
    {
        return eq(to, default_epsilon);
    }

    /**
     * @brief Tests practical inequality against @p to using the default epsilon.
     *
     * @param to Comparison target.
     * @return true if the cyclic values are not practically equal.
     */
    [[nodiscard]] constexpr auto ne(cyclic to) const noexcept -> bool
    {
        return ne(to, default_epsilon);
    }

    /**
     * @brief Tests practical equality against @p to using an explicit epsilon.
     *
     * The comparison is based on the shortest cyclic difference rather than a
     * direct comparison of the representative values.
     *
     * @param to Comparison target.
     * @param epsilon Allowed absolute shortest-distance error.
     * @return true if the cyclic values are practically equal.
     */
    [[nodiscard]] constexpr auto eq(cyclic to, T epsilon) const noexcept -> bool
    {
        const T tolerance = std::abs(epsilon);
        return std::abs(diff(to)) <= tolerance;
    }

    /**
     * @brief Tests practical inequality against @p to using an explicit epsilon.
     *
     * @param to Comparison target.
     * @param epsilon Allowed absolute shortest-distance error.
     * @return true if the cyclic values are not practically equal.
     */
    [[nodiscard]] constexpr auto ne(cyclic to, T epsilon) const noexcept -> bool
    {
        return !eq(to, epsilon);
    }

    /**
     * @brief Returns this value unchanged.
     *
     * @return Same cyclic value.
     */
    [[nodiscard]] constexpr auto operator+() const noexcept -> cyclic
    {
        return *this;
    }

    /**
     * @brief Returns the additive inverse on the cycle.
     *
     * @return Inverted cyclic value.
     */
    [[nodiscard]] constexpr auto operator-() const noexcept -> cyclic
    {
        return cyclic(-value_);
    }

    /**
     * @brief Adds another cyclic value and renormalizes the result.
     *
     * @param value Value to add.
     * @return *this.
     */
    constexpr auto operator+=(cyclic value) noexcept -> cyclic&
    {
        value_ = normalize(value_ + value.value_);
        return *this;
    }

    /**
     * @brief Subtracts another cyclic value and renormalizes the result.
     *
     * @param value Value to subtract.
     * @return *this.
     */
    constexpr auto operator-=(cyclic value) noexcept -> cyclic&
    {
        value_ = normalize(value_ - value.value_);
        return *this;
    }

private:
    /**
     * @brief Normalizes an arbitrary scalar to the half-open interval [0, 1).
     *
     * Non-finite values are mapped to zero.
     *
     * @param value Source scalar value.
     * @return Normalized value.
     */
    [[nodiscard]] static constexpr auto normalize(T value) noexcept -> T
    {
        if (!std::isfinite(value)) {
            return static_cast<T>(0);
        }

        T normalized = value - std::floor(value);
        if (normalized >= static_cast<T>(1)) {
            normalized = static_cast<T>(0);
        }
        if (normalized < static_cast<T>(0)) {
            normalized += static_cast<T>(1);
        }
        return normalized;
    }

    T value_ = static_cast<T>(0);
};

/**
 * @brief Adds two cyclic values and renormalizes the result.
 *
 * @tparam T Floating-point storage type.
 * @param left Left operand.
 * @param right Right operand.
 * @return Sum on the cycle.
 */
template<std::floating_point T>
[[nodiscard]] constexpr auto operator+(cyclic<T> left, cyclic<T> right) noexcept
    -> cyclic<T>
{
    left += right;
    return left;
}

/**
 * @brief Subtracts two cyclic values and renormalizes the result.
 *
 * @tparam T Floating-point storage type.
 * @param left Left operand.
 * @param right Right operand.
 * @return Difference on the cycle.
 */
template<std::floating_point T>
[[nodiscard]] constexpr auto operator-(cyclic<T> left, cyclic<T> right) noexcept
    -> cyclic<T>
{
    left -= right;
    return left;
}

/**
 * @brief Converts degrees to a cyclic value.
 *
 * @tparam T Floating-point type.
 * @param value Degree value.
 * @return Corresponding cyclic value.
 */
template<std::floating_point T>
[[nodiscard]] constexpr auto from_degree(T value) noexcept -> cyclic<T>
{
    return cyclic<T>(value / static_cast<T>(360));
}

/**
 * @brief Converts a cyclic value to degrees.
 *
 * @tparam T Floating-point type.
 * @param value Cyclic value.
 * @return Degree value.
 */
template<std::floating_point T>
[[nodiscard]] constexpr auto to_degree(cyclic<T> value) noexcept -> T
{
    return value.value() * static_cast<T>(360);
}

/**
 * @brief Converts a turn-based scalar value to degrees.
 *
 * This overload is intended for values such as the return values of
 * `cw`, `ccw`, and `diff`, which use one turn as 1 but are not represented
 * as `cyclic<T>` objects.
 *
 * The input is not normalized. Negative values and values greater than or
 * equal to 1 are converted as-is.
 *
 * @tparam T Floating-point type.
 * @param value Turn-based scalar value.
 * @return Degree value.
 */
template<std::floating_point T>
[[nodiscard]] constexpr auto to_degree(T value) noexcept -> T
{
    return value * static_cast<T>(360);
}

/**
 * @brief Converts radians to a cyclic value.
 *
 * @tparam T Floating-point type.
 * @param value Radian value.
 * @return Corresponding cyclic value.
 */
template<std::floating_point T>
[[nodiscard]] constexpr auto from_radian(T value) noexcept -> cyclic<T>
{
    return cyclic<T>(value / (static_cast<T>(2) * pi_v<T>));
}

/**
 * @brief Converts a cyclic value to radians.
 *
 * @tparam T Floating-point type.
 * @param value Cyclic value.
 * @return Radian value.
 */
template<std::floating_point T>
[[nodiscard]] constexpr auto to_radian(cyclic<T> value) noexcept -> T
{
    return value.value() * (static_cast<T>(2) * pi_v<T>);
}

} // namespace xer

#endif /* XER_BITS_CYCLIC_H_INCLUDED_ */

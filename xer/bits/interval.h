/**
 * @file xer/bits/interval.h
 * @brief Internal bounded interval value type.
 */

#pragma once

#ifndef XER_BITS_INTERVAL_H_INCLUDED_
#define XER_BITS_INTERVAL_H_INCLUDED_

#include <compare>
#include <concepts>
#include <limits>
#include <stdexcept>

namespace xer {

/**
 * @brief Bounded floating-point value clamped to a closed interval.
 *
 * `interval<T, Min, Max>` stores a finite value that always satisfies
 * `Min <= value() <= Max`. Finite out-of-range inputs are clamped. NaN and
 * infinity are rejected by throwing `std::domain_error`.
 *
 * @tparam T Floating-point storage type.
 * @tparam Min Inclusive lower bound.
 * @tparam Max Inclusive upper bound.
 */
template<std::floating_point T, T Min = static_cast<T>(0), T Max = static_cast<T>(1)>
class interval {
    static_assert(Min == Min);
    static_assert(Max == Max);
    static_assert(Min >= std::numeric_limits<T>::lowest());
    static_assert(Min <= std::numeric_limits<T>::max());
    static_assert(Max >= std::numeric_limits<T>::lowest());
    static_assert(Max <= std::numeric_limits<T>::max());
    static_assert(Min < Max);

public:
    /**
     * @brief Underlying floating-point type.
     */
    using value_type = T;

    /**
     * @brief Inclusive lower bound.
     */
    static constexpr T min_value = Min;

    /**
     * @brief Inclusive upper bound.
     */
    static constexpr T max_value = Max;

    /**
     * @brief Constructs the lower-bound value.
     */
    constexpr interval() noexcept = default;

    /**
     * @brief Constructs an interval value from a raw scalar.
     *
     * Finite values are clamped into `[Min, Max]`. NaN and infinity throw
     * `std::domain_error`.
     *
     * @param value Source scalar value.
     */
    constexpr explicit interval(T value) : value_(normalize(value)) {}

    /**
     * @brief Returns the stored value.
     *
     * @return Finite value in `[Min, Max]`.
     */
    [[nodiscard]] constexpr auto value() const noexcept -> T { return value_; }

    /**
     * @brief Assigns a raw scalar value.
     *
     * Finite values are clamped into `[Min, Max]`. NaN and infinity throw
     * `std::domain_error`.
     *
     * @param value Source scalar value.
     */
    constexpr auto assign(T value) -> void { value_ = normalize(value); }

    /**
     * @brief Assigns a raw scalar value.
     *
     * @param value Source scalar value.
     * @return *this.
     */
    constexpr auto operator=(T value) -> interval&
    {
        assign(value);
        return *this;
    }

    /**
     * @brief Returns the relative position in the interval as `[0, 1]`.
     *
     * @return Relative position.
     */
    [[nodiscard]] constexpr auto ratio() const noexcept -> T
    {
        return (value_ - Min) / (Max - Min);
    }

    /**
     * @brief Creates an interval value from a relative position.
     *
     * Finite ratio values are clamped into `[0, 1]`. NaN and infinity throw
     * `std::domain_error`.
     *
     * @param ratio Relative position.
     * @return Corresponding interval value.
     */
    [[nodiscard]] static constexpr auto from_ratio(T ratio) -> interval
    {
        const T clamped_ratio = clamp_unit(validate(ratio));
        return interval(Min + clamped_ratio * (Max - Min));
    }

    /**
     * @brief Returns this value unchanged.
     *
     * @return Same interval value.
     */
    [[nodiscard]] constexpr auto operator+() const noexcept -> interval
    {
        return *this;
    }

    /**
     * @brief Returns the negated value clamped to the same interval.
     *
     * @return Negated interval value.
     */
    [[nodiscard]] constexpr auto operator-() const -> interval
    {
        return interval(-value_);
    }

    /**
     * @brief Adds another interval value.
     *
     * @param value Value to add.
     * @return *this.
     */
    constexpr auto operator+=(interval value) -> interval&
    {
        value_ = normalize(value_ + value.value_);
        return *this;
    }

    /**
     * @brief Subtracts another interval value.
     *
     * @param value Value to subtract.
     * @return *this.
     */
    constexpr auto operator-=(interval value) -> interval&
    {
        value_ = normalize(value_ - value.value_);
        return *this;
    }

    /**
     * @brief Multiplies by another interval value.
     *
     * @param value Value to multiply by.
     * @return *this.
     */
    constexpr auto operator*=(interval value) -> interval&
    {
        value_ = normalize(value_ * value.value_);
        return *this;
    }

    /**
     * @brief Divides by another interval value.
     *
     * Division by zero throws `std::domain_error`.
     *
     * @param value Divisor value.
     * @return *this.
     */
    constexpr auto operator/=(interval value) -> interval&
    {
        if (value.value_ == static_cast<T>(0)) {
            std::__throw_domain_error("xer::interval division by zero");
        }
        value_ = normalize(value_ / value.value_);
        return *this;
    }

    /**
     * @brief Adds a right-hand scalar value.
     *
     * @param value Value to add.
     * @return *this.
     */
    constexpr auto operator+=(T value) -> interval&
    {
        value_ = normalize(value_ + validate(value));
        return *this;
    }

    /**
     * @brief Subtracts a right-hand scalar value.
     *
     * @param value Value to subtract.
     * @return *this.
     */
    constexpr auto operator-=(T value) -> interval&
    {
        value_ = normalize(value_ - validate(value));
        return *this;
    }

    /**
     * @brief Multiplies by a right-hand scalar value.
     *
     * @param value Value to multiply by.
     * @return *this.
     */
    constexpr auto operator*=(T value) -> interval&
    {
        value_ = normalize(value_ * validate(value));
        return *this;
    }

    /**
     * @brief Divides by a right-hand scalar value.
     *
     * Division by zero throws `std::domain_error`.
     *
     * @param value Divisor value.
     * @return *this.
     */
    constexpr auto operator/=(T value) -> interval&
    {
        const T divisor = validate(value);
        if (divisor == static_cast<T>(0)) {
            std::__throw_domain_error("xer::interval division by zero");
        }
        value_ = normalize(value_ / divisor);
        return *this;
    }

    /**
     * @brief Compares two interval values for equality.
     *
     * @param right Comparison target.
     * @return true if the stored values are equal.
     */
    [[nodiscard]] constexpr auto operator==(interval right) const noexcept -> bool
    {
        return value_ == right.value_;
    }

    /**
     * @brief Compares two interval values by their stored scalar values.
     *
     * @param right Comparison target.
     * @return Strong ordering result of the stored values.
     */
    [[nodiscard]] constexpr auto operator<=>(interval right) const noexcept
        -> std::strong_ordering
    {
        if (value_ < right.value_) {
            return std::strong_ordering::less;
        }
        if (right.value_ < value_) {
            return std::strong_ordering::greater;
        }
        return std::strong_ordering::equal;
    }

private:
    /**
     * @brief Validates that a scalar is finite.
     *
     * @param value Source scalar value.
     * @return The same value.
     */
    [[nodiscard]] static constexpr auto validate(T value) -> T
    {
        if (!(value == value) ||
            value < std::numeric_limits<T>::lowest() ||
            std::numeric_limits<T>::max() < value) {
            std::__throw_domain_error(
                "xer::interval cannot store NaN or infinity");
        }
        return value;
    }

    /**
     * @brief Clamps a finite scalar to the interval bounds.
     *
     * @param value Finite source scalar value.
     * @return Clamped value.
     */
    [[nodiscard]] static constexpr auto clamp(T value) noexcept -> T
    {
        if (value < Min) {
            return Min;
        }
        if (Max < value) {
            return Max;
        }
        return value;
    }

    /**
     * @brief Clamps a finite scalar to `[0, 1]`.
     *
     * @param value Finite source scalar value.
     * @return Clamped value.
     */
    [[nodiscard]] static constexpr auto clamp_unit(T value) noexcept -> T
    {
        if (value < static_cast<T>(0)) {
            return static_cast<T>(0);
        }
        if (static_cast<T>(1) < value) {
            return static_cast<T>(1);
        }
        return value;
    }

    /**
     * @brief Validates and clamps a scalar to the interval bounds.
     *
     * @param value Source scalar value.
     * @return Validated and clamped value.
     */
    [[nodiscard]] static constexpr auto normalize(T value) -> T
    {
        return clamp(validate(value));
    }

    T value_ = Min;
};

/**
 * @brief Adds two interval values and clamps the result.
 */
template<std::floating_point T, T Min, T Max>
[[nodiscard]] constexpr auto operator+(
    interval<T, Min, Max> left,
    interval<T, Min, Max> right) -> interval<T, Min, Max>
{
    left += right;
    return left;
}

/**
 * @brief Subtracts two interval values and clamps the result.
 */
template<std::floating_point T, T Min, T Max>
[[nodiscard]] constexpr auto operator-(
    interval<T, Min, Max> left,
    interval<T, Min, Max> right) -> interval<T, Min, Max>
{
    left -= right;
    return left;
}

/**
 * @brief Multiplies two interval values and clamps the result.
 */
template<std::floating_point T, T Min, T Max>
[[nodiscard]] constexpr auto operator*(
    interval<T, Min, Max> left,
    interval<T, Min, Max> right) -> interval<T, Min, Max>
{
    left *= right;
    return left;
}

/**
 * @brief Divides two interval values and clamps the result.
 */
template<std::floating_point T, T Min, T Max>
[[nodiscard]] constexpr auto operator/(
    interval<T, Min, Max> left,
    interval<T, Min, Max> right) -> interval<T, Min, Max>
{
    left /= right;
    return left;
}

/**
 * @brief Adds a right-hand scalar value and clamps the result.
 */
template<std::floating_point T, T Min, T Max, class U>
    requires std::convertible_to<U, T>
[[nodiscard]] constexpr auto operator+(interval<T, Min, Max> left, U right)
    -> interval<T, Min, Max>
{
    left += static_cast<T>(right);
    return left;
}

/**
 * @brief Subtracts a right-hand scalar value and clamps the result.
 */
template<std::floating_point T, T Min, T Max, class U>
    requires std::convertible_to<U, T>
[[nodiscard]] constexpr auto operator-(interval<T, Min, Max> left, U right)
    -> interval<T, Min, Max>
{
    left -= static_cast<T>(right);
    return left;
}

/**
 * @brief Multiplies by a right-hand scalar value and clamps the result.
 */
template<std::floating_point T, T Min, T Max, class U>
    requires std::convertible_to<U, T>
[[nodiscard]] constexpr auto operator*(interval<T, Min, Max> left, U right)
    -> interval<T, Min, Max>
{
    left *= static_cast<T>(right);
    return left;
}

/**
 * @brief Divides by a right-hand scalar value and clamps the result.
 */
template<std::floating_point T, T Min, T Max, class U>
    requires std::convertible_to<U, T>
[[nodiscard]] constexpr auto operator/(interval<T, Min, Max> left, U right)
    -> interval<T, Min, Max>
{
    left /= static_cast<T>(right);
    return left;
}

/**
 * @brief Multiplies a scalar value by an interval value and clamps the result.
 */
template<class U, std::floating_point T, T Min, T Max>
    requires std::convertible_to<U, T>
[[nodiscard]] constexpr auto operator*(U left, interval<T, Min, Max> right)
    -> interval<T, Min, Max>
{
    right *= static_cast<T>(left);
    return right;
}

} // namespace xer

#endif /* XER_BITS_INTERVAL_H_INCLUDED_ */

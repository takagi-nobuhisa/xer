/**
 * @file xer/bits/quantity.h
 * @brief Internal physical quantity and unit support.
 */

#pragma once

#ifndef XER_BITS_QUANTITY_H_INCLUDED_
#define XER_BITS_QUANTITY_H_INCLUDED_

#include <concepts>
#include <cstdint>
#include <ratio>
#include <type_traits>

#include <xer/bits/math_constants.h>

namespace xer {

/**
 * @brief Dimension represented by exponents of base units.
 *
 * The template parameters correspond to the MKSA base dimensions used by the
 * initial XER quantity design.
 *
 * @tparam L Exponent of length.
 * @tparam M Exponent of mass.
 * @tparam Ti Exponent of time.
 * @tparam I Exponent of electric current.
 */
template<int L, int M, int Ti, int I>
struct dimension {
    static constexpr int length = L;
    static constexpr int mass = M;
    static constexpr int time = Ti;
    static constexpr int current = I;
};

/**
 * @brief Dimensionless quantity alias.
 */
using dimensionless = dimension<0, 0, 0, 0>;

namespace detail {

template<class Dim>
struct is_dimension : std::false_type {};

template<int L, int M, int Ti, int I>
struct is_dimension<dimension<L, M, Ti, I>> : std::true_type {};

template<class Dim>
inline constexpr bool is_dimension_v = is_dimension<Dim>::value;

template<class LeftDim, class RightDim>
struct dimension_add;

template<int LL, int LM, int LT, int LI, int RL, int RM, int RT, int RI>
struct dimension_add<dimension<LL, LM, LT, LI>, dimension<RL, RM, RT, RI>> {
    using type = dimension<LL + RL, LM + RM, LT + RT, LI + RI>;
};

template<class LeftDim, class RightDim>
using dimension_add_t = typename dimension_add<LeftDim, RightDim>::type;

template<class LeftDim, class RightDim>
struct dimension_subtract;

template<int LL, int LM, int LT, int LI, int RL, int RM, int RT, int RI>
struct dimension_subtract<dimension<LL, LM, LT, LI>, dimension<RL, RM, RT, RI>> {
    using type = dimension<LL - RL, LM - RM, LT - RT, LI - RI>;
};

template<class LeftDim, class RightDim>
using dimension_subtract_t = typename dimension_subtract<LeftDim, RightDim>::type;

template<class Scale>
struct scale_value;

template<std::intmax_t Num, std::intmax_t Den>
struct scale_value<std::ratio<Num, Den>> {
    template<std::floating_point T>
    static constexpr auto get() noexcept -> T
    {
        return static_cast<T>(Num) / static_cast<T>(Den);
    }
};

template<long double Value>
struct floating_scale {
    static constexpr long double value = Value;
};

template<long double Value>
struct scale_value<floating_scale<Value>> {
    template<std::floating_point T>
    static constexpr auto get() noexcept -> T
    {
        return static_cast<T>(Value);
    }
};

template<class Scale>
concept scale_type = requires {
    { scale_value<Scale>::template get<long double>() } -> std::convertible_to<long double>;
};

template<class LeftScale, class RightScale>
struct scale_multiply {
    using type = floating_scale<scale_value<LeftScale>::template get<long double>() *
                                scale_value<RightScale>::template get<long double>()>;
};

template<std::intmax_t LNum,
         std::intmax_t LDen,
         std::intmax_t RNum,
         std::intmax_t RDen>
struct scale_multiply<std::ratio<LNum, LDen>, std::ratio<RNum, RDen>> {
    using type = typename std::ratio_multiply<std::ratio<LNum, LDen>,
                                              std::ratio<RNum, RDen>>::type;
};

template<class LeftScale, class RightScale>
using scale_multiply_t = typename scale_multiply<LeftScale, RightScale>::type;

template<class LeftScale, class RightScale>
struct scale_divide {
    using type = floating_scale<scale_value<LeftScale>::template get<long double>() /
                                scale_value<RightScale>::template get<long double>()>;
};

template<std::intmax_t LNum,
         std::intmax_t LDen,
         std::intmax_t RNum,
         std::intmax_t RDen>
struct scale_divide<std::ratio<LNum, LDen>, std::ratio<RNum, RDen>> {
    using type = typename std::ratio_divide<std::ratio<LNum, LDen>,
                                            std::ratio<RNum, RDen>>::type;
};

template<class LeftScale, class RightScale>
using scale_divide_t = typename scale_divide<LeftScale, RightScale>::type;

template<class Dim>
inline constexpr bool is_dimensionless_v =
    std::same_as<Dim, dimensionless>;

} // namespace detail

/**
 * @brief Unit represented only by dimension and scale type.
 *
 * The class intentionally stores no runtime state. It exists so that unit
 * expressions remain strongly typed while staying lightweight.
 *
 * @tparam Dim Dimension type.
 * @tparam Scale Scale relative to the base unit system.
 */
template<class Dim, class Scale = std::ratio<1>>
    requires(detail::is_dimension_v<Dim> && detail::scale_type<Scale>)
class unit {
public:
    using dimension_type = Dim;
    using scale_type = Scale;

    constexpr unit() noexcept = default;

    /**
     * @brief Returns the scale factor relative to the base unit system.
     *
     * @tparam T Floating-point type used for the result.
     * @return Scale factor as a floating-point value.
     */
    template<std::floating_point T = long double>
    [[nodiscard]] static constexpr auto scale() noexcept -> T
    {
        return detail::scale_value<Scale>::template get<T>();
    }
};

/**
 * @brief Quantity value normalized to the base unit system.
 *
 * @tparam T Floating-point storage type.
 * @tparam Dim Dimension type.
 */
template<std::floating_point T, class Dim>
    requires(detail::is_dimension_v<Dim>)
class quantity {
public:
    using value_type = T;
    using dimension_type = Dim;

    /**
     * @brief Constructs the zero quantity.
     */
    constexpr quantity() noexcept = default;

    /**
     * @brief Constructs a quantity from a base-unit value.
     *
     * The argument is interpreted as already normalized to the base unit
     * system.
     *
     * @param value Base-unit value.
     */
    constexpr explicit quantity(T value) noexcept : value_(value) {}

    /**
     * @brief Returns the stored value in base units.
     *
     * @return Base-unit value.
     */
    [[nodiscard]] constexpr auto value() const noexcept -> T { return value_; }

    /**
     * @brief Returns the quantity expressed in the specified unit.
     *
     * @tparam Scale Unit scale type.
     * @param unused Unit tag used only for type selection.
     * @return Value converted from base units into the requested unit.
     */
    template<class Scale>
    [[nodiscard]] constexpr auto value(unit<Dim, Scale> unused = {}) const noexcept
        -> T
    {
        (void)unused;
        return value_ / unit<Dim, Scale>::template scale<T>();
    }

    /**
     * @brief Adds another quantity of the same dimension.
     *
     * @param other Quantity to add.
     * @return *this.
     */
    constexpr auto operator+=(quantity other) noexcept -> quantity&
    {
        value_ += other.value_;
        return *this;
    }

    /**
     * @brief Subtracts another quantity of the same dimension.
     *
     * @param other Quantity to subtract.
     * @return *this.
     */
    constexpr auto operator-=(quantity other) noexcept -> quantity&
    {
        value_ -= other.value_;
        return *this;
    }

    /**
     * @brief Multiplies the quantity by a scalar.
     *
     * @param scalar Scalar multiplier.
     * @return *this.
     */
    constexpr auto operator*=(T scalar) noexcept -> quantity&
    {
        value_ *= scalar;
        return *this;
    }

    /**
     * @brief Divides the quantity by a scalar.
     *
     * @param scalar Scalar divisor.
     * @return *this.
     */
    constexpr auto operator/=(T scalar) noexcept -> quantity&
    {
        value_ /= scalar;
        return *this;
    }

    /**
     * @brief Explicit conversion to the raw scalar for dimensionless values.
     *
     * @return Stored value.
     */
    [[nodiscard]] constexpr explicit operator T() const noexcept
        requires(detail::is_dimensionless_v<Dim>)
    {
        return value_;
    }

private:
    T value_ = static_cast<T>(0);
};

/**
 * @brief Adds quantities of the same dimension.
 */
template<std::floating_point T, class Dim>
[[nodiscard]] constexpr auto operator+(quantity<T, Dim> left, quantity<T, Dim> right) noexcept
    -> quantity<T, Dim>
{
    left += right;
    return left;
}

/**
 * @brief Subtracts quantities of the same dimension.
 */
template<std::floating_point T, class Dim>
[[nodiscard]] constexpr auto operator-(quantity<T, Dim> left, quantity<T, Dim> right) noexcept
    -> quantity<T, Dim>
{
    left -= right;
    return left;
}

/**
 * @brief Returns the quantity unchanged.
 */
template<std::floating_point T, class Dim>
[[nodiscard]] constexpr auto operator+(quantity<T, Dim> value) noexcept -> quantity<T, Dim>
{
    return value;
}

/**
 * @brief Negates a quantity.
 */
template<std::floating_point T, class Dim>
[[nodiscard]] constexpr auto operator-(quantity<T, Dim> value) noexcept -> quantity<T, Dim>
{
    return quantity<T, Dim>(-value.value());
}

/**
 * @brief Multiplies a scalar by a unit to create a quantity.
 */
template<std::floating_point T, class Dim, class Scale>
[[nodiscard]] constexpr auto operator*(T value, unit<Dim, Scale> unused) noexcept
    -> quantity<T, Dim>
{
    (void)unused;
    return quantity<T, Dim>(value * unit<Dim, Scale>::template scale<T>());
}

/**
 * @brief Multiplies a unit by a scalar to create a quantity.
 */
template<std::floating_point T, class Dim, class Scale>
[[nodiscard]] constexpr auto operator*(unit<Dim, Scale> unused, T value) noexcept
    -> quantity<T, Dim>
{
    return value * unused;
}

/**
 * @brief Multiplies two quantities and combines their dimensions.
 */
template<std::floating_point T, class LeftDim, class RightDim>
[[nodiscard]] constexpr auto operator*(quantity<T, LeftDim> left,
                                       quantity<T, RightDim> right) noexcept
    -> quantity<T, detail::dimension_add_t<LeftDim, RightDim>>
{
    using result_dim = detail::dimension_add_t<LeftDim, RightDim>;
    return quantity<T, result_dim>(left.value() * right.value());
}

/**
 * @brief Divides two quantities and subtracts their dimensions.
 */
template<std::floating_point T, class LeftDim, class RightDim>
[[nodiscard]] constexpr auto operator/(quantity<T, LeftDim> left,
                                       quantity<T, RightDim> right) noexcept
    -> quantity<T, detail::dimension_subtract_t<LeftDim, RightDim>>
{
    using result_dim = detail::dimension_subtract_t<LeftDim, RightDim>;
    return quantity<T, result_dim>(left.value() / right.value());
}

/**
 * @brief Multiplies a quantity by a scalar.
 */
template<std::floating_point T, class Dim>
[[nodiscard]] constexpr auto operator*(quantity<T, Dim> value, T scalar) noexcept
    -> quantity<T, Dim>
{
    value *= scalar;
    return value;
}

/**
 * @brief Multiplies a scalar by a quantity.
 */
template<std::floating_point T, class Dim>
[[nodiscard]] constexpr auto operator*(T scalar, quantity<T, Dim> value) noexcept
    -> quantity<T, Dim>
{
    return value * scalar;
}

/**
 * @brief Divides a quantity by a scalar.
 */
template<std::floating_point T, class Dim>
[[nodiscard]] constexpr auto operator/(quantity<T, Dim> value, T scalar) noexcept
    -> quantity<T, Dim>
{
    value /= scalar;
    return value;
}

/**
 * @brief Multiplies a quantity by a unit and combines dimensions.
 */
template<std::floating_point T, class LeftDim, class RightDim, class Scale>
[[nodiscard]] constexpr auto operator*(quantity<T, LeftDim> value,
                                       unit<RightDim, Scale> unused) noexcept
    -> quantity<T, detail::dimension_add_t<LeftDim, RightDim>>
{
    using result_dim = detail::dimension_add_t<LeftDim, RightDim>;
    (void)unused;
    return quantity<T, result_dim>(
        value.value() * unit<RightDim, Scale>::template scale<T>());
}

/**
 * @brief Multiplies a unit by a quantity and combines dimensions.
 */
template<std::floating_point T, class LeftDim, class Scale, class RightDim>
[[nodiscard]] constexpr auto operator*(unit<LeftDim, Scale> unused,
                                       quantity<T, RightDim> value) noexcept
    -> quantity<T, detail::dimension_add_t<LeftDim, RightDim>>
{
    return value * unused;
}

/**
 * @brief Divides a quantity by a unit and subtracts dimensions.
 */
template<std::floating_point T, class LeftDim, class RightDim, class Scale>
[[nodiscard]] constexpr auto operator/(quantity<T, LeftDim> value,
                                       unit<RightDim, Scale> unused) noexcept
    -> quantity<T, detail::dimension_subtract_t<LeftDim, RightDim>>
{
    using result_dim = detail::dimension_subtract_t<LeftDim, RightDim>;
    (void)unused;
    return quantity<T, result_dim>(
        value.value() / unit<RightDim, Scale>::template scale<T>());
}

/**
 * @brief Divides a quantity by a unit of the same dimension and returns the
 *        numeric value in that unit.
 */
template<std::floating_point T, class Dim, class Scale>
[[nodiscard]] constexpr auto value_in(quantity<T, Dim> value, unit<Dim, Scale> unused = {}) noexcept
    -> T
{
    (void)unused;
    return value.value(unit<Dim, Scale>{});
}

/**
 * @brief Compares quantities of the same dimension for equality.
 */
template<std::floating_point T, class Dim>
[[nodiscard]] constexpr auto operator==(quantity<T, Dim> left,
                                        quantity<T, Dim> right) noexcept -> bool
{
    return left.value() == right.value();
}

/**
 * @brief Three-way style ordering helpers are intentionally not provided.
 */
template<std::floating_point T, class Dim>
[[nodiscard]] constexpr auto operator!=(quantity<T, Dim> left,
                                        quantity<T, Dim> right) noexcept -> bool
{
    return !(left == right);
}

/**
 * @brief Less-than comparison for quantities of the same dimension.
 */
template<std::floating_point T, class Dim>
[[nodiscard]] constexpr auto operator<(quantity<T, Dim> left,
                                       quantity<T, Dim> right) noexcept -> bool
{
    return left.value() < right.value();
}

/**
 * @brief Less-than-or-equal comparison for quantities of the same dimension.
 */
template<std::floating_point T, class Dim>
[[nodiscard]] constexpr auto operator<=(quantity<T, Dim> left,
                                        quantity<T, Dim> right) noexcept -> bool
{
    return left.value() <= right.value();
}

/**
 * @brief Greater-than comparison for quantities of the same dimension.
 */
template<std::floating_point T, class Dim>
[[nodiscard]] constexpr auto operator>(quantity<T, Dim> left,
                                       quantity<T, Dim> right) noexcept -> bool
{
    return left.value() > right.value();
}

/**
 * @brief Greater-than-or-equal comparison for quantities of the same dimension.
 */
template<std::floating_point T, class Dim>
[[nodiscard]] constexpr auto operator>=(quantity<T, Dim> left,
                                        quantity<T, Dim> right) noexcept -> bool
{
    return left.value() >= right.value();
}

/**
 * @brief Multiplies two units and combines their dimensions and scales.
 */
template<class LeftDim, class LeftScale, class RightDim, class RightScale>
[[nodiscard]] constexpr auto operator*(unit<LeftDim, LeftScale> left,
                                       unit<RightDim, RightScale> right) noexcept
    -> unit<detail::dimension_add_t<LeftDim, RightDim>,
            detail::scale_multiply_t<LeftScale, RightScale>>
{
    (void)left;
    (void)right;
    return {};
}

/**
 * @brief Divides two units and subtracts their dimensions and scales.
 */
template<class LeftDim, class LeftScale, class RightDim, class RightScale>
[[nodiscard]] constexpr auto operator/(unit<LeftDim, LeftScale> left,
                                       unit<RightDim, RightScale> right) noexcept
    -> unit<detail::dimension_subtract_t<LeftDim, RightDim>,
            detail::scale_divide_t<LeftScale, RightScale>>
{
    (void)left;
    (void)right;
    return {};
}

namespace units {

using length_dim = dimension<1, 0, 0, 0>;
using mass_dim = dimension<0, 1, 0, 0>;
using time_dim = dimension<0, 0, 1, 0>;
using current_dim = dimension<0, 0, 0, 1>;

inline constexpr unit<length_dim> m{};
inline constexpr unit<mass_dim> kg{};
inline constexpr unit<time_dim> sec{};
inline constexpr unit<current_dim> A{};

inline constexpr unit<length_dim, std::milli> mm{};
inline constexpr unit<length_dim, std::centi> cm{};
inline constexpr unit<length_dim, std::kilo> km{};
inline constexpr unit<length_dim, std::micro> microm{};
inline constexpr unit<length_dim, std::nano> nm{};
inline constexpr unit<length_dim, std::micro> μm{};

inline constexpr unit<mass_dim, std::ratio<1, 1000>> g{};
inline constexpr unit<mass_dim, std::ratio<1, 1000000>> mg{};

inline constexpr unit<time_dim, std::nano> nsec{};
inline constexpr unit<time_dim, std::micro> microsec{};
inline constexpr unit<time_dim, std::milli> msec{};
inline constexpr unit<time_dim, std::micro> μsec{};

inline constexpr unit<current_dim, std::milli> mA{};

inline constexpr auto Hz = unit<dimension<0, 0, -1, 0>>{};
inline constexpr auto kHz = unit<dimension<0, 0, -1, 0>, std::kilo>{};
inline constexpr auto GHz = unit<dimension<0, 0, -1, 0>, std::giga>{};

inline constexpr auto N = kg * m / (sec * sec);
inline constexpr auto J = N * m;
inline constexpr auto W = J / sec;
inline constexpr auto V = W / A;
inline constexpr auto Pa = N / (m * m);
inline constexpr auto hPa = unit<dimension<-1, 1, -2, 0>, std::hecto>{};

inline constexpr auto ha = unit<dimension<2, 0, 0, 0>, std::ratio<10000>>{};
inline constexpr auto mL = unit<dimension<3, 0, 0, 0>, std::ratio<1, 1000000>>{};
inline constexpr auto dL = unit<dimension<3, 0, 0, 0>, std::ratio<1, 10000>>{};
inline constexpr auto L = unit<dimension<3, 0, 0, 0>, std::ratio<1, 1000>>{};
inline constexpr auto kL = unit<dimension<3, 0, 0, 0>>{};
inline constexpr auto cc = mL;

inline constexpr auto cal = unit<dimension<2, 1, -2, 0>, std::ratio<523, 125>>{};
inline constexpr auto kcal = unit<dimension<2, 1, -2, 0>, std::ratio<4184>>{};

inline constexpr unit<dimensionless> taurad{};
inline constexpr unit<dimensionless> τrad{};
inline constexpr unit<dimensionless,
                      detail::floating_scale<1.0L / tau_v<long double>>> rad{};

} // namespace units

} // namespace xer

#endif /* XER_BITS_QUANTITY_H_INCLUDED_ */

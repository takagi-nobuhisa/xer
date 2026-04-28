/**
 * @file xer/bits/color.h
 * @brief Internal color-system value types and conversions.
 */

#pragma once

#ifndef XER_BITS_COLOR_H_INCLUDED_
#define XER_BITS_COLOR_H_INCLUDED_

#include <cmath>
#include <concepts>

#include <xer/cyclic.h>
#include <xer/interval.h>

namespace xer {

/**
 * @brief RGB color with normalized sRGB components.
 *
 * `basic_rgb<T>` stores ordinary nonlinear sRGB component values. When it is
 * converted to or from XYZ, the conversion assumes sRGB with the D65 white
 * point.
 *
 * @tparam T Floating-point storage type.
 */
template<std::floating_point T>
struct basic_rgb {
    /**
     * @brief Underlying floating-point type.
     */
    using value_type = T;

    /**
     * @brief Normalized component type.
     */
    using component_type = interval<T>;

    /**
     * @brief Red component in [0, 1].
     */
    component_type r;

    /**
     * @brief Green component in [0, 1].
     */
    component_type g;

    /**
     * @brief Blue component in [0, 1].
     */
    component_type b;

    /**
     * @brief Constructs black.
     */
    constexpr basic_rgb() noexcept = default;

    /**
     * @brief Constructs an RGB color from normalized components.
     *
     * Finite out-of-range values are clamped by `interval<T>`.
     *
     * @param red Red component.
     * @param green Green component.
     * @param blue Blue component.
     */
    constexpr basic_rgb(T red, T green, T blue)
        : r(red), g(green), b(blue)
    {
    }

    /**
     * @brief Constructs an RGB color from interval components.
     *
     * @param red Red component.
     * @param green Green component.
     * @param blue Blue component.
     */
    constexpr basic_rgb(
        component_type red,
        component_type green,
        component_type blue) noexcept
        : r(red), g(green), b(blue)
    {
    }
};

/**
 * @brief CMY color with normalized components.
 *
 * This is a simple normalized complement model, not a device-specific printer
 * color-management model.
 *
 * @tparam T Floating-point storage type.
 */
template<std::floating_point T>
struct basic_cmy {
    /**
     * @brief Underlying floating-point type.
     */
    using value_type = T;

    /**
     * @brief Normalized component type.
     */
    using component_type = interval<T>;

    /**
     * @brief Cyan component in [0, 1].
     */
    component_type c;

    /**
     * @brief Magenta component in [0, 1].
     */
    component_type m;

    /**
     * @brief Yellow component in [0, 1].
     */
    component_type y;

    /**
     * @brief Constructs zero cyan, zero magenta, and zero yellow.
     */
    constexpr basic_cmy() noexcept = default;

    /**
     * @brief Constructs a CMY color from normalized components.
     *
     * Finite out-of-range values are clamped by `interval<T>`.
     *
     * @param cyan Cyan component.
     * @param magenta Magenta component.
     * @param yellow Yellow component.
     */
    constexpr basic_cmy(T cyan, T magenta, T yellow)
        : c(cyan), m(magenta), y(yellow)
    {
    }

    /**
     * @brief Constructs a CMY color from interval components.
     *
     * @param cyan Cyan component.
     * @param magenta Magenta component.
     * @param yellow Yellow component.
     */
    constexpr basic_cmy(
        component_type cyan,
        component_type magenta,
        component_type yellow) noexcept
        : c(cyan), m(magenta), y(yellow)
    {
    }
};

/**
 * @brief HSV color with cyclic hue and normalized saturation/value.
 *
 * @tparam T Floating-point storage type.
 */
template<std::floating_point T>
struct basic_hsv {
    /**
     * @brief Underlying floating-point type.
     */
    using value_type = T;

    /**
     * @brief Hue type.
     */
    using hue_type = cyclic<T>;

    /**
     * @brief Normalized component type.
     */
    using component_type = interval<T>;

    /**
     * @brief Hue in [0, 1).
     */
    hue_type h;

    /**
     * @brief Saturation in [0, 1].
     */
    component_type s;

    /**
     * @brief Value in [0, 1].
     */
    component_type v;

    /**
     * @brief Constructs black with zero hue and zero saturation.
     */
    constexpr basic_hsv() noexcept = default;

    /**
     * @brief Constructs an HSV color from normalized scalar components.
     *
     * Hue is normalized by `cyclic<T>`. Saturation and value are clamped by
     * `interval<T>`.
     *
     * @param hue Hue, where one full turn is 1.
     * @param saturation Saturation.
     * @param value Value.
     */
    constexpr basic_hsv(T hue, T saturation, T value)
        : h(hue), s(saturation), v(value)
    {
    }

    /**
     * @brief Constructs an HSV color from component objects.
     *
     * @param hue Hue.
     * @param saturation Saturation.
     * @param value Value.
     */
    constexpr basic_hsv(
        hue_type hue,
        component_type saturation,
        component_type value) noexcept
        : h(hue), s(saturation), v(value)
    {
    }
};

/**
 * @brief CIE 1931 XYZ tristimulus values.
 *
 * @tparam T Floating-point storage type.
 */
template<std::floating_point T>
struct basic_xyz {
    /**
     * @brief Underlying floating-point type.
     */
    using value_type = T;

    /**
     * @brief X tristimulus value.
     */
    T x = static_cast<T>(0);

    /**
     * @brief Y tristimulus value.
     */
    T y = static_cast<T>(0);

    /**
     * @brief Z tristimulus value.
     */
    T z = static_cast<T>(0);

    /**
     * @brief Constructs zero XYZ.
     */
    constexpr basic_xyz() noexcept = default;

    /**
     * @brief Constructs XYZ values.
     *
     * @param x_value X tristimulus value.
     * @param y_value Y tristimulus value.
     * @param z_value Z tristimulus value.
     */
    constexpr basic_xyz(T x_value, T y_value, T z_value) noexcept
        : x(x_value), y(y_value), z(z_value)
    {
    }
};

/**
 * @brief CIE 1976 L*a*b* color values.
 *
 * @tparam T Floating-point storage type.
 */
template<std::floating_point T>
struct basic_lab {
    /**
     * @brief Underlying floating-point type.
     */
    using value_type = T;

    /**
     * @brief L* component.
     */
    T l = static_cast<T>(0);

    /**
     * @brief a* component.
     */
    T a = static_cast<T>(0);

    /**
     * @brief b* component.
     */
    T b = static_cast<T>(0);

    /**
     * @brief Constructs zero Lab.
     */
    constexpr basic_lab() noexcept = default;

    /**
     * @brief Constructs Lab values.
     *
     * @param l_value L* component.
     * @param a_value a* component.
     * @param b_value b* component.
     */
    constexpr basic_lab(T l_value, T a_value, T b_value) noexcept
        : l(l_value), a(a_value), b(b_value)
    {
    }
};

/**
 * @brief CIE 1976 L*u*v* color values.
 *
 * @tparam T Floating-point storage type.
 */
template<std::floating_point T>
struct basic_luv {
    /**
     * @brief Underlying floating-point type.
     */
    using value_type = T;

    /**
     * @brief L* component.
     */
    T l = static_cast<T>(0);

    /**
     * @brief u* component.
     */
    T u = static_cast<T>(0);

    /**
     * @brief v* component.
     */
    T v = static_cast<T>(0);

    /**
     * @brief Constructs zero Luv.
     */
    constexpr basic_luv() noexcept = default;

    /**
     * @brief Constructs Luv values.
     *
     * @param l_value L* component.
     * @param u_value u* component.
     * @param v_value v* component.
     */
    constexpr basic_luv(T l_value, T u_value, T v_value) noexcept
        : l(l_value), u(u_value), v(v_value)
    {
    }
};

/**
 * @brief Ordinary RGB color type using float.
 */
using rgb = basic_rgb<float>;

/**
 * @brief Ordinary CMY color type using float.
 */
using cmy = basic_cmy<float>;

/**
 * @brief Ordinary HSV color type using float.
 */
using hsv = basic_hsv<float>;

/**
 * @brief Ordinary XYZ color type using float.
 */
using xyz = basic_xyz<float>;

/**
 * @brief Ordinary Lab color type using float.
 */
using lab = basic_lab<float>;

/**
 * @brief Ordinary Luv color type using float.
 */
using luv = basic_luv<float>;

namespace detail {

template<std::floating_point T>
[[nodiscard]] constexpr auto color_max(T a, T b, T c) noexcept -> T
{
    T value = a < b ? b : a;
    return value < c ? c : value;
}

template<std::floating_point T>
[[nodiscard]] constexpr auto color_min(T a, T b, T c) noexcept -> T
{
    T value = b < a ? b : a;
    return c < value ? c : value;
}

template<std::floating_point T>
[[nodiscard]] auto color_gamma_decode(T value) -> T
{
    if (value <= static_cast<T>(0.04045)) {
        return value / static_cast<T>(12.92);
    }
    return static_cast<T>(std::pow(
        (value + static_cast<T>(0.055)) / static_cast<T>(1.055),
        static_cast<T>(2.4)));
}

template<std::floating_point T>
[[nodiscard]] auto color_gamma_encode(T value) -> T
{
    if (value <= static_cast<T>(0.0031308)) {
        return value * static_cast<T>(12.92);
    }
    return static_cast<T>(
        static_cast<T>(1.055) *
            std::pow(value, static_cast<T>(1) / static_cast<T>(2.4)) -
        static_cast<T>(0.055));
}

template<std::floating_point T>
[[nodiscard]] constexpr auto color_lab_epsilon() noexcept -> T
{
    return static_cast<T>(216.0L / 24389.0L);
}

template<std::floating_point T>
[[nodiscard]] constexpr auto color_lab_kappa() noexcept -> T
{
    return static_cast<T>(24389.0L / 27.0L);
}

template<std::floating_point T>
[[nodiscard]] auto color_lab_f(T value) -> T
{
    if (value > color_lab_epsilon<T>()) {
        return static_cast<T>(std::cbrt(value));
    }
    return (color_lab_kappa<T>() * value + static_cast<T>(16)) /
           static_cast<T>(116);
}

template<std::floating_point T>
[[nodiscard]] constexpr auto color_lab_f_inv(T value) noexcept -> T
{
    const T value3 = value * value * value;
    if (value3 > color_lab_epsilon<T>()) {
        return value3;
    }
    return (static_cast<T>(116) * value - static_cast<T>(16)) /
           color_lab_kappa<T>();
}

template<std::floating_point T>
[[nodiscard]] constexpr auto d65_x() noexcept -> T
{
    return static_cast<T>(0.95047L);
}

template<std::floating_point T>
[[nodiscard]] constexpr auto d65_y() noexcept -> T
{
    return static_cast<T>(1.0L);
}

template<std::floating_point T>
[[nodiscard]] constexpr auto d65_z() noexcept -> T
{
    return static_cast<T>(1.08883L);
}

template<std::floating_point T>
[[nodiscard]] constexpr auto luv_denominator(T x, T y, T z) noexcept -> T
{
    return x + static_cast<T>(15) * y + static_cast<T>(3) * z;
}

template<std::floating_point T>
[[nodiscard]] constexpr auto luv_u_prime(T x, T y, T z) noexcept -> T
{
    const T denominator = luv_denominator(x, y, z);
    if (denominator == static_cast<T>(0)) {
        return static_cast<T>(0);
    }
    return static_cast<T>(4) * x / denominator;
}

template<std::floating_point T>
[[nodiscard]] constexpr auto luv_v_prime(T x, T y, T z) noexcept -> T
{
    const T denominator = luv_denominator(x, y, z);
    if (denominator == static_cast<T>(0)) {
        return static_cast<T>(0);
    }
    return static_cast<T>(9) * y / denominator;
}

} // namespace detail

/**
 * @brief Converts RGB to CMY.
 *
 * @tparam T Floating-point storage type.
 * @param value RGB color.
 * @return CMY color.
 */
template<std::floating_point T>
[[nodiscard]] constexpr auto to_cmy(basic_rgb<T> value) -> basic_cmy<T>
{
    return basic_cmy<T>(
        static_cast<T>(1) - value.r.value(),
        static_cast<T>(1) - value.g.value(),
        static_cast<T>(1) - value.b.value());
}

/**
 * @brief Converts CMY to RGB.
 *
 * @tparam T Floating-point storage type.
 * @param value CMY color.
 * @return RGB color.
 */
template<std::floating_point T>
[[nodiscard]] constexpr auto to_rgb(basic_cmy<T> value) -> basic_rgb<T>
{
    return basic_rgb<T>(
        static_cast<T>(1) - value.c.value(),
        static_cast<T>(1) - value.m.value(),
        static_cast<T>(1) - value.y.value());
}

/**
 * @brief Converts RGB to HSV.
 *
 * @tparam T Floating-point storage type.
 * @param value RGB color.
 * @return HSV color.
 */
template<std::floating_point T>
[[nodiscard]] constexpr auto to_hsv(basic_rgb<T> value) -> basic_hsv<T>
{
    const T red = value.r.value();
    const T green = value.g.value();
    const T blue = value.b.value();

    const T max_value = detail::color_max(red, green, blue);
    const T min_value = detail::color_min(red, green, blue);
    const T chroma = max_value - min_value;

    T hue = static_cast<T>(0);
    if (chroma != static_cast<T>(0)) {
        if (max_value == red) {
            hue = (green - blue) / chroma / static_cast<T>(6);
        } else if (max_value == green) {
            hue = ((blue - red) / chroma + static_cast<T>(2)) /
                  static_cast<T>(6);
        } else {
            hue = ((red - green) / chroma + static_cast<T>(4)) /
                  static_cast<T>(6);
        }
    }

    const T saturation = max_value == static_cast<T>(0)
                             ? static_cast<T>(0)
                             : chroma / max_value;

    return basic_hsv<T>(hue, saturation, max_value);
}

/**
 * @brief Converts HSV to RGB.
 *
 * @tparam T Floating-point storage type.
 * @param value HSV color.
 * @return RGB color.
 */
template<std::floating_point T>
[[nodiscard]] constexpr auto to_rgb(basic_hsv<T> value) -> basic_rgb<T>
{
    const T hue = value.h.value();
    const T saturation = value.s.value();
    const T v = value.v.value();

    if (saturation == static_cast<T>(0)) {
        return basic_rgb<T>(v, v, v);
    }

    const T h6 = hue * static_cast<T>(6);
    const int sector = static_cast<int>(h6);
    const T f = h6 - static_cast<T>(sector);
    const T p = v * (static_cast<T>(1) - saturation);
    const T q = v * (static_cast<T>(1) - saturation * f);
    const T t = v *
                (static_cast<T>(1) -
                 saturation * (static_cast<T>(1) - f));

    switch (sector) {
    case 0:
        return basic_rgb<T>(v, t, p);
    case 1:
        return basic_rgb<T>(q, v, p);
    case 2:
        return basic_rgb<T>(p, v, t);
    case 3:
        return basic_rgb<T>(p, q, v);
    case 4:
        return basic_rgb<T>(t, p, v);
    default:
        return basic_rgb<T>(v, p, q);
    }
}

/**
 * @brief Converts RGB to CIE XYZ using sRGB/D65.
 *
 * @tparam T Floating-point storage type.
 * @param value RGB color interpreted as nonlinear sRGB.
 * @return XYZ color using the D65 white point.
 */
template<std::floating_point T>
[[nodiscard]] auto to_xyz(basic_rgb<T> value) -> basic_xyz<T>
{
    const T red = detail::color_gamma_decode(value.r.value());
    const T green = detail::color_gamma_decode(value.g.value());
    const T blue = detail::color_gamma_decode(value.b.value());

    return basic_xyz<T>(
        static_cast<T>(0.4124564L) * red +
            static_cast<T>(0.3575761L) * green +
            static_cast<T>(0.1804375L) * blue,
        static_cast<T>(0.2126729L) * red +
            static_cast<T>(0.7151522L) * green +
            static_cast<T>(0.0721750L) * blue,
        static_cast<T>(0.0193339L) * red +
            static_cast<T>(0.1191920L) * green +
            static_cast<T>(0.9503041L) * blue);
}

/**
 * @brief Converts CIE XYZ to RGB using sRGB/D65.
 *
 * @tparam T Floating-point storage type.
 * @param value XYZ color using the D65 white point.
 * @return RGB color interpreted as nonlinear sRGB.
 */
template<std::floating_point T>
[[nodiscard]] auto to_rgb(basic_xyz<T> value) -> basic_rgb<T>
{
    const T red_linear = static_cast<T>(3.2404542L) * value.x -
                         static_cast<T>(1.5371385L) * value.y -
                         static_cast<T>(0.4985314L) * value.z;
    const T green_linear = -static_cast<T>(0.9692660L) * value.x +
                           static_cast<T>(1.8760108L) * value.y +
                           static_cast<T>(0.0415560L) * value.z;
    const T blue_linear = static_cast<T>(0.0556434L) * value.x -
                          static_cast<T>(0.2040259L) * value.y +
                          static_cast<T>(1.0572252L) * value.z;

    return basic_rgb<T>(
        detail::color_gamma_encode(red_linear),
        detail::color_gamma_encode(green_linear),
        detail::color_gamma_encode(blue_linear));
}

/**
 * @brief Converts CIE XYZ to CIE L*a*b* using D65.
 *
 * @tparam T Floating-point storage type.
 * @param value XYZ color using the D65 white point.
 * @return Lab color.
 */
template<std::floating_point T>
[[nodiscard]] auto to_lab(basic_xyz<T> value) -> basic_lab<T>
{
    const T fx = detail::color_lab_f(value.x / detail::d65_x<T>());
    const T fy = detail::color_lab_f(value.y / detail::d65_y<T>());
    const T fz = detail::color_lab_f(value.z / detail::d65_z<T>());

    return basic_lab<T>(
        static_cast<T>(116) * fy - static_cast<T>(16),
        static_cast<T>(500) * (fx - fy),
        static_cast<T>(200) * (fy - fz));
}

/**
 * @brief Converts CIE L*a*b* to CIE XYZ using D65.
 *
 * @tparam T Floating-point storage type.
 * @param value Lab color.
 * @return XYZ color using the D65 white point.
 */
template<std::floating_point T>
[[nodiscard]] constexpr auto to_xyz(basic_lab<T> value) -> basic_xyz<T>
{
    const T fy = (value.l + static_cast<T>(16)) / static_cast<T>(116);
    const T fx = fy + value.a / static_cast<T>(500);
    const T fz = fy - value.b / static_cast<T>(200);

    return basic_xyz<T>(
        detail::d65_x<T>() * detail::color_lab_f_inv(fx),
        detail::d65_y<T>() * detail::color_lab_f_inv(fy),
        detail::d65_z<T>() * detail::color_lab_f_inv(fz));
}

/**
 * @brief Converts CIE XYZ to CIE L*u*v* using D65.
 *
 * @tparam T Floating-point storage type.
 * @param value XYZ color using the D65 white point.
 * @return Luv color.
 */
template<std::floating_point T>
[[nodiscard]] auto to_luv(basic_xyz<T> value) -> basic_luv<T>
{
    const T yr = value.y / detail::d65_y<T>();
    const T l = yr > detail::color_lab_epsilon<T>()
                    ? static_cast<T>(116) * static_cast<T>(std::cbrt(yr)) -
                          static_cast<T>(16)
                    : detail::color_lab_kappa<T>() * yr;

    if (l == static_cast<T>(0)) {
        return basic_luv<T>();
    }

    const T u_prime = detail::luv_u_prime(value.x, value.y, value.z);
    const T v_prime = detail::luv_v_prime(value.x, value.y, value.z);
    const T un = detail::luv_u_prime(
        detail::d65_x<T>(), detail::d65_y<T>(), detail::d65_z<T>());
    const T vn = detail::luv_v_prime(
        detail::d65_x<T>(), detail::d65_y<T>(), detail::d65_z<T>());

    return basic_luv<T>(
        l,
        static_cast<T>(13) * l * (u_prime - un),
        static_cast<T>(13) * l * (v_prime - vn));
}

/**
 * @brief Converts CIE L*u*v* to CIE XYZ using D65.
 *
 * @tparam T Floating-point storage type.
 * @param value Luv color.
 * @return XYZ color using the D65 white point.
 */
template<std::floating_point T>
[[nodiscard]] auto to_xyz(basic_luv<T> value) -> basic_xyz<T>
{
    if (value.l == static_cast<T>(0)) {
        return basic_xyz<T>();
    }

    const T un = detail::luv_u_prime(
        detail::d65_x<T>(), detail::d65_y<T>(), detail::d65_z<T>());
    const T vn = detail::luv_v_prime(
        detail::d65_x<T>(), detail::d65_y<T>(), detail::d65_z<T>());

    const T u_prime = value.u / (static_cast<T>(13) * value.l) + un;
    const T v_prime = value.v / (static_cast<T>(13) * value.l) + vn;

    const T y = value.l >
                        detail::color_lab_kappa<T>() *
                            detail::color_lab_epsilon<T>()
                    ? detail::d65_y<T>() *
                          static_cast<T>(std::pow(
                              (value.l + static_cast<T>(16)) /
                                  static_cast<T>(116),
                              static_cast<T>(3)))
                    : detail::d65_y<T>() * value.l /
                          detail::color_lab_kappa<T>();

    if (v_prime == static_cast<T>(0)) {
        return basic_xyz<T>(static_cast<T>(0), y, static_cast<T>(0));
    }

    const T x = y * static_cast<T>(9) * u_prime /
                (static_cast<T>(4) * v_prime);
    const T z = y *
                (static_cast<T>(12) - static_cast<T>(3) * u_prime -
                 static_cast<T>(20) * v_prime) /
                (static_cast<T>(4) * v_prime);

    return basic_xyz<T>(x, y, z);
}

} // namespace xer

#endif /* XER_BITS_COLOR_H_INCLUDED_ */

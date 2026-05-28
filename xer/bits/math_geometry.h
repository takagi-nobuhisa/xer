/**
 * @file xer/bits/math_geometry.h
 * @brief Small vector and coordinate helpers for real-number math.
 */

#pragma once

#ifndef XER_BITS_MATH_GEOMETRY_H_INCLUDED_
#define XER_BITS_MATH_GEOMETRY_H_INCLUDED_

#include <cmath>
#include <concepts>
#include <cstddef>
#include <expected>
#include <functional>
#include <type_traits>
#include <utility>

#include <xer/bits/error.h>

namespace xer {

template<class T, std::size_t N = 2>
struct vec;

template<class T>
struct vec<T, 2> {
    T x;
    T y;

    [[nodiscard]] constexpr auto operator[](std::size_t index) noexcept -> T&
    {
        switch (index) {
        case 0:
            return this->x;
        case 1:
            return this->y;
        default:
            std::unreachable();
        }
    }

    [[nodiscard]] constexpr auto operator[](std::size_t index) const noexcept -> const T&
    {
        switch (index) {
        case 0:
            return this->x;
        case 1:
            return this->y;
        default:
            std::unreachable();
        }
    }

    [[nodiscard]] constexpr auto at(std::size_t index) noexcept
        -> result<std::reference_wrapper<T>>
    {
        switch (index) {
        case 0:
            return std::ref(this->x);
        case 1:
            return std::ref(this->y);
        default:
            return std::unexpected(make_error(error_t::out_of_range));
        }
    }

    [[nodiscard]] constexpr auto at(std::size_t index) const noexcept
        -> result<std::reference_wrapper<const T>>
    {
        switch (index) {
        case 0:
            return std::cref(this->x);
        case 1:
            return std::cref(this->y);
        default:
            return std::unexpected(make_error(error_t::out_of_range));
        }
    }
};

template<class T>
struct vec<T, 3> {
    T x;
    T y;
    T z;

    [[nodiscard]] constexpr auto operator[](std::size_t index) noexcept -> T&
    {
        switch (index) {
        case 0:
            return this->x;
        case 1:
            return this->y;
        case 2:
            return this->z;
        default:
            std::unreachable();
        }
    }

    [[nodiscard]] constexpr auto operator[](std::size_t index) const noexcept -> const T&
    {
        switch (index) {
        case 0:
            return this->x;
        case 1:
            return this->y;
        case 2:
            return this->z;
        default:
            std::unreachable();
        }
    }

    [[nodiscard]] constexpr auto at(std::size_t index) noexcept
        -> result<std::reference_wrapper<T>>
    {
        switch (index) {
        case 0:
            return std::ref(this->x);
        case 1:
            return std::ref(this->y);
        case 2:
            return std::ref(this->z);
        default:
            return std::unexpected(make_error(error_t::out_of_range));
        }
    }

    [[nodiscard]] constexpr auto at(std::size_t index) const noexcept
        -> result<std::reference_wrapper<const T>>
    {
        switch (index) {
        case 0:
            return std::cref(this->x);
        case 1:
            return std::cref(this->y);
        case 2:
            return std::cref(this->z);
        default:
            return std::unexpected(make_error(error_t::out_of_range));
        }
    }
};

template<class T>
struct vec<T, 4> {
    T x;
    T y;
    T z;
    T w;

    [[nodiscard]] constexpr auto operator[](std::size_t index) noexcept -> T&
    {
        switch (index) {
        case 0:
            return this->x;
        case 1:
            return this->y;
        case 2:
            return this->z;
        case 3:
            return this->w;
        default:
            std::unreachable();
        }
    }

    [[nodiscard]] constexpr auto operator[](std::size_t index) const noexcept -> const T&
    {
        switch (index) {
        case 0:
            return this->x;
        case 1:
            return this->y;
        case 2:
            return this->z;
        case 3:
            return this->w;
        default:
            std::unreachable();
        }
    }

    [[nodiscard]] constexpr auto at(std::size_t index) noexcept
        -> result<std::reference_wrapper<T>>
    {
        switch (index) {
        case 0:
            return std::ref(this->x);
        case 1:
            return std::ref(this->y);
        case 2:
            return std::ref(this->z);
        case 3:
            return std::ref(this->w);
        default:
            return std::unexpected(make_error(error_t::out_of_range));
        }
    }

    [[nodiscard]] constexpr auto at(std::size_t index) const noexcept
        -> result<std::reference_wrapper<const T>>
    {
        switch (index) {
        case 0:
            return std::cref(this->x);
        case 1:
            return std::cref(this->y);
        case 2:
            return std::cref(this->z);
        case 3:
            return std::cref(this->w);
        default:
            return std::unexpected(make_error(error_t::out_of_range));
        }
    }
};



template<class T, std::size_t N>
    requires std::is_arithmetic_v<T>
[[nodiscard]] constexpr auto dot(vec<T, N> a, vec<T, N> b) noexcept -> T
{
    auto value = T{};

    for (std::size_t i = 0; i < N; ++i) {
        value += a[i] * b[i];
    }

    return value;
}

template<class T, std::size_t N>
    requires std::is_arithmetic_v<T>
[[nodiscard]] auto length(vec<T, N> v) noexcept -> std::common_type_t<T, double>
{
    using result_type = std::common_type_t<T, double>;

    auto value = result_type{};

    for (std::size_t i = 0; i < N; ++i) {
        const auto component = static_cast<result_type>(v[i]);
        value += component * component;
    }

    return std::sqrt(value);
}

template<class T, std::size_t N>
    requires std::is_arithmetic_v<T>
[[nodiscard]] auto distance(vec<T, N> a, vec<T, N> b) noexcept -> std::common_type_t<T, double>
{
    using result_type = std::common_type_t<T, double>;

    auto value = result_type{};

    for (std::size_t i = 0; i < N; ++i) {
        const auto diff = static_cast<result_type>(a[i]) - static_cast<result_type>(b[i]);
        value += diff * diff;
    }

    return std::sqrt(value);
}

template<class T, std::size_t N>
    requires std::is_arithmetic_v<T>
[[nodiscard]] auto normalize(vec<T, N> v) noexcept
    -> result<vec<std::common_type_t<T, double>, N>>
{
    using result_type = std::common_type_t<T, double>;

    const auto len = length(v);
    if (len == result_type{}) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    auto result = vec<result_type, N>{};
    for (std::size_t i = 0; i < N; ++i) {
        result[i] = static_cast<result_type>(v[i]) / len;
    }

    return result;
}

template<class T, std::size_t N>
    requires std::is_arithmetic_v<T>
[[nodiscard]] auto angle(vec<T, N> a, vec<T, N> b) noexcept
    -> result<std::common_type_t<T, double>>
{
    using result_type = std::common_type_t<T, double>;

    auto product = result_type{};
    auto length_a_squared = result_type{};
    auto length_b_squared = result_type{};

    for (std::size_t i = 0; i < N; ++i) {
        const auto ai = static_cast<result_type>(a[i]);
        const auto bi = static_cast<result_type>(b[i]);

        product += ai * bi;
        length_a_squared += ai * ai;
        length_b_squared += bi * bi;
    }

    if (length_a_squared == result_type{} || length_b_squared == result_type{}) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    auto cosine = product / std::sqrt(length_a_squared * length_b_squared);

    if (cosine < static_cast<result_type>(-1)) {
        cosine = static_cast<result_type>(-1);
    }
    else if (static_cast<result_type>(1) < cosine) {
        cosine = static_cast<result_type>(1);
    }

    return std::acos(cosine);
}

template<class T, class Angle>
    requires (std::is_arithmetic_v<T> && std::is_arithmetic_v<Angle>)
[[nodiscard]] auto rotate(vec<T, 2> v, Angle theta) noexcept
    -> vec<std::common_type_t<T, Angle, double>, 2>
{
    using result_type = std::common_type_t<T, Angle, double>;

    const auto x = static_cast<result_type>(v.x);
    const auto y = static_cast<result_type>(v.y);
    const auto t = static_cast<result_type>(theta);
    const auto c = std::cos(t);
    const auto s = std::sin(t);

    return vec<result_type, 2>{
        x * c - y * s,
        x * s + y * c,
    };
}

template<class T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] constexpr auto cross(vec<T, 3> a, vec<T, 3> b) noexcept -> vec<T, 3>
{
    return vec<T, 3>{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
}

template<class T, std::size_t N = 2>
struct polar;

template<class T>
struct polar<T, 2> {
    T r;
    T theta;
};

template<std::floating_point T>
[[nodiscard]] auto to_polar(vec<T, 2> v) noexcept -> polar<T, 2>
{
    return polar<T, 2>{
        std::hypot(v.x, v.y),
        std::atan2(v.y, v.x),
    };
}

template<std::floating_point T>
[[nodiscard]] auto to_cartesian(polar<T, 2> p) noexcept -> vec<T, 2>
{
    return vec<T, 2>{
        p.r * std::cos(p.theta),
        p.r * std::sin(p.theta),
    };
}

} // namespace xer

#endif /* XER_BITS_MATH_GEOMETRY_H_INCLUDED_ */

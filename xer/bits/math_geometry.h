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

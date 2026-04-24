/**
 * @file xer/bits/stdfloat.h
 * @brief Floating-point type aliases, helper aliases, and floating-point literals.
 */

#pragma once

#ifndef XER_BITS_STDFLOAT_H_INCLUDED_
#define XER_BITS_STDFLOAT_H_INCLUDED_

#include <cstdint>
#include <limits>
#include <type_traits>

#if defined(__has_include)
#    if __has_include(<stdfloat>)
#        include <stdfloat>
#        define XER_HAS_STDFLOAT_HEADER 1
#    else
#        define XER_HAS_STDFLOAT_HEADER 0
#    endif
#    if __has_include(<decimal/decimal>)
#        include <decimal/decimal>
#        define XER_HAS_DECIMAL_HEADER 1
#    else
#        define XER_HAS_DECIMAL_HEADER 0
#    endif
#else
#    define XER_HAS_STDFLOAT_HEADER 0
#    define XER_HAS_DECIMAL_HEADER 0
#endif

#include <xer/bits/common.h>
#include <xer/bits/numeric_limits.h>

namespace xer {

#if XER_HAS_STDFLOAT_HEADER && defined(__STDCPP_FLOAT16_T__)
using std::float16_t;
#    define XER_HAS_FLOAT16_T 1
#else
#    define XER_HAS_FLOAT16_T 0
#endif

#if XER_HAS_STDFLOAT_HEADER && defined(__STDCPP_FLOAT32_T__)
using std::float32_t;
#else
using float32_t = float;
#endif
#define XER_HAS_FLOAT32_T 1

#if XER_HAS_STDFLOAT_HEADER && defined(__STDCPP_FLOAT64_T__)
using std::float64_t;
#else
using float64_t = double;
#endif
#define XER_HAS_FLOAT64_T 1

#if XER_HAS_STDFLOAT_HEADER && defined(__STDCPP_FLOAT128_T__)
using std::float128_t;
#    define XER_HAS_FLOAT128_T 1
#elif defined(__SIZEOF_FLOAT128__)
using float128_t = __float128;
#    define XER_HAS_FLOAT128_T 1
#else
#    define XER_HAS_FLOAT128_T 0
#endif

#if XER_HAS_STDFLOAT_HEADER && defined(__STDCPP_BFLOAT16_T__)
using std::bfloat16_t;
#    define XER_HAS_BFLOAT16_T 1
#else
#    define XER_HAS_BFLOAT16_T 0
#endif

#if defined(__LDBL_MANT_DIG__) && __LDBL_MANT_DIG__ == 64 &&                  \
    defined(__LDBL_MAX_EXP__) && __LDBL_MAX_EXP__ == 16384
/**
 * @brief 80-bit extended floating-point type when `long double` has that format.
 *
 * GCC on the primary XER targets commonly stores this format in a `long double`
 * object whose storage size may be larger than 80 bits. The name describes the
 * floating-point format rather than the object size.
 */
using float80_t = long double;
#    define XER_HAS_FLOAT80_T 1
#else
#    define XER_HAS_FLOAT80_T 0
#endif

#if XER_HAS_FLOAT16_T
using float_least16_t = float16_t;
#else
using float_least16_t = float32_t;
#endif
#define XER_HAS_FLOAT_LEAST16_T 1

using float_least32_t = float32_t;
#define XER_HAS_FLOAT_LEAST32_T 1

using float_least64_t = float64_t;
#define XER_HAS_FLOAT_LEAST64_T 1

#if XER_HAS_FLOAT80_T
using float_least80_t = float80_t;
#    define XER_HAS_FLOAT_LEAST80_T 1
#elif XER_HAS_FLOAT128_T
using float_least80_t = float128_t;
#    define XER_HAS_FLOAT_LEAST80_T 1
#else
#    define XER_HAS_FLOAT_LEAST80_T 0
#endif

#if XER_HAS_FLOAT128_T
using float_least128_t = float128_t;
#    define XER_HAS_FLOAT_LEAST128_T 1
#else
#    define XER_HAS_FLOAT_LEAST128_T 0
#endif

using float_fast16_t = float;
#define XER_HAS_FLOAT_FAST16_T 1

using float_fast32_t = float;
#define XER_HAS_FLOAT_FAST32_T 1

using float_fast64_t = double;
#define XER_HAS_FLOAT_FAST64_T 1

#if XER_HAS_FLOAT80_T
using float_fast80_t = float80_t;
#    define XER_HAS_FLOAT_FAST80_T 1
#elif XER_HAS_FLOAT128_T
using float_fast80_t = float128_t;
#    define XER_HAS_FLOAT_FAST80_T 1
#else
#    define XER_HAS_FLOAT_FAST80_T 0
#endif

#if XER_HAS_FLOAT128_T
using float_fast128_t = float128_t;
#    define XER_HAS_FLOAT_FAST128_T 1
#else
#    define XER_HAS_FLOAT_FAST128_T 0
#endif

#if XER_HAS_FLOAT128_T
using floatmax_t = float128_t;
#elif XER_HAS_FLOAT80_T
using floatmax_t = float80_t;
#else
using floatmax_t = long double;
#endif

#if XER_HAS_DECIMAL_HEADER
using decimal32_t = std::decimal::decimal32;
using decimal64_t = std::decimal::decimal64;
using decimal128_t = std::decimal::decimal128;
#    define XER_HAS_DECIMAL32_T 1
#    define XER_HAS_DECIMAL64_T 1
#    define XER_HAS_DECIMAL128_T 1
#else
#    define XER_HAS_DECIMAL32_T 0
#    define XER_HAS_DECIMAL64_T 0
#    define XER_HAS_DECIMAL128_T 0
#endif

#if XER_HAS_DECIMAL32_T
using decimal_least32_t = decimal32_t;
using decimal_fast32_t = decimal32_t;
#    define XER_HAS_DECIMAL_LEAST32_T 1
#    define XER_HAS_DECIMAL_FAST32_T 1
#else
#    define XER_HAS_DECIMAL_LEAST32_T 0
#    define XER_HAS_DECIMAL_FAST32_T 0
#endif

#if XER_HAS_DECIMAL64_T
using decimal_least64_t = decimal64_t;
using decimal_fast64_t = decimal64_t;
#    define XER_HAS_DECIMAL_LEAST64_T 1
#    define XER_HAS_DECIMAL_FAST64_T 1
#else
#    define XER_HAS_DECIMAL_LEAST64_T 0
#    define XER_HAS_DECIMAL_FAST64_T 0
#endif

#if XER_HAS_DECIMAL128_T
using decimal_least128_t = decimal128_t;
using decimal_fast128_t = decimal128_t;
using decimalmax_t = decimal128_t;
#    define XER_HAS_DECIMAL_LEAST128_T 1
#    define XER_HAS_DECIMAL_FAST128_T 1
#    define XER_HAS_DECIMALMAX_T 1
#else
#    define XER_HAS_DECIMAL_LEAST128_T 0
#    define XER_HAS_DECIMAL_FAST128_T 0
#    define XER_HAS_DECIMALMAX_T 0
#endif

namespace literals::floating_literals {

#if XER_HAS_FLOAT16_T
[[nodiscard]] constexpr auto operator""_f16(long double value) noexcept -> float16_t
{
    return static_cast<float16_t>(value);
}

[[nodiscard]] constexpr auto operator""_f16(unsigned long long value) noexcept
    -> float16_t
{
    return static_cast<float16_t>(value);
}
#endif

[[nodiscard]] constexpr auto operator""_f32(long double value) noexcept -> float32_t
{
    return static_cast<float32_t>(value);
}

[[nodiscard]] constexpr auto operator""_f32(unsigned long long value) noexcept
    -> float32_t
{
    return static_cast<float32_t>(value);
}

[[nodiscard]] constexpr auto operator""_f64(long double value) noexcept -> float64_t
{
    return static_cast<float64_t>(value);
}

[[nodiscard]] constexpr auto operator""_f64(unsigned long long value) noexcept
    -> float64_t
{
    return static_cast<float64_t>(value);
}

#if XER_HAS_FLOAT80_T
[[nodiscard]] constexpr auto operator""_f80(long double value) noexcept -> float80_t
{
    return static_cast<float80_t>(value);
}

[[nodiscard]] constexpr auto operator""_f80(unsigned long long value) noexcept
    -> float80_t
{
    return static_cast<float80_t>(value);
}
#endif

#if XER_HAS_FLOAT128_T
[[nodiscard]] constexpr auto operator""_f128(long double value) noexcept
    -> float128_t
{
    return static_cast<float128_t>(value);
}

[[nodiscard]] constexpr auto operator""_f128(unsigned long long value) noexcept
    -> float128_t
{
    return static_cast<float128_t>(value);
}
#endif

#if XER_HAS_BFLOAT16_T
[[nodiscard]] constexpr auto operator""_bf16(long double value) noexcept
    -> bfloat16_t
{
    return static_cast<bfloat16_t>(value);
}

[[nodiscard]] constexpr auto operator""_bf16(unsigned long long value) noexcept
    -> bfloat16_t
{
    return static_cast<bfloat16_t>(value);
}
#endif

[[nodiscard]] constexpr auto operator""_fl16(long double value) noexcept
    -> float_least16_t
{
    return static_cast<float_least16_t>(value);
}

[[nodiscard]] constexpr auto operator""_fl16(unsigned long long value) noexcept
    -> float_least16_t
{
    return static_cast<float_least16_t>(value);
}

[[nodiscard]] constexpr auto operator""_fl32(long double value) noexcept
    -> float_least32_t
{
    return static_cast<float_least32_t>(value);
}

[[nodiscard]] constexpr auto operator""_fl32(unsigned long long value) noexcept
    -> float_least32_t
{
    return static_cast<float_least32_t>(value);
}

[[nodiscard]] constexpr auto operator""_fl64(long double value) noexcept
    -> float_least64_t
{
    return static_cast<float_least64_t>(value);
}

[[nodiscard]] constexpr auto operator""_fl64(unsigned long long value) noexcept
    -> float_least64_t
{
    return static_cast<float_least64_t>(value);
}

#if XER_HAS_FLOAT_LEAST80_T
[[nodiscard]] constexpr auto operator""_fl80(long double value) noexcept
    -> float_least80_t
{
    return static_cast<float_least80_t>(value);
}

[[nodiscard]] constexpr auto operator""_fl80(unsigned long long value) noexcept
    -> float_least80_t
{
    return static_cast<float_least80_t>(value);
}
#endif

#if XER_HAS_FLOAT_LEAST128_T
[[nodiscard]] constexpr auto operator""_fl128(long double value) noexcept
    -> float_least128_t
{
    return static_cast<float_least128_t>(value);
}

[[nodiscard]] constexpr auto operator""_fl128(unsigned long long value) noexcept
    -> float_least128_t
{
    return static_cast<float_least128_t>(value);
}
#endif

} // namespace literals::floating_literals

} // namespace xer

#endif /* XER_BITS_STDFLOAT_H_INCLUDED_ */

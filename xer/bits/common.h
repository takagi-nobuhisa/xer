/**
 * @file xer/bits/common.h
 * @brief Common compiler/environment checks and portability macros.
 */

#pragma once

#ifndef XER_BITS_COMMON_H_INCLUDED_
#define XER_BITS_COMMON_H_INCLUDED_

#include <climits>
#include <cstddef>
#include <string>


#if defined(_LIBCPP_VERSION)
namespace std {

/**
 * @brief Compatibility specialization for std::basic_string<unsigned char> on libc++.
 *
 * libc++ does not provide std::char_traits<unsigned char>, while libstdc++
 * accepts it as an extension. xer supports unsigned char as a byte string
 * character type, so this specialization keeps that API usable with libc++.
 */
template<>
struct char_traits<unsigned char> {
    using char_type = unsigned char;
    using int_type = unsigned int;
    using off_type = streamoff;
    using pos_type = streampos;
    using state_type = mbstate_t;
    using comparison_category = strong_ordering;

    static constexpr auto assign(char_type& r, const char_type a) noexcept -> void
    {
        r = a;
    }

    [[nodiscard]] static constexpr auto eq(const char_type a, const char_type b) noexcept -> bool
    {
        return a == b;
    }

    [[nodiscard]] static constexpr auto lt(const char_type a, const char_type b) noexcept -> bool
    {
        return a < b;
    }

    [[nodiscard]] static constexpr auto compare(
        const char_type* s1,
        const char_type* s2,
        const size_t n) -> int
    {
        for (size_t i = 0; i < n; ++i) {
            if (lt(s1[i], s2[i])) {
                return -1;
            }
            if (lt(s2[i], s1[i])) {
                return 1;
            }
        }

        return 0;
    }

    [[nodiscard]] static constexpr auto length(const char_type* s) -> size_t
    {
        size_t n = 0;
        while (!eq(s[n], char_type{})) {
            ++n;
        }

        return n;
    }

    [[nodiscard]] static constexpr auto find(
        const char_type* s,
        const size_t n,
        const char_type a) -> const char_type*
    {
        for (size_t i = 0; i < n; ++i) {
            if (eq(s[i], a)) {
                return s + i;
            }
        }

        return nullptr;
    }

    static constexpr auto move(
        char_type* dst,
        const char_type* src,
        const size_t n) -> char_type*
    {
        if (dst == src || n == 0) {
            return dst;
        }

        if (dst < src) {
            for (size_t i = 0; i < n; ++i) {
                dst[i] = src[i];
            }
        } else {
            for (size_t i = n; i > 0; --i) {
                dst[i - 1] = src[i - 1];
            }
        }

        return dst;
    }

    static constexpr auto copy(
        char_type* dst,
        const char_type* src,
        const size_t n) -> char_type*
    {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = src[i];
        }

        return dst;
    }

    static constexpr auto assign(
        char_type* dst,
        const size_t n,
        const char_type a) -> char_type*
    {
        for (size_t i = 0; i < n; ++i) {
            dst[i] = a;
        }

        return dst;
    }

    [[nodiscard]] static constexpr auto not_eof(const int_type c) noexcept -> int_type
    {
        return eq_int_type(c, eof()) ? 0 : c;
    }

    [[nodiscard]] static constexpr auto to_char_type(const int_type c) noexcept -> char_type
    {
        return static_cast<char_type>(c);
    }

    [[nodiscard]] static constexpr auto to_int_type(const char_type c) noexcept -> int_type
    {
        return static_cast<int_type>(c);
    }

    [[nodiscard]] static constexpr auto eq_int_type(const int_type c1, const int_type c2) noexcept -> bool
    {
        return c1 == c2;
    }

    [[nodiscard]] static constexpr auto eof() noexcept -> int_type
    {
        return static_cast<int_type>(-1);
    }
};

} // namespace std
#endif

#if !defined(__cplusplus)
#    error "xer requires C++."
#endif

/**
 * @brief Common name for the active C++ language version.
 */
#define XER_STDCPP_VERSION __cplusplus

/**
 * @brief Common name for the current function signature string.
 */
#define XER_PRETTY_FUNCTION __PRETTY_FUNCTION__

#if defined(__clang__)
static_assert(
    (__clang_major__ > 18) ||
    (__clang_major__ == 18 && __clang_minor__ >= 0),
    "xer requires Clang 18.0.0 or later.");
#elif defined(__GNUC__)
static_assert(
    (__GNUC__ > 13) ||
    (__GNUC__ == 13 && __GNUC_MINOR__ > 3) ||
    (__GNUC__ == 13 && __GNUC_MINOR__ == 3 && __GNUC_PATCHLEVEL__ >= 0),
    "xer requires GCC 13.3.0 or later.");
#else
#    error "xer requires GCC 13.3.0 or later, or Clang 18.0.0 or later."
#endif

static_assert(XER_STDCPP_VERSION >= 202100L, "xer requires C++23 or later.");

static_assert(CHAR_BIT == 8, "xer requires CHAR_BIT == 8.");

static_assert(sizeof(short) == 2, "xer requires 16-bit short.");
static_assert(sizeof(int) == 4, "xer requires 32-bit int.");
static_assert(sizeof(long long) == 8, "xer requires 64-bit long long.");

static_assert(sizeof(float) == 4, "xer requires 32-bit float.");
static_assert(sizeof(double) == 8, "xer requires 64-bit double.");

#endif /* XER_BITS_COMMON_H_INCLUDED_ */

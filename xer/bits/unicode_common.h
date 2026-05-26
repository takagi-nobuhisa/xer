/**
 * @file xer/bits/unicode_common.h
 * @brief Internal Unicode scalar value helpers.
 */

#pragma once

#ifndef XER_BITS_UNICODE_COMMON_H_INCLUDED_
#define XER_BITS_UNICODE_COMMON_H_INCLUDED_

#include <cstdint>

namespace xer::detail {

[[nodiscard]] constexpr auto is_unicode_surrogate(char32_t value) noexcept -> bool
{
    return value >= static_cast<char32_t>(0xD800) &&
           value <= static_cast<char32_t>(0xDFFF);
}

[[nodiscard]] constexpr auto is_unicode_high_surrogate(char16_t value) noexcept -> bool
{
    return value >= static_cast<char16_t>(0xD800) &&
           value <= static_cast<char16_t>(0xDBFF);
}

[[nodiscard]] constexpr auto is_unicode_low_surrogate(char16_t value) noexcept -> bool
{
    return value >= static_cast<char16_t>(0xDC00) &&
           value <= static_cast<char16_t>(0xDFFF);
}

[[nodiscard]] constexpr auto is_unicode_scalar_value(char32_t value) noexcept -> bool
{
    return value <= static_cast<char32_t>(0x10FFFF) &&
           !is_unicode_surrogate(value);
}

[[nodiscard]] constexpr auto is_unicode_bmp_scalar_value(char32_t value) noexcept
    -> bool
{
    return value <= static_cast<char32_t>(0xFFFF) &&
           is_unicode_scalar_value(value);
}

[[nodiscard]] constexpr auto combine_unicode_surrogates(
    char16_t high,
    char16_t low) noexcept -> char32_t
{
    const auto high_value =
        static_cast<char32_t>(high) - static_cast<char32_t>(0xD800);
    const auto low_value =
        static_cast<char32_t>(low) - static_cast<char32_t>(0xDC00);
    return static_cast<char32_t>(0x10000) + ((high_value << 10) | low_value);
}

} // namespace xer::detail

#endif /* XER_BITS_UNICODE_COMMON_H_INCLUDED_ */

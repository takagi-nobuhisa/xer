/**
 * @file xer/bits/unicode_common.h
 * @brief Internal Unicode scalar value helpers.
 */

#pragma once

#ifndef XER_BITS_UNICODE_COMMON_H_INCLUDED_
#define XER_BITS_UNICODE_COMMON_H_INCLUDED_

#include <cstdint>

namespace xer::detail {

inline constexpr char32_t unicode_max_code_point = static_cast<char32_t>(0x10FFFF);
inline constexpr char32_t unicode_bmp_max_code_point = static_cast<char32_t>(0xFFFF);
inline constexpr char32_t unicode_surrogate_first = static_cast<char32_t>(0xD800);
inline constexpr char32_t unicode_high_surrogate_first = static_cast<char32_t>(0xD800);
inline constexpr char32_t unicode_high_surrogate_last = static_cast<char32_t>(0xDBFF);
inline constexpr char32_t unicode_low_surrogate_first = static_cast<char32_t>(0xDC00);
inline constexpr char32_t unicode_low_surrogate_last = static_cast<char32_t>(0xDFFF);
inline constexpr char32_t unicode_surrogate_last = static_cast<char32_t>(0xDFFF);
inline constexpr char32_t unicode_supplementary_first = static_cast<char32_t>(0x10000);

[[nodiscard]] constexpr auto is_unicode_surrogate(char32_t value) noexcept -> bool
{
    return value >= unicode_surrogate_first && value <= unicode_surrogate_last;
}

[[nodiscard]] constexpr auto is_unicode_high_surrogate(char16_t value) noexcept -> bool
{
    return value >= static_cast<char16_t>(unicode_high_surrogate_first) &&
           value <= static_cast<char16_t>(unicode_high_surrogate_last);
}

[[nodiscard]] constexpr auto is_unicode_high_surrogate(char32_t value) noexcept -> bool
{
    return value >= unicode_high_surrogate_first &&
           value <= unicode_high_surrogate_last;
}

[[nodiscard]] constexpr auto is_unicode_low_surrogate(char16_t value) noexcept -> bool
{
    return value >= static_cast<char16_t>(unicode_low_surrogate_first) &&
           value <= static_cast<char16_t>(unicode_low_surrogate_last);
}

[[nodiscard]] constexpr auto is_unicode_low_surrogate(char32_t value) noexcept -> bool
{
    return value >= unicode_low_surrogate_first &&
           value <= unicode_low_surrogate_last;
}

[[nodiscard]] constexpr auto is_unicode_scalar_value(char32_t value) noexcept -> bool
{
    return value <= unicode_max_code_point && !is_unicode_surrogate(value);
}

[[nodiscard]] constexpr auto is_unicode_bmp_scalar_value(char32_t value) noexcept
    -> bool
{
    return value <= unicode_bmp_max_code_point && is_unicode_scalar_value(value);
}

[[nodiscard]] constexpr auto is_valid_code_point(char32_t value) noexcept -> bool
{
    return is_unicode_scalar_value(value);
}

[[nodiscard]] constexpr auto unicode_range_intersects_surrogates(
    char32_t first_code_point,
    char32_t last_code_point) noexcept -> bool
{
    return first_code_point <= unicode_surrogate_last &&
           last_code_point >= unicode_surrogate_first;
}

[[nodiscard]] constexpr auto combine_unicode_surrogates(
    char16_t high,
    char16_t low) noexcept -> char32_t
{
    const auto high_value =
        static_cast<char32_t>(high) - unicode_high_surrogate_first;
    const auto low_value =
        static_cast<char32_t>(low) - unicode_low_surrogate_first;
    return unicode_supplementary_first + ((high_value << 10) | low_value);
}

} // namespace xer::detail

#endif /* XER_BITS_UNICODE_COMMON_H_INCLUDED_ */

/**
 * @file xer/bits/strto_common.h
 * @brief Shared helper functions for string-to-number conversions.
 */

#pragma once

#ifndef XER_BITS_STRTO_COMMON_H_INCLUDED_
#define XER_BITS_STRTO_COMMON_H_INCLUDED_

#include <concepts>
#include <type_traits>

#include <xer/bits/common.h>

namespace xer::detail {

template<typename CharT>
concept strto_character =
    std::same_as<std::remove_cv_t<CharT>, char> ||
    std::same_as<std::remove_cv_t<CharT>, char8_t> ||
    std::same_as<std::remove_cv_t<CharT>, wchar_t> ||
    std::same_as<std::remove_cv_t<CharT>, char16_t> ||
    std::same_as<std::remove_cv_t<CharT>, char32_t>;

template<strto_character CharT>
[[nodiscard]] constexpr auto ascii_char(char ch) noexcept -> std::remove_cv_t<CharT>
{
    return static_cast<std::remove_cv_t<CharT>>(ch);
}

/**
 * @brief Checks whether the character is an ASCII whitespace.
 *
 * @param ch Character to test.
 * @return true if the character is an ASCII whitespace.
 */
template<strto_character CharT>
[[nodiscard]] constexpr auto is_ascii_space(CharT ch) noexcept -> bool
{
    switch (static_cast<std::remove_cv_t<CharT>>(ch)) {
        case ascii_char<CharT>(' '):
        case ascii_char<CharT>('\f'):
        case ascii_char<CharT>('\n'):
        case ascii_char<CharT>('\r'):
        case ascii_char<CharT>('\t'):
        case ascii_char<CharT>('\v'):
            return true;
        default:
            return false;
    }
}

/**
 * @brief Converts an ASCII digit or alphabetic digit to its numeric value.
 *
 * @param ch Character to convert.
 * @return Digit value, or -1 if the character is not a supported digit.
 */
template<strto_character CharT>
[[nodiscard]] constexpr auto ascii_digit_value(CharT ch) noexcept -> int
{
    const auto value = static_cast<std::remove_cv_t<CharT>>(ch);

    if (value >= ascii_char<CharT>('0') && value <= ascii_char<CharT>('9')) {
        return static_cast<int>(value - ascii_char<CharT>('0'));
    }

    if (value >= ascii_char<CharT>('a') && value <= ascii_char<CharT>('z')) {
        return 10 + static_cast<int>(value - ascii_char<CharT>('a'));
    }

    if (value >= ascii_char<CharT>('A') && value <= ascii_char<CharT>('Z')) {
        return 10 + static_cast<int>(value - ascii_char<CharT>('A'));
    }

    return -1;
}

template<strto_character CharT>
[[nodiscard]] constexpr auto ascii_to_lower(CharT ch) noexcept -> std::remove_cv_t<CharT>
{
    const auto value = static_cast<std::remove_cv_t<CharT>>(ch);

    if (value >= ascii_char<CharT>('A') && value <= ascii_char<CharT>('Z')) {
        return static_cast<std::remove_cv_t<CharT>>(
            value - ascii_char<CharT>('A') + ascii_char<CharT>('a'));
    }

    return value;
}

template<strto_character CharT, strto_character KeywordCharT>
[[nodiscard]] constexpr auto ascii_iequal(CharT lhs, KeywordCharT rhs) noexcept -> bool
{
    return ascii_to_lower(lhs) == ascii_to_lower(static_cast<std::remove_cv_t<CharT>>(rhs));
}

} // namespace xer::detail

#endif /* XER_BITS_STRTO_COMMON_H_INCLUDED_ */

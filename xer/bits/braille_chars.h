/**
 * @file xer/bits/braille_chars.h
 * @brief Basic one-character braille conversion helpers.
 */

#pragma once

#ifndef XER_BITS_BRAILLE_CHARS_H_INCLUDED_
#define XER_BITS_BRAILLE_CHARS_H_INCLUDED_

#include <array>
#include <expected>
#include <string_view>

#include <xer/error.h>

namespace xer::braille {

namespace detail {

inline constexpr std::array<std::u8string_view, 26> alpha_braille_table = {
    u8"⠁", // a
    u8"⠃", // b
    u8"⠉", // c
    u8"⠙", // d
    u8"⠑", // e
    u8"⠋", // f
    u8"⠛", // g
    u8"⠓", // h
    u8"⠊", // i
    u8"⠚", // j
    u8"⠅", // k
    u8"⠇", // l
    u8"⠍", // m
    u8"⠝", // n
    u8"⠕", // o
    u8"⠏", // p
    u8"⠟", // q
    u8"⠗", // r
    u8"⠎", // s
    u8"⠞", // t
    u8"⠥", // u
    u8"⠧", // v
    u8"⠺", // w
    u8"⠭", // x
    u8"⠽", // y
    u8"⠵", // z
};

} // namespace detail

/**
 * @brief Converts one alphabetic character to a braille cell.
 *
 * This function assumes that the caller has already emitted any required
 * alphabetic, capital, or other mode indicator. Uppercase and lowercase letters
 * are mapped to the same braille cell.
 *
 * @param c ASCII alphabetic character.
 * @return Corresponding braille cell, or invalid_argument for unsupported input.
 */
[[nodiscard]] constexpr auto alpha_to_braille(char32_t c)
    -> result<std::u8string_view>
{
    if (c >= U'A' && c <= U'Z') {
        c = static_cast<char32_t>(c - U'A' + U'a');
    }

    if (c < U'a' || c > U'z') {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return detail::alpha_braille_table[static_cast<std::size_t>(c - U'a')];
}

/**
 * @brief Converts one digit character to a braille cell.
 *
 * This function assumes that the caller has already emitted any required
 * numeric indicator. Digits 1 to 9 are mapped to the same cells as a to i, and
 * digit 0 is mapped to the same cell as j.
 *
 * @param c ASCII digit character.
 * @return Corresponding braille cell, or invalid_argument for unsupported input.
 */
[[nodiscard]] constexpr auto digit_to_braille(char32_t c)
    -> result<std::u8string_view>
{
    if (c >= U'1' && c <= U'9') {
        return detail::alpha_braille_table[static_cast<std::size_t>(c - U'1')];
    }

    if (c == U'0') {
        return detail::alpha_braille_table[9];
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

/**
 * @brief Converts one alphanumeric character to a braille cell.
 *
 * This function is a small dispatcher for alpha_to_braille and
 * digit_to_braille. It does not emit alphabetic, capital, numeric, or other
 * mode indicators.
 *
 * @param c ASCII alphanumeric character.
 * @return Corresponding braille cell, or invalid_argument for unsupported input.
 */
[[nodiscard]] constexpr auto alnum_to_braille(char32_t c)
    -> result<std::u8string_view>
{
    if ((c >= U'A' && c <= U'Z') || (c >= U'a' && c <= U'z')) {
        return alpha_to_braille(c);
    }

    if (c >= U'0' && c <= U'9') {
        return digit_to_braille(c);
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

/**
 * @brief Converts one English braille punctuation character to a braille cell.
 *
 * This function maps the basic Grade 1 English braille punctuation marks. It
 * does not infer the position of ASCII quotation marks; use U+201C or U+201D
 * when an opening or closing quotation mark must be distinguished.
 *
 * @param c Punctuation character.
 * @return Corresponding braille cell, or invalid_argument for unsupported input.
 */
[[nodiscard]] constexpr auto punct_to_braille(char32_t c)
    -> result<std::u8string_view>
{
    switch (c) {
    case U',':
        return u8"⠂";
    case U';':
        return u8"⠆";
    case U':':
        return u8"⠒";
    case U'.':
        return u8"⠲";
    case U'!':
        return u8"⠖";
    case U'(':
    case U')':
        return u8"⠶";
    case U'?':
    case U'“':
        return u8"⠦";
    case U'*':
        return u8"⠔";
    case U'”':
        return u8"⠴";
    case U'\'':
        return u8"⠄";
    case U'-':
    case U'‐':
        return u8"⠤";
    default:
        return std::unexpected(make_error(error_t::invalid_argument));
    }
}

} // namespace xer::braille

#endif /* XER_BITS_BRAILLE_CHARS_H_INCLUDED_ */

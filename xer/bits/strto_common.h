/**
 * @file xer/bits/strto_common.h
 * @brief Shared helper functions for string-to-number conversions.
 */

#pragma once

#ifndef XER_BITS_STRTO_COMMON_H_INCLUDED_
#define XER_BITS_STRTO_COMMON_H_INCLUDED_

#include <xer/bits/common.h>

namespace xer::detail {

/**
 * @brief Checks whether the character is an ASCII whitespace.
 *
 * @param ch Character to test.
 * @return true if the character is an ASCII whitespace.
 */
[[nodiscard]] constexpr bool is_ascii_space(char8_t ch) noexcept
{
    switch (ch) {
        case u8' ':
        case u8'\f':
        case u8'\n':
        case u8'\r':
        case u8'\t':
        case u8'\v':
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
[[nodiscard]] constexpr int ascii_digit_value(char8_t ch) noexcept
{
    if (ch >= u8'0' && ch <= u8'9') {
        return static_cast<int>(ch - u8'0');
    }

    if (ch >= u8'a' && ch <= u8'z') {
        return 10 + static_cast<int>(ch - u8'a');
    }

    if (ch >= u8'A' && ch <= u8'Z') {
        return 10 + static_cast<int>(ch - u8'A');
    }

    return -1;
}

} // namespace xer::detail

#endif /* XER_BITS_STRTO_COMMON_H_INCLUDED_ */

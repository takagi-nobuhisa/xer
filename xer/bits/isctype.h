/**
 * @file xer/bits/isctype.h
 * @brief ASCII-only character classification functions.
 */

#pragma once

#ifndef XER_BITS_ISCTYPE_H_INCLUDED_
#define XER_BITS_ISCTYPE_H_INCLUDED_

#include <xer/bits/common.h>

namespace xer {

/**
 * @brief Identifiers for ASCII-only character classes.
 */
enum class ctype_id {
    alpha,
    digit,
    alnum,
    lower,
    upper,
    space,
    blank,
    cntrl,
    print,
    graph,
    punct,
    xdigit,
    ascii,
    octal,
    binary,
};

/**
 * @brief Returns whether the code point is an ASCII character.
 *
 * @param c Code point to test.
 * @return True if the code point is in the ASCII range.
 */
[[nodiscard]] constexpr auto isascii(char32_t c) noexcept -> bool {
    return c >= U'\0' && c <= U'\x7f';
}

/**
 * @brief Returns whether the code point is an ASCII uppercase letter.
 *
 * @param c Code point to test.
 * @return True if the code point is in the range 'A'..'Z'.
 */
[[nodiscard]] constexpr auto isupper(char32_t c) noexcept -> bool {
    return c >= U'A' && c <= U'Z';
}

/**
 * @brief Returns whether the code point is an ASCII lowercase letter.
 *
 * @param c Code point to test.
 * @return True if the code point is in the range 'a'..'z'.
 */
[[nodiscard]] constexpr auto islower(char32_t c) noexcept -> bool {
    return c >= U'a' && c <= U'z';
}

/**
 * @brief Returns whether the code point is an ASCII decimal digit.
 *
 * @param c Code point to test.
 * @return True if the code point is in the range '0'..'9'.
 */
[[nodiscard]] constexpr auto isdigit(char32_t c) noexcept -> bool {
    return c >= U'0' && c <= U'9';
}

/**
 * @brief Returns whether the code point is an ASCII alphabetic letter.
 *
 * @param c Code point to test.
 * @return True if the code point is an ASCII letter.
 */
[[nodiscard]] constexpr auto isalpha(char32_t c) noexcept -> bool {
    return isupper(c) || islower(c);
}

/**
 * @brief Returns whether the code point is an ASCII alphanumeric character.
 *
 * @param c Code point to test.
 * @return True if the code point is an ASCII letter or digit.
 */
[[nodiscard]] constexpr auto isalnum(char32_t c) noexcept -> bool {
    return isalpha(c) || isdigit(c);
}

/**
 * @brief Returns whether the code point is an ASCII horizontal blank.
 *
 * @param c Code point to test.
 * @return True if the code point is space or horizontal tab.
 */
[[nodiscard]] constexpr auto isblank(char32_t c) noexcept -> bool {
    return c == U' ' || c == U'\t';
}

/**
 * @brief Returns whether the code point is an ASCII whitespace character.
 *
 * @param c Code point to test.
 * @return True if the code point is one of the C locale whitespace characters.
 */
[[nodiscard]] constexpr auto isspace(char32_t c) noexcept -> bool {
    return c == U' ' || (c >= U'\t' && c <= U'\r');
}

/**
 * @brief Returns whether the code point is an ASCII control character.
 *
 * @param c Code point to test.
 * @return True if the code point is in the ASCII control range.
 */
[[nodiscard]] constexpr auto iscntrl(char32_t c) noexcept -> bool {
    return (c >= U'\0' && c <= U'\x1f') || c == U'\x7f';
}

/**
 * @brief Returns whether the code point is an ASCII printable character.
 *
 * @param c Code point to test.
 * @return True if the code point is in the range 0x20..0x7E.
 */
[[nodiscard]] constexpr auto isprint(char32_t c) noexcept -> bool {
    return c >= U'\x20' && c <= U'\x7e';
}

/**
 * @brief Returns whether the code point is an ASCII graphic character.
 *
 * @param c Code point to test.
 * @return True if the code point is in the range 0x21..0x7E.
 */
[[nodiscard]] constexpr auto isgraph(char32_t c) noexcept -> bool {
    return c >= U'\x21' && c <= U'\x7e';
}

/**
 * @brief Returns whether the code point is an ASCII hexadecimal digit.
 *
 * @param c Code point to test.
 * @return True if the code point is 0-9, A-F, or a-f.
 */
[[nodiscard]] constexpr auto isxdigit(char32_t c) noexcept -> bool {
    return isdigit(c) ||
           (c >= U'A' && c <= U'F') ||
           (c >= U'a' && c <= U'f');
}

/**
 * @brief Returns whether the code point is an ASCII octal digit.
 *
 * @param c Code point to test.
 * @return True if the code point is in the range '0'..'7'.
 */
[[nodiscard]] constexpr auto isoctal(char32_t c) noexcept -> bool {
    return c >= U'0' && c <= U'7';
}

/**
 * @brief Returns whether the code point is an ASCII binary digit.
 *
 * @param c Code point to test.
 * @return True if the code point is '0' or '1'.
 */
[[nodiscard]] constexpr auto isbinary(char32_t c) noexcept -> bool {
    return c == U'0' || c == U'1';
}

/**
 * @brief Returns whether the code point is an ASCII punctuation character.
 *
 * @param c Code point to test.
 * @return True if the code point is printable and neither alnum nor space.
 */
[[nodiscard]] constexpr auto ispunct(char32_t c) noexcept -> bool {
    return isgraph(c) && !isalnum(c);
}

/**
 * @brief Classifies a code point by the specified ASCII-only class identifier.
 *
 * @param c Code point to test.
 * @param id Character class identifier.
 * @return Classification result.
 */
[[nodiscard]] constexpr auto isctype(char32_t c, ctype_id id) noexcept -> bool {
    switch (id) {
        case ctype_id::alpha:
            return isalpha(c);
        case ctype_id::digit:
            return isdigit(c);
        case ctype_id::alnum:
            return isalnum(c);
        case ctype_id::lower:
            return islower(c);
        case ctype_id::upper:
            return isupper(c);
        case ctype_id::space:
            return isspace(c);
        case ctype_id::blank:
            return isblank(c);
        case ctype_id::cntrl:
            return iscntrl(c);
        case ctype_id::print:
            return isprint(c);
        case ctype_id::graph:
            return isgraph(c);
        case ctype_id::punct:
            return ispunct(c);
        case ctype_id::xdigit:
            return isxdigit(c);
        case ctype_id::ascii:
            return isascii(c);
        case ctype_id::octal:
            return isoctal(c);
        case ctype_id::binary:
            return isbinary(c);
    }

    return false;
}

} // namespace xer

#endif /* XER_BITS_ISCTYPE_H_INCLUDED_ */

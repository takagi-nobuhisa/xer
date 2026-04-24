/**
 * @file xer/bits/isctype.h
 * @brief ASCII and Latin-1 character classification functions.
 */

#pragma once

#ifndef XER_BITS_ISCTYPE_H_INCLUDED_
#define XER_BITS_ISCTYPE_H_INCLUDED_

#include <xer/bits/common.h>
#include <xer/bits/character_width.h>

namespace xer {

/**
 * @brief Identifiers for character classes.
 *
 * ASCII-only identifiers keep their original meanings.
 * latin1_* identifiers extend classification to Latin-1 characters and also
 * include the ASCII range where applicable.
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
    latin1_alpha,
    latin1_upper,
    latin1_lower,
    latin1_alnum,
    latin1_graph,
    latin1_print,
    fullwidth_kana,
    halfwidth_kana,
    fullwidth_digit,
    halfwidth_digit,
    fullwidth_alpha,
    halfwidth_alpha,
    fullwidth_punct,
    halfwidth_punct,
    fullwidth_space,
    halfwidth_space,
    fullwidth_graph,
    halfwidth_graph,
    fullwidth_print,
    halfwidth_print,
    fullwidth,
    halfwidth,
};

/**
 * @brief Returns whether the code point is an ASCII character.
 *
 * @param c Code point to test.
 * @return True if the code point is in the ASCII range.
 */
[[nodiscard]] constexpr auto isascii(char32_t c) noexcept -> bool
{
    return c >= U'\0' && c <= U'\x7f';
}

/**
 * @brief Returns whether the code point is an ASCII uppercase letter.
 *
 * @param c Code point to test.
 * @return True if the code point is in the range 'A'..'Z'.
 */
[[nodiscard]] constexpr auto isupper(char32_t c) noexcept -> bool
{
    return c >= U'A' && c <= U'Z';
}

/**
 * @brief Returns whether the code point is an ASCII lowercase letter.
 *
 * @param c Code point to test.
 * @return True if the code point is in the range 'a'..'z'.
 */
[[nodiscard]] constexpr auto islower(char32_t c) noexcept -> bool
{
    return c >= U'a' && c <= U'z';
}

/**
 * @brief Returns whether the code point is an ASCII decimal digit.
 *
 * @param c Code point to test.
 * @return True if the code point is in the range '0'..'9'.
 */
[[nodiscard]] constexpr auto isdigit(char32_t c) noexcept -> bool
{
    return c >= U'0' && c <= U'9';
}

/**
 * @brief Returns whether the code point is an ASCII alphabetic letter.
 *
 * @param c Code point to test.
 * @return True if the code point is an ASCII letter.
 */
[[nodiscard]] constexpr auto isalpha(char32_t c) noexcept -> bool
{
    return isupper(c) || islower(c);
}

/**
 * @brief Returns whether the code point is an ASCII alphanumeric character.
 *
 * @param c Code point to test.
 * @return True if the code point is an ASCII letter or digit.
 */
[[nodiscard]] constexpr auto isalnum(char32_t c) noexcept -> bool
{
    return isalpha(c) || isdigit(c);
}

/**
 * @brief Returns whether the code point is an ASCII horizontal blank.
 *
 * @param c Code point to test.
 * @return True if the code point is space or horizontal tab.
 */
[[nodiscard]] constexpr auto isblank(char32_t c) noexcept -> bool
{
    return c == U' ' || c == U'\t';
}

/**
 * @brief Returns whether the code point is an ASCII whitespace character.
 *
 * @param c Code point to test.
 * @return True if the code point is one of the C locale whitespace characters.
 */
[[nodiscard]] constexpr auto isspace(char32_t c) noexcept -> bool
{
    return c == U' ' || (c >= U'\t' && c <= U'\r');
}

/**
 * @brief Returns whether the code point is an ASCII control character.
 *
 * @param c Code point to test.
 * @return True if the code point is in the ASCII control range.
 */
[[nodiscard]] constexpr auto iscntrl(char32_t c) noexcept -> bool
{
    return (c >= U'\0' && c <= U'\x1f') || c == U'\x7f';
}

/**
 * @brief Returns whether the code point is an ASCII printable character.
 *
 * @param c Code point to test.
 * @return True if the code point is in the range 0x20..0x7E.
 */
[[nodiscard]] constexpr auto isprint(char32_t c) noexcept -> bool
{
    return c >= U'\x20' && c <= U'\x7e';
}

/**
 * @brief Returns whether the code point is an ASCII graphic character.
 *
 * @param c Code point to test.
 * @return True if the code point is in the range 0x21..0x7E.
 */
[[nodiscard]] constexpr auto isgraph(char32_t c) noexcept -> bool
{
    return c >= U'\x21' && c <= U'\x7e';
}

/**
 * @brief Returns whether the code point is an ASCII hexadecimal digit.
 *
 * @param c Code point to test.
 * @return True if the code point is 0-9, A-F, or a-f.
 */
[[nodiscard]] constexpr auto isxdigit(char32_t c) noexcept -> bool
{
    return isdigit(c) || (c >= U'A' && c <= U'F') || (c >= U'a' && c <= U'f');
}

/**
 * @brief Returns whether the code point is an ASCII octal digit.
 *
 * @param c Code point to test.
 * @return True if the code point is in the range '0'..'7'.
 */
[[nodiscard]] constexpr auto isoctal(char32_t c) noexcept -> bool
{
    return c >= U'0' && c <= U'7';
}

/**
 * @brief Returns whether the code point is an ASCII binary digit.
 *
 * @param c Code point to test.
 * @return True if the code point is '0' or '1'.
 */
[[nodiscard]] constexpr auto isbinary(char32_t c) noexcept -> bool
{
    return c == U'0' || c == U'1';
}

/**
 * @brief Returns whether the code point is an ASCII punctuation character.
 *
 * @param c Code point to test.
 * @return True if the code point is printable and neither alnum nor space.
 */
[[nodiscard]] constexpr auto ispunct(char32_t c) noexcept -> bool
{
    return isgraph(c) && !isalnum(c);
}

/**
 * @brief Returns whether the code point is a Latin-1 uppercase letter.
 *
 * ASCII uppercase letters are included.
 *
 * @param c Code point to test.
 * @return True if the code point is an uppercase Latin-1 letter.
 */
[[nodiscard]] constexpr auto islatin1_upper(char32_t c) noexcept -> bool
{
    return isupper(c) || (c >= U'\u00c0' && c <= U'\u00d6') ||
           (c >= U'\u00d8' && c <= U'\u00de');
}

/**
 * @brief Returns whether the code point is a Latin-1 lowercase letter.
 *
 * ASCII lowercase letters are included.
 *
 * @param c Code point to test.
 * @return True if the code point is a lowercase Latin-1 letter.
 */
[[nodiscard]] constexpr auto islatin1_lower(char32_t c) noexcept -> bool
{
    return islower(c) || c == U'\u00aa' || c == U'\u00ba' || c == U'\u00df' ||
           (c >= U'\u00e0' && c <= U'\u00f6') ||
           (c >= U'\u00f8' && c <= U'\u00ff');
}

/**
 * @brief Returns whether the code point is a Latin-1 alphabetic letter.
 *
 * ASCII letters are included.
 *
 * @param c Code point to test.
 * @return True if the code point is an alphabetic Latin-1 character.
 */
[[nodiscard]] constexpr auto islatin1_alpha(char32_t c) noexcept -> bool
{
    return islatin1_upper(c) || islatin1_lower(c);
}

/**
 * @brief Returns whether the code point is a Latin-1 alphanumeric character.
 *
 * ASCII digits are included.
 *
 * @param c Code point to test.
 * @return True if the code point is a Latin-1 letter or an ASCII digit.
 */
[[nodiscard]] constexpr auto islatin1_alnum(char32_t c) noexcept -> bool
{
    return islatin1_alpha(c) || isdigit(c);
}

/**
 * @brief Returns whether the code point is a printable Latin-1 character.
 *
 * ASCII printable characters are included. In the Latin-1 supplement block,
 * U+00A0..U+00FF are treated as printable.
 *
 * @param c Code point to test.
 * @return True if the code point is printable in Latin-1.
 */
[[nodiscard]] constexpr auto islatin1_print(char32_t c) noexcept -> bool
{
    return isprint(c) || (c >= U'\u00a0' && c <= U'\u00ff');
}

/**
 * @brief Returns whether the code point is a graphic Latin-1 character.
 *
 * ASCII graphic characters are included. In the Latin-1 supplement block,
 * U+00A1..U+00FF are treated as graphic.
 *
 * @param c Code point to test.
 * @return True if the code point is graphic in Latin-1.
 */
[[nodiscard]] constexpr auto islatin1_graph(char32_t c) noexcept -> bool
{
    return isgraph(c) || (c >= U'\u00a1' && c <= U'\u00ff');
}

/**
 * @brief Classifies a code point by the specified character class identifier.
 *
 * ASCII-only identifiers keep their original meanings.
 * latin1_* identifiers include ASCII where appropriate and extend
 * classification to Latin-1 characters.
 *
 * @param c Code point to test.
 * @param id Character class identifier.
 * @return Classification result.
 */
[[nodiscard]] constexpr auto isctype(char32_t c, ctype_id id) noexcept -> bool
{
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
        case ctype_id::latin1_alpha:
            return islatin1_alpha(c);
        case ctype_id::latin1_upper:
            return islatin1_upper(c);
        case ctype_id::latin1_lower:
            return islatin1_lower(c);
        case ctype_id::latin1_alnum:
            return islatin1_alnum(c);
        case ctype_id::latin1_graph:
            return islatin1_graph(c);
        case ctype_id::latin1_print:
            return islatin1_print(c);
        case ctype_id::fullwidth_kana:
            return detail::is_fullwidth_kana(c);
        case ctype_id::halfwidth_kana:
            return detail::is_halfwidth_kana(c);
        case ctype_id::fullwidth_digit:
            return detail::is_fullwidth_digit(c);
        case ctype_id::halfwidth_digit:
            return detail::is_halfwidth_digit(c);
        case ctype_id::fullwidth_alpha:
            return detail::is_fullwidth_alpha(c);
        case ctype_id::halfwidth_alpha:
            return detail::is_halfwidth_alpha(c);
        case ctype_id::fullwidth_punct:
            return detail::is_fullwidth_punct(c);
        case ctype_id::halfwidth_punct:
            return detail::is_halfwidth_punct(c);
        case ctype_id::fullwidth_space:
            return detail::is_fullwidth_space(c);
        case ctype_id::halfwidth_space:
            return detail::is_halfwidth_space(c);
        case ctype_id::fullwidth_graph:
            return detail::is_fullwidth_graph(c);
        case ctype_id::halfwidth_graph:
            return detail::is_halfwidth_graph(c);
        case ctype_id::fullwidth_print:
            return detail::is_fullwidth_print(c);
        case ctype_id::halfwidth_print:
            return detail::is_halfwidth_print(c);
        case ctype_id::fullwidth:
            return detail::is_fullwidth(c);
        case ctype_id::halfwidth:
            return detail::is_halfwidth(c);
    }

    return false;
}

} // namespace xer

#endif /* XER_BITS_ISCTYPE_H_INCLUDED_ */

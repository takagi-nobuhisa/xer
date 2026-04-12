/**
 * @file xer/bits/totrans.h
 * @brief ASCII and Latin-1 character conversion functions.
 */

#pragma once

#ifndef XER_BITS_TOTRANS_H_INCLUDED_
#define XER_BITS_TOTRANS_H_INCLUDED_

#include <expected>

#include <xer/bits/common.h>
#include <xer/bits/isctype.h>
#include <xer/error.h>

namespace xer {

/**
 * @brief Identifiers for character transformations.
 *
 * ASCII-only identifiers keep their original meanings.
 * latin1_* identifiers extend transformation to Latin-1 characters and also
 * include the ASCII range where applicable.
 */
enum class ctrans_id {
    lower,
    upper,
    latin1_lowercase,
    latin1_uppercase,
};

namespace detail {

/**
 * @brief Returns whether the code point is supported by Latin-1 conversion.
 *
 * ASCII and Latin-1 code points are supported. In addition, U+1E9E
 * (LATIN CAPITAL LETTER SHARP S) is supported as a special case so that
 * U+00DF (LATIN SMALL LETTER SHARP S) can be converted reversibly.
 *
 * @param c Code point to test.
 * @return True if the code point is supported by Latin-1 conversion.
 */
[[nodiscard]] constexpr auto islatin1_conversion_target(char32_t c) noexcept
    -> bool
{
    return xer::isascii(c) || (c >= U'\u00a0' && c <= U'\u00ff') ||
           c == U'\u1e9e';
}

/**
 * @brief Converts an ASCII character to lowercase.
 *
 * Non-ASCII code points are rejected.
 *
 * @param c Code point to convert.
 * @return Converted character on success.
 */
[[nodiscard]] constexpr auto tolower_ascii(char32_t c)
    -> std::expected<char32_t, error<void>>
{
    if (!xer::isascii(c)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (xer::isupper(c)) {
        return static_cast<char32_t>(c - U'A' + U'a');
    }

    return c;
}

/**
 * @brief Converts an ASCII character to uppercase.
 *
 * Non-ASCII code points are rejected.
 *
 * @param c Code point to convert.
 * @return Converted character on success.
 */
[[nodiscard]] constexpr auto toupper_ascii(char32_t c)
    -> std::expected<char32_t, error<void>>
{
    if (!xer::isascii(c)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (xer::islower(c)) {
        return static_cast<char32_t>(c - U'a' + U'A');
    }

    return c;
}

/**
 * @brief Converts a Latin-1 character to lowercase.
 *
 * ASCII is included. U+1E9E maps to U+00DF as a special case.
 * Code points outside the supported Latin-1 conversion set are rejected.
 *
 * @param c Code point to convert.
 * @return Converted character on success.
 */
[[nodiscard]] constexpr auto tolower_latin1(char32_t c)
    -> std::expected<char32_t, error<void>>
{
    if (!islatin1_conversion_target(c)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (c == U'\u1e9e') {
        return U'\u00df';
    }

    if (xer::isupper(c)) {
        return static_cast<char32_t>(c - U'A' + U'a');
    }

    if (c >= U'\u00c0' && c <= U'\u00d6') {
        return static_cast<char32_t>(c + 0x20);
    }

    if (c >= U'\u00d8' && c <= U'\u00de') {
        return static_cast<char32_t>(c + 0x20);
    }

    return c;
}

/**
 * @brief Converts a Latin-1 character to uppercase.
 *
 * ASCII is included. U+00DF maps to U+1E9E as a special case.
 * Code points outside the supported Latin-1 conversion set are rejected.
 *
 * @param c Code point to convert.
 * @return Converted character on success.
 */
[[nodiscard]] constexpr auto toupper_latin1(char32_t c)
    -> std::expected<char32_t, error<void>>
{
    if (!islatin1_conversion_target(c)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (c == U'\u00df') {
        return U'\u1e9e';
    }

    if (xer::islower(c)) {
        return static_cast<char32_t>(c - U'a' + U'A');
    }

    if (c >= U'\u00e0' && c <= U'\u00f6') {
        return static_cast<char32_t>(c - 0x20);
    }

    if (c >= U'\u00f8' && c <= U'\u00fe') {
        return static_cast<char32_t>(c - 0x20);
    }

    return c;
}

} // namespace detail

/**
 * @brief Converts an ASCII character to lowercase.
 *
 * - If the code point is an ASCII uppercase letter, the lowercase letter is
 *   returned.
 * - If the code point is an ASCII character but not uppercase, it is returned
 *   unchanged.
 * - If the code point is non-ASCII, an error is returned.
 *
 * @param c Code point to convert.
 * @return Converted character on success.
 */
[[nodiscard]] constexpr auto tolower(char32_t c)
    -> std::expected<char32_t, error<void>>
{
    return detail::tolower_ascii(c);
}

/**
 * @brief Converts an ASCII character to uppercase.
 *
 * - If the code point is an ASCII lowercase letter, the uppercase letter is
 *   returned.
 * - If the code point is an ASCII character but not lowercase, it is returned
 *   unchanged.
 * - If the code point is non-ASCII, an error is returned.
 *
 * @param c Code point to convert.
 * @return Converted character on success.
 */
[[nodiscard]] constexpr auto toupper(char32_t c)
    -> std::expected<char32_t, error<void>>
{
    return detail::toupper_ascii(c);
}

/**
 * @brief Converts a code point according to the specified transformation.
 *
 * ASCII-only identifiers reject non-ASCII code points.
 * Latin-1 identifiers include ASCII and support Latin-1 conversion.
 *
 * @param c Code point to convert.
 * @param id Transformation identifier.
 * @return Converted character on success.
 */
[[nodiscard]] constexpr auto toctrans(char32_t c, ctrans_id id)
    -> std::expected<char32_t, error<void>>
{
    switch (id) {
        case ctrans_id::lower:
            return detail::tolower_ascii(c);
        case ctrans_id::upper:
            return detail::toupper_ascii(c);
        case ctrans_id::latin1_lowercase:
            return detail::tolower_latin1(c);
        case ctrans_id::latin1_uppercase:
            return detail::toupper_latin1(c);
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

} // namespace xer

#endif /* XER_BITS_TOTRANS_H_INCLUDED_ */

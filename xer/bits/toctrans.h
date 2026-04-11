/**
 * @file xer/bits/toctrans.h
 * @brief ASCII-only character conversion functions.
 */

#pragma once

#ifndef XER_BITS_TOCTRANS_H_INCLUDED_
#define XER_BITS_TOCTRANS_H_INCLUDED_

#include <expected>

#include <xer/bits/common.h>
#include <xer/error.h>

namespace xer {

/**
 * @brief Identifiers for ASCII-only character conversions.
 */
enum class ctrans_id {
    lower,
    upper,
};

/**
 * @brief Converts an ASCII letter to lowercase.
 *
 * Non-ASCII input is rejected as an error.
 *
 * @param c Code point to convert.
 * @return Converted code point on success.
 */
[[nodiscard]] constexpr std::expected<char32_t, error<void>>
tolower(char32_t c) noexcept {
    if (c > U'\x7f') {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (c >= U'A' && c <= U'Z') {
        return static_cast<char32_t>(c - U'A' + U'a');
    }

    return c;
}

/**
 * @brief Converts an ASCII letter to uppercase.
 *
 * Non-ASCII input is rejected as an error.
 *
 * @param c Code point to convert.
 * @return Converted code point on success.
 */
[[nodiscard]] constexpr std::expected<char32_t, error<void>>
toupper(char32_t c) noexcept {
    if (c > U'\x7f') {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (c >= U'a' && c <= U'z') {
        return static_cast<char32_t>(c - U'a' + U'A');
    }

    return c;
}

/**
 * @brief Converts a code point by the specified ASCII-only conversion identifier.
 *
 * @param c Code point to convert.
 * @param id Conversion identifier.
 * @return Converted code point on success.
 */
[[nodiscard]] constexpr std::expected<char32_t, error<void>>
toctrans(char32_t c, ctrans_id id) noexcept {
    switch (id) {
        case ctrans_id::lower:
            return tolower(c);
        case ctrans_id::upper:
            return toupper(c);
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

} // namespace xer

#endif /* XER_BITS_TOCTRANS_H_INCLUDED_ */

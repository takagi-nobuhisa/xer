/**
 * @file xer/bits/utf8_char_encode.h
 * @brief Internal UTF-8 single-character encoding helper.
 */

#pragma once

#ifndef XER_BITS_UTF8_CHAR_ENCODE_H_INCLUDED_
#define XER_BITS_UTF8_CHAR_ENCODE_H_INCLUDED_

#include <expected>
#include <string>

#include <xer/bits/common.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Encodes one Unicode scalar value into UTF-8.
 *
 * @param value Source Unicode scalar value.
 * @return UTF-8 encoded string on success.
 * @return error<void> on failure.
 */
[[nodiscard]] inline std::expected<std::u8string, error<void>> encode_utf8_char(
    char32_t value) noexcept
{
    if (value > 0x10ffffu) {
        return std::unexpected(make_error(error_t::ilseq));
    }

    if (value >= 0xd800u && value <= 0xdfffu) {
        return std::unexpected(make_error(error_t::ilseq));
    }

    std::u8string result;

    if (value <= 0x7fu) {
        result.push_back(static_cast<char8_t>(value));
        return result;
    }

    if (value <= 0x7ffu) {
        result.push_back(static_cast<char8_t>(0xc0u | ((value >> 6) & 0x1fu)));
        result.push_back(static_cast<char8_t>(0x80u | (value & 0x3fu)));
        return result;
    }

    if (value <= 0xffffu) {
        result.push_back(static_cast<char8_t>(0xe0u | ((value >> 12) & 0x0fu)));
        result.push_back(static_cast<char8_t>(0x80u | ((value >> 6) & 0x3fu)));
        result.push_back(static_cast<char8_t>(0x80u | (value & 0x3fu)));
        return result;
    }

    result.push_back(static_cast<char8_t>(0xf0u | ((value >> 18) & 0x07u)));
    result.push_back(static_cast<char8_t>(0x80u | ((value >> 12) & 0x3fu)));
    result.push_back(static_cast<char8_t>(0x80u | ((value >> 6) & 0x3fu)));
    result.push_back(static_cast<char8_t>(0x80u | (value & 0x3fu)));
    return result;
}


/**
 * @brief Appends one Unicode scalar value to a UTF-8 string.
 *
 * @param out Output UTF-8 string.
 * @param value Source Unicode scalar value.
 * @return Success or failure.
 */
[[nodiscard]] inline std::expected<void, error<void>> append_utf8_char(
    std::u8string& out,
    char32_t value) noexcept
{
    const auto encoded = encode_utf8_char(value);
    if (!encoded.has_value()) {
        return std::unexpected(encoded.error());
    }

    out += *encoded;
    return {};
}

} // namespace xer::detail

#endif /* XER_BITS_UTF8_CHAR_ENCODE_H_INCLUDED_ */

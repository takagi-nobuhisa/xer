/**
 * @file xer/bits/json_encode.h
 * @brief Internal JSON encoding implementation.
 */

#pragma once

#ifndef XER_BITS_JSON_ENCODE_H_INCLUDED_
#define XER_BITS_JSON_ENCODE_H_INCLUDED_

#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <system_error>

#include <xer/bits/advanced_encoding.h>
#include <xer/bits/common.h>
#include <xer/bits/json_value.h>
#include <xer/error.h>

namespace xer::detail {

inline void json_encode_append_utf8(std::u8string& out, char32_t code_point)
{
    const std::uint32_t packed = xer::advanced::utf32_to_packed_utf8(code_point);
    if (packed == xer::advanced::detail::invalid_packed_utf8) {
        return;
    }

    const std::uint8_t b1 = static_cast<std::uint8_t>(packed & 0xFFu);
    const std::uint8_t b2 = static_cast<std::uint8_t>((packed >> 8) & 0xFFu);
    const std::uint8_t b3 = static_cast<std::uint8_t>((packed >> 16) & 0xFFu);
    const std::uint8_t b4 = static_cast<std::uint8_t>((packed >> 24) & 0xFFu);

    if (b1 != 0) {
        out.push_back(static_cast<char8_t>(b1));
    }

    if (b2 != 0) {
        out.push_back(static_cast<char8_t>(b2));
    }

    if (b3 != 0) {
        out.push_back(static_cast<char8_t>(b3));
    }

    if (b4 != 0) {
        out.push_back(static_cast<char8_t>(b4));
    }
}

inline void json_encode_append_hex4(std::u8string& out, std::uint16_t value)
{
    constexpr char8_t hex[] = u8"0123456789ABCDEF";

    out.push_back(hex[(value >> 12) & 0x0Fu]);
    out.push_back(hex[(value >> 8) & 0x0Fu]);
    out.push_back(hex[(value >> 4) & 0x0Fu]);
    out.push_back(hex[value & 0x0Fu]);
}

[[nodiscard]] inline auto json_decode_utf8_code_point(
    std::u8string_view text,
    std::size_t& pos) -> result<char32_t>
{
    if (pos >= text.size()) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    const std::uint8_t b1 = static_cast<std::uint8_t>(text[pos++]);
    if (b1 <= 0x7Fu) {
        return static_cast<char32_t>(b1);
    }

    std::size_t extra = 0;
    if (b1 >= 0xC2u && b1 <= 0xDFu) {
        extra = 1;
    } else if (b1 >= 0xE0u && b1 <= 0xEFu) {
        extra = 2;
    } else if (b1 >= 0xF0u && b1 <= 0xF4u) {
        extra = 3;
    } else {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    if (pos + extra > text.size()) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    std::uint32_t packed = b1;
    for (std::size_t i = 0; i < extra; ++i) {
        const std::uint8_t byte = static_cast<std::uint8_t>(text[pos++]);
        packed |= static_cast<std::uint32_t>(byte) << ((i + 1u) * 8u);
    }

    const char32_t code_point = xer::advanced::packed_utf8_to_utf32(packed);
    if (code_point == xer::advanced::detail::invalid_utf32) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    return code_point;
}

[[nodiscard]] inline auto json_encode_string(std::u8string_view value) -> result<std::u8string>
{
    std::u8string out;
    out.push_back(u8'"');

    std::size_t pos = 0;
    while (pos < value.size()) {
        auto code_point = json_decode_utf8_code_point(value, pos);
        if (!code_point.has_value()) {
            return std::unexpected(code_point.error());
        }

        switch (*code_point) {
        case U'"':
            out.append(u8"\\\"");
            break;
        case U'\\':
            out.append(u8"\\\\");
            break;
        case U'\b':
            out.append(u8"\\b");
            break;
        case U'\f':
            out.append(u8"\\f");
            break;
        case U'\n':
            out.append(u8"\\n");
            break;
        case U'\r':
            out.append(u8"\\r");
            break;
        case U'\t':
            out.append(u8"\\t");
            break;
        default:
            if (*code_point < 0x20u) {
                out.append(u8"\\u");
                json_encode_append_hex4(out, static_cast<std::uint16_t>(*code_point));
            } else {
                json_encode_append_utf8(out, *code_point);
            }
            break;
        }
    }

    out.push_back(u8'"');
    return out;
}

[[nodiscard]] inline auto json_encode_number(double value) -> result<std::u8string>
{
    if (!std::isfinite(value)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (value == 0.0) {
        return std::u8string(u8"0");
    }

    char buffer[64];
    const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);
    if (result.ec != std::errc()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return std::u8string(reinterpret_cast<const char8_t*>(buffer),
                         static_cast<std::size_t>(result.ptr - buffer));
}

[[nodiscard]] inline auto json_encode_value(const json_value& value) -> result<std::u8string>
{
    if (value.is_null()) {
        return std::u8string(u8"null");
    }

    if (value.is_bool()) {
        return value.as_bool() ? std::u8string(u8"true") : std::u8string(u8"false");
    }

    if (value.is_number()) {
        return json_encode_number(value.as_number());
    }

    if (value.is_string()) {
        return json_encode_string(value.as_string());
    }

    if (value.is_array()) {
        std::u8string out;
        out.push_back(u8'[');

        bool first = true;
        for (const auto& element : value.as_array()) {
            auto encoded = json_encode_value(element);
            if (!encoded.has_value()) {
                return encoded;
            }

            if (!first) {
                out.push_back(u8',');
            }

            first = false;
            out.append(encoded->data(), encoded->size());
        }

        out.push_back(u8']');
        return out;
    }

    std::u8string out;
    out.push_back(u8'{');

    bool first = true;
    for (const auto& [key, element] : value.as_object()) {
        auto encoded_key = json_encode_string(key);
        if (!encoded_key.has_value()) {
            return std::unexpected(encoded_key.error());
        }

        auto encoded_value = json_encode_value(element);
        if (!encoded_value.has_value()) {
            return encoded_value;
        }

        if (!first) {
            out.push_back(u8',');
        }

        first = false;
        out.append(encoded_key->data(), encoded_key->size());
        out.push_back(u8':');
        out.append(encoded_value->data(), encoded_value->size());
    }

    out.push_back(u8'}');
    return out;
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Encodes a JSON value into a JSON text.
 *
 * The output is a compact UTF-8 JSON representation without extra whitespace.
 * Numbers are emitted from the stored double value.
 * Non-finite numbers are rejected.
 *
 * @param value JSON value to encode.
 * @return UTF-8 JSON text on success.
 */
[[nodiscard]] inline auto json_encode(const json_value& value) -> result<std::u8string>
{
    return detail::json_encode_value(value);
}

} // namespace xer

#endif /* XER_BITS_JSON_ENCODE_H_INCLUDED_ */

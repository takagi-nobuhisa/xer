/**
 * @file xer/bits/json_decode.h
 * @brief Internal JSON decoding implementation.
 */

#pragma once

#ifndef XER_BITS_JSON_DECODE_H_INCLUDED_
#define XER_BITS_JSON_DECODE_H_INCLUDED_

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <system_error>

#include <xer/bits/advanced_encoding.h>
#include <xer/bits/common.h>
#include <xer/bits/json_value.h>
#include <xer/error.h>
#include <xer/parse.h>

namespace xer::detail {

[[nodiscard]] inline auto json_is_whitespace(char8_t ch) noexcept -> bool
{
    return ch == u8' ' || ch == u8'\t' || ch == u8'\n' || ch == u8'\r';
}

[[nodiscard]] inline auto json_is_digit(char8_t ch) noexcept -> bool
{
    return ch >= u8'0' && ch <= u8'9';
}

inline void json_append_utf8(std::u8string& out, char32_t code_point)
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

class json_decoder {
public:
    explicit json_decoder(std::u8string_view text_) noexcept
        : text(text_), pos(0)
    {
    }

    [[nodiscard]] auto parse() -> result<json_value, parse_error_detail>
    {
        skip_whitespace();

        auto value = parse_value();
        if (!value.has_value()) {
            return value;
        }

        skip_whitespace();
        if (pos != text.size()) {
            return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
        }

        return value;
    }

private:
    [[nodiscard]] auto make_parse_error(
        parse_error_reason reason = parse_error_reason::invalid_syntax,
        std::size_t offset = static_cast<std::size_t>(-1),
        error_t code = error_t::invalid_argument) const -> error<parse_error_detail>
    {
        if (offset == static_cast<std::size_t>(-1)) {
            offset = pos;
        }

        std::size_t line = 1;
        std::size_t column = 1;
        for (std::size_t i = 0; i < offset && i < text.size(); ++i) {
            if (text[i] == u8'\n') {
                ++line;
                column = 1;
            } else {
                ++column;
            }
        }

        return xer::make_error<parse_error_detail>(
            code,
            parse_error_detail{offset, line, column, reason});
    }

    std::u8string_view text;
    std::size_t pos;

    void skip_whitespace() noexcept
    {
        while (pos < text.size() && json_is_whitespace(text[pos])) {
            ++pos;
        }
    }

    [[nodiscard]] auto parse_value() -> result<json_value, parse_error_detail>
    {
        if (pos >= text.size()) {
            return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
        }

        switch (text[pos]) {
        case u8'n':
            return parse_null();
        case u8't':
            return parse_true();
        case u8'f':
            return parse_false();
        case u8'"':
            return parse_string();
        case u8'[':
            return parse_array();
        case u8'{':
            return parse_object();
        default:
            if (text[pos] == u8'-' || json_is_digit(text[pos])) {
                return parse_number();
            }

            return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
        }
    }

    [[nodiscard]] auto parse_null() -> result<json_value, parse_error_detail>
    {
        if (match_literal(u8"null")) {
            return json_value(nullptr);
        }

        return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
    }

    [[nodiscard]] auto parse_true() -> result<json_value, parse_error_detail>
    {
        if (match_literal(u8"true")) {
            return json_value(true);
        }

        return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
    }

    [[nodiscard]] auto parse_false() -> result<json_value, parse_error_detail>
    {
        if (match_literal(u8"false")) {
            return json_value(false);
        }

        return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
    }

    [[nodiscard]] auto parse_number() -> result<json_value, parse_error_detail>
    {
        const std::size_t begin = pos;

        if (text[pos] == u8'-') {
            ++pos;
            if (pos >= text.size()) {
                return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
            }
        }

        if (text[pos] == u8'0') {
            ++pos;
        } else {
            if (!json_is_digit(text[pos])) {
                return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
            }

            while (pos < text.size() && json_is_digit(text[pos])) {
                ++pos;
            }
        }

        if (pos < text.size() && text[pos] == u8'.') {
            ++pos;
            if (pos >= text.size() || !json_is_digit(text[pos])) {
                return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
            }

            while (pos < text.size() && json_is_digit(text[pos])) {
                ++pos;
            }
        }

        if (pos < text.size() && (text[pos] == u8'e' || text[pos] == u8'E')) {
            ++pos;

            if (pos < text.size() && (text[pos] == u8'+' || text[pos] == u8'-')) {
                ++pos;
            }

            if (pos >= text.size() || !json_is_digit(text[pos])) {
                return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
            }

            while (pos < text.size() && json_is_digit(text[pos])) {
                ++pos;
            }
        }

        const char* first = reinterpret_cast<const char*>(text.data() + begin);
        const char* last = reinterpret_cast<const char*>(text.data() + pos);
        double value = 0.0;
        const auto convert_result = std::from_chars(first, last, value);
        if (convert_result.ec != std::errc() || convert_result.ptr != last) {
            return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
        }

        return json_value(value);
    }

    [[nodiscard]] auto parse_string() -> result<json_value, parse_error_detail>
    {
        if (text[pos] != u8'"') {
            return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
        }

        ++pos;
        std::u8string out;

        while (pos < text.size()) {
            const char8_t ch = text[pos++];

            if (ch == u8'"') {
                return json_value(std::move(out));
            }

            const std::uint8_t byte = static_cast<std::uint8_t>(ch);
            if (byte < 0x20u) {
                return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
            }

            if (ch == u8'\\') {
                auto escaped = parse_escape_sequence();
                if (!escaped.has_value()) {
                    return std::unexpected(escaped.error());
                }

                out.append(escaped->data(), escaped->size());
                continue;
            }

            if (byte <= 0x7Fu) {
                out.push_back(ch);
                continue;
            }

            auto code_point = parse_utf8_code_point(ch);
            if (!code_point.has_value()) {
                return std::unexpected(code_point.error());
            }

            json_append_utf8(out, *code_point);
        }

        return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
    }

    [[nodiscard]] auto parse_escape_sequence() -> result<std::u8string, parse_error_detail>
    {
        if (pos >= text.size()) {
            return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
        }

        const char8_t escaped = text[pos++];
        switch (escaped) {
        case u8'"':
            return std::u8string(1, u8'"');
        case u8'\\':
            return std::u8string(1, u8'\\');
        case u8'/':
            return std::u8string(1, u8'/');
        case u8'b':
            return std::u8string(1, u8'\b');
        case u8'f':
            return std::u8string(1, u8'\f');
        case u8'n':
            return std::u8string(1, u8'\n');
        case u8'r':
            return std::u8string(1, u8'\r');
        case u8't':
            return std::u8string(1, u8'\t');
        case u8'u':
            return parse_unicode_escape();
        default:
            return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
        }
    }

    [[nodiscard]] auto parse_unicode_escape() -> result<std::u8string, parse_error_detail>
    {
        auto first_unit = parse_hex16();
        if (!first_unit.has_value()) {
            return std::unexpected(first_unit.error());
        }

        char32_t code_point = *first_unit;

        if (code_point >= 0xD800u && code_point <= 0xDBFFu) {
            if (pos + 1 >= text.size() || text[pos] != u8'\\' || text[pos + 1] != u8'u') {
                return std::unexpected(make_parse_error(parse_error_reason::invalid_encoding, static_cast<std::size_t>(-1), error_t::encoding_error));
            }

            pos += 2;
            auto second_unit = parse_hex16();
            if (!second_unit.has_value()) {
                return std::unexpected(second_unit.error());
            }

            if (*second_unit < 0xDC00u || *second_unit > 0xDFFFu) {
                return std::unexpected(make_parse_error(parse_error_reason::invalid_encoding, static_cast<std::size_t>(-1), error_t::encoding_error));
            }

            const std::uint32_t high = static_cast<std::uint32_t>(code_point) - 0xD800u;
            const std::uint32_t low = static_cast<std::uint32_t>(*second_unit) - 0xDC00u;
            code_point = static_cast<char32_t>(0x10000u + ((high << 10) | low));
        } else if (code_point >= 0xDC00u && code_point <= 0xDFFFu) {
            return std::unexpected(make_parse_error(parse_error_reason::invalid_encoding, static_cast<std::size_t>(-1), error_t::encoding_error));
        }

        std::u8string out;
        json_append_utf8(out, code_point);
        return out;
    }

    [[nodiscard]] auto parse_hex16() -> result<char32_t, parse_error_detail>
    {
        if (pos + 4 > text.size()) {
            return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
        }

        std::uint32_t value = 0;
        for (int i = 0; i < 4; ++i) {
            const int hex = decode_hex_digit(text[pos++]);
            if (hex < 0) {
                return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
            }

            value = (value << 4) | static_cast<std::uint32_t>(hex);
        }

        return static_cast<char32_t>(value);
    }

    [[nodiscard]] auto parse_utf8_code_point(char8_t first_byte) -> result<char32_t, parse_error_detail>
    {
        const std::uint8_t b1 = static_cast<std::uint8_t>(first_byte);
        std::uint32_t packed = b1;
        std::size_t extra = 0;

        if (b1 >= 0xC2u && b1 <= 0xDFu) {
            extra = 1;
        } else if (b1 >= 0xE0u && b1 <= 0xEFu) {
            extra = 2;
        } else if (b1 >= 0xF0u && b1 <= 0xF4u) {
            extra = 3;
        } else {
            return std::unexpected(make_parse_error(parse_error_reason::invalid_encoding, static_cast<std::size_t>(-1), error_t::encoding_error));
        }

        if (pos + extra > text.size()) {
            return std::unexpected(make_parse_error(parse_error_reason::invalid_encoding, static_cast<std::size_t>(-1), error_t::encoding_error));
        }

        for (std::size_t i = 0; i < extra; ++i) {
            const std::uint8_t byte = static_cast<std::uint8_t>(text[pos++]);
            packed |= static_cast<std::uint32_t>(byte) << ((i + 1u) * 8u);
        }

        const char32_t code_point = xer::advanced::packed_utf8_to_utf32(packed);
        if (code_point == xer::advanced::detail::invalid_utf32) {
            return std::unexpected(make_parse_error(parse_error_reason::invalid_encoding, static_cast<std::size_t>(-1), error_t::encoding_error));
        }

        return code_point;
    }

    [[nodiscard]] auto parse_array() -> result<json_value, parse_error_detail>
    {
        ++pos;
        skip_whitespace();

        json_array array;
        if (pos < text.size() && text[pos] == u8']') {
            ++pos;
            return json_value(std::move(array));
        }

        while (true) {
            auto element = parse_value();
            if (!element.has_value()) {
                return element;
            }

            array.push_back(std::move(*element));
            skip_whitespace();

            if (pos >= text.size()) {
                return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
            }

            if (text[pos] == u8']') {
                ++pos;
                return json_value(std::move(array));
            }

            if (text[pos] != u8',') {
                return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
            }

            ++pos;
            skip_whitespace();
        }
    }

    [[nodiscard]] auto parse_object() -> result<json_value, parse_error_detail>
    {
        ++pos;
        skip_whitespace();

        json_object object;
        if (pos < text.size() && text[pos] == u8'}') {
            ++pos;
            return json_value(std::move(object));
        }

        while (true) {
            if (pos >= text.size() || text[pos] != u8'"') {
                return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
            }

            auto key = parse_string();
            if (!key.has_value()) {
                return key;
            }

            skip_whitespace();
            if (pos >= text.size() || text[pos] != u8':') {
                return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
            }

            ++pos;
            skip_whitespace();

            auto value = parse_value();
            if (!value.has_value()) {
                return value;
            }

            object.emplace_back(std::move(key->as_string()), std::move(*value));
            skip_whitespace();

            if (pos >= text.size()) {
                return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
            }

            if (text[pos] == u8'}') {
                ++pos;
                return json_value(std::move(object));
            }

            if (text[pos] != u8',') {
                return std::unexpected(make_parse_error(parse_error_reason::invalid_syntax));
            }

            ++pos;
            skip_whitespace();
        }
    }

    [[nodiscard]] auto match_literal(std::u8string_view literal) noexcept -> bool
    {
        if (text.substr(pos, literal.size()) != literal) {
            return false;
        }

        pos += literal.size();
        return true;
    }

    [[nodiscard]] static auto decode_hex_digit(char8_t ch) noexcept -> int
    {
        if (ch >= u8'0' && ch <= u8'9') {
            return static_cast<int>(ch - u8'0');
        }

        if (ch >= u8'a' && ch <= u8'f') {
            return 10 + static_cast<int>(ch - u8'a');
        }

        if (ch >= u8'A' && ch <= u8'F') {
            return 10 + static_cast<int>(ch - u8'A');
        }

        return -1;
    }
};

} // namespace xer::detail

namespace xer {

/**
 * @brief Decodes a JSON text into a JSON value.
 *
 * The accepted syntax is strict JSON without comments or trailing commas.
 * Numbers are stored as double.
 *
 * @param json_text UTF-8 JSON text.
 * @return Parsed JSON value on success.
 */
[[nodiscard]] inline auto json_decode(std::u8string_view json_text) -> result<json_value, parse_error_detail>
{
    detail::json_decoder decoder(json_text);
    return decoder.parse();
}

} // namespace xer

#endif /* XER_BITS_JSON_DECODE_H_INCLUDED_ */

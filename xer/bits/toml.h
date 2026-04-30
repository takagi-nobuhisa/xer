/**
 * @file xer/bits/toml.h
 * @brief Internal TOML subset decode and encode implementation.
 */

#pragma once

#ifndef XER_BITS_TOML_H_INCLUDED_
#define XER_BITS_TOML_H_INCLUDED_

#include <cerrno>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <expected>
#include <limits>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include <xer/bits/common.h>
#include <xer/error.h>

namespace xer {

struct toml_value;

using toml_array = std::vector<toml_value>;
using toml_table = std::vector<std::pair<std::u8string, toml_value>>;

/**
 * @brief Represents one TOML value in the initial XER TOML subset.
 *
 * The initial subset supports booleans, signed 64-bit integers, double-precision
 * floating-point numbers, UTF-8 strings, arrays, and tables.
 *
 * TOML date/time values, inline tables, dotted keys, quoted keys, and
 * array-of-tables are intentionally deferred.
 */
struct toml_value {
    using array_type = toml_array;
    using table_type = toml_table;
    using variant_type = std::variant<
        bool,
        std::int64_t,
        double,
        std::u8string,
        array_type,
        table_type>;

    variant_type value;

    toml_value() : value(table_type{}) {}
    toml_value(bool v) : value(v) {}
    toml_value(std::int64_t v) : value(v) {}
    toml_value(double v) : value(v) {}
    toml_value(std::u8string v) : value(std::move(v)) {}
    toml_value(std::u8string_view v) : value(std::u8string(v)) {}
    toml_value(const char8_t* v) : value(std::u8string(v)) {}
    toml_value(array_type v) : value(std::move(v)) {}
    toml_value(table_type v) : value(std::move(v)) {}

    [[nodiscard]] auto is_bool() const noexcept -> bool
    {
        return std::holds_alternative<bool>(value);
    }

    [[nodiscard]] auto is_integer() const noexcept -> bool
    {
        return std::holds_alternative<std::int64_t>(value);
    }

    [[nodiscard]] auto is_float() const noexcept -> bool
    {
        return std::holds_alternative<double>(value);
    }

    [[nodiscard]] auto is_string() const noexcept -> bool
    {
        return std::holds_alternative<std::u8string>(value);
    }

    [[nodiscard]] auto is_array() const noexcept -> bool
    {
        return std::holds_alternative<array_type>(value);
    }

    [[nodiscard]] auto is_table() const noexcept -> bool
    {
        return std::holds_alternative<table_type>(value);
    }

    [[nodiscard]] auto as_bool() noexcept -> bool*
    {
        return std::get_if<bool>(&value);
    }

    [[nodiscard]] auto as_bool() const noexcept -> const bool*
    {
        return std::get_if<bool>(&value);
    }

    [[nodiscard]] auto as_integer() noexcept -> std::int64_t*
    {
        return std::get_if<std::int64_t>(&value);
    }

    [[nodiscard]] auto as_integer() const noexcept -> const std::int64_t*
    {
        return std::get_if<std::int64_t>(&value);
    }

    [[nodiscard]] auto as_float() noexcept -> double*
    {
        return std::get_if<double>(&value);
    }

    [[nodiscard]] auto as_float() const noexcept -> const double*
    {
        return std::get_if<double>(&value);
    }

    [[nodiscard]] auto as_string() noexcept -> std::u8string*
    {
        return std::get_if<std::u8string>(&value);
    }

    [[nodiscard]] auto as_string() const noexcept -> const std::u8string*
    {
        return std::get_if<std::u8string>(&value);
    }

    [[nodiscard]] auto as_array() noexcept -> array_type*
    {
        return std::get_if<array_type>(&value);
    }

    [[nodiscard]] auto as_array() const noexcept -> const array_type*
    {
        return std::get_if<array_type>(&value);
    }

    [[nodiscard]] auto as_table() noexcept -> table_type*
    {
        return std::get_if<table_type>(&value);
    }

    [[nodiscard]] auto as_table() const noexcept -> const table_type*
    {
        return std::get_if<table_type>(&value);
    }
};

} // namespace xer

namespace xer::detail {

[[nodiscard]] constexpr auto toml_is_ascii_space(char8_t ch) noexcept -> bool
{
    return ch == u8' ' || ch == u8'\t' || ch == u8'\n' ||
           ch == u8'\r' || ch == u8'\f' || ch == u8'\v';
}

[[nodiscard]] constexpr auto toml_is_bare_key_char(char8_t ch) noexcept -> bool
{
    return (ch >= u8'a' && ch <= u8'z') ||
           (ch >= u8'A' && ch <= u8'Z') ||
           (ch >= u8'0' && ch <= u8'9') || ch == u8'_' || ch == u8'-';
}

[[nodiscard]] constexpr auto toml_trim_ascii_space(
    std::u8string_view value) noexcept -> std::u8string_view
{
    std::size_t first = 0;
    while (first < value.size() && toml_is_ascii_space(value[first])) {
        ++first;
    }

    std::size_t last = value.size();
    while (last > first && toml_is_ascii_space(value[last - 1])) {
        --last;
    }

    return value.substr(first, last - first);
}

[[nodiscard]] inline auto toml_is_valid_utf8(std::u8string_view value) noexcept
    -> bool
{
    std::size_t i = 0;

    while (i < value.size()) {
        const auto c0 = static_cast<unsigned char>(value[i]);

        if (c0 <= 0x7f) {
            ++i;
            continue;
        }

        if (c0 >= 0xc2 && c0 <= 0xdf) {
            if (i + 1 >= value.size()) {
                return false;
            }

            const auto c1 = static_cast<unsigned char>(value[i + 1]);
            if ((c1 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 2;
            continue;
        }

        if (c0 == 0xe0) {
            if (i + 2 >= value.size()) {
                return false;
            }

            const auto c1 = static_cast<unsigned char>(value[i + 1]);
            const auto c2 = static_cast<unsigned char>(value[i + 2]);
            if (c1 < 0xa0 || c1 > 0xbf || (c2 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 3;
            continue;
        }

        if (c0 >= 0xe1 && c0 <= 0xec) {
            if (i + 2 >= value.size()) {
                return false;
            }

            const auto c1 = static_cast<unsigned char>(value[i + 1]);
            const auto c2 = static_cast<unsigned char>(value[i + 2]);
            if ((c1 & 0xc0u) != 0x80u || (c2 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 3;
            continue;
        }

        if (c0 == 0xed) {
            if (i + 2 >= value.size()) {
                return false;
            }

            const auto c1 = static_cast<unsigned char>(value[i + 1]);
            const auto c2 = static_cast<unsigned char>(value[i + 2]);
            if (c1 < 0x80 || c1 > 0x9f || (c2 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 3;
            continue;
        }

        if (c0 >= 0xee && c0 <= 0xef) {
            if (i + 2 >= value.size()) {
                return false;
            }

            const auto c1 = static_cast<unsigned char>(value[i + 1]);
            const auto c2 = static_cast<unsigned char>(value[i + 2]);
            if ((c1 & 0xc0u) != 0x80u || (c2 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 3;
            continue;
        }

        if (c0 == 0xf0) {
            if (i + 3 >= value.size()) {
                return false;
            }

            const auto c1 = static_cast<unsigned char>(value[i + 1]);
            const auto c2 = static_cast<unsigned char>(value[i + 2]);
            const auto c3 = static_cast<unsigned char>(value[i + 3]);
            if (c1 < 0x90 || c1 > 0xbf || (c2 & 0xc0u) != 0x80u ||
                (c3 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 4;
            continue;
        }

        if (c0 >= 0xf1 && c0 <= 0xf3) {
            if (i + 3 >= value.size()) {
                return false;
            }

            const auto c1 = static_cast<unsigned char>(value[i + 1]);
            const auto c2 = static_cast<unsigned char>(value[i + 2]);
            const auto c3 = static_cast<unsigned char>(value[i + 3]);
            if ((c1 & 0xc0u) != 0x80u || (c2 & 0xc0u) != 0x80u ||
                (c3 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 4;
            continue;
        }

        if (c0 == 0xf4) {
            if (i + 3 >= value.size()) {
                return false;
            }

            const auto c1 = static_cast<unsigned char>(value[i + 1]);
            const auto c2 = static_cast<unsigned char>(value[i + 2]);
            const auto c3 = static_cast<unsigned char>(value[i + 3]);
            if (c1 < 0x80 || c1 > 0x8f || (c2 & 0xc0u) != 0x80u ||
                (c3 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 4;
            continue;
        }

        return false;
    }

    return true;
}

[[nodiscard]] inline auto toml_is_bare_key(std::u8string_view value) noexcept
    -> bool
{
    if (value.empty()) {
        return false;
    }

    for (const char8_t ch : value) {
        if (!toml_is_bare_key_char(ch)) {
            return false;
        }
    }

    return true;
}

[[nodiscard]] inline auto toml_starts_with(
    std::u8string_view value,
    std::u8string_view prefix) noexcept -> bool
{
    return value.size() >= prefix.size() &&
           value.substr(0, prefix.size()) == prefix;
}

[[nodiscard]] inline auto toml_find_unquoted(
    std::u8string_view value,
    char8_t target) noexcept -> std::size_t
{
    enum class state {
        ordinary,
        basic_string,
        literal_string,
        multiline_basic_string,
        multiline_literal_string,
    } current = state::ordinary;

    bool escaped = false;

    for (std::size_t i = 0; i < value.size(); ++i) {
        const char8_t ch = value[i];

        switch (current) {
        case state::ordinary:
            if (toml_starts_with(value.substr(i), u8"\"\"\"")) {
                current = state::multiline_basic_string;
                i += 2;
                continue;
            }

            if (toml_starts_with(value.substr(i), u8"'''")) {
                current = state::multiline_literal_string;
                i += 2;
                continue;
            }

            if (ch == u8'"') {
                current = state::basic_string;
                escaped = false;
                continue;
            }

            if (ch == u8'\'') {
                current = state::literal_string;
                continue;
            }

            if (ch == target) {
                return i;
            }

            break;

        case state::basic_string:
            if (escaped) {
                escaped = false;
                continue;
            }

            if (ch == u8'\\') {
                escaped = true;
                continue;
            }

            if (ch == u8'"') {
                current = state::ordinary;
            }

            break;

        case state::literal_string:
            if (ch == u8'\'') {
                current = state::ordinary;
            }

            break;

        case state::multiline_basic_string:
            if (escaped) {
                escaped = false;
                continue;
            }

            if (ch == u8'\\') {
                escaped = true;
                continue;
            }

            if (toml_starts_with(value.substr(i), u8"\"\"\"")) {
                current = state::ordinary;
                i += 2;
            }

            break;

        case state::multiline_literal_string:
            if (toml_starts_with(value.substr(i), u8"'''")) {
                current = state::ordinary;
                i += 2;
            }

            break;
        }
    }

    return std::u8string_view::npos;
}

[[nodiscard]] inline auto toml_line_has_unclosed_multiline_string(
    std::u8string_view value) noexcept -> bool
{
    enum class state {
        ordinary,
        basic_string,
        literal_string,
        multiline_basic_string,
        multiline_literal_string,
    } current = state::ordinary;

    bool escaped = false;

    for (std::size_t i = 0; i < value.size(); ++i) {
        const char8_t ch = value[i];

        switch (current) {
        case state::ordinary:
            if (ch == u8'#') {
                return false;
            }

            if (toml_starts_with(value.substr(i), u8"\"\"\"")) {
                current = state::multiline_basic_string;
                i += 2;
                continue;
            }

            if (toml_starts_with(value.substr(i), u8"'''")) {
                current = state::multiline_literal_string;
                i += 2;
                continue;
            }

            if (ch == u8'"') {
                current = state::basic_string;
                escaped = false;
                continue;
            }

            if (ch == u8'\'') {
                current = state::literal_string;
                continue;
            }

            break;

        case state::basic_string:
            if (escaped) {
                escaped = false;
                continue;
            }

            if (ch == u8'\\') {
                escaped = true;
                continue;
            }

            if (ch == u8'"') {
                current = state::ordinary;
            }

            break;

        case state::literal_string:
            if (ch == u8'\'') {
                current = state::ordinary;
            }

            break;

        case state::multiline_basic_string:
            if (escaped) {
                escaped = false;
                continue;
            }

            if (ch == u8'\\') {
                escaped = true;
                continue;
            }

            if (toml_starts_with(value.substr(i), u8"\"\"\"")) {
                current = state::ordinary;
                i += 2;
            }

            break;

        case state::multiline_literal_string:
            if (toml_starts_with(value.substr(i), u8"'''")) {
                current = state::ordinary;
                i += 2;
            }

            break;
        }
    }

    return current == state::multiline_basic_string ||
           current == state::multiline_literal_string;
}

[[nodiscard]] inline auto toml_strip_comment(
    std::u8string_view line) noexcept -> std::u8string_view
{
    const std::size_t pos = toml_find_unquoted(line, u8'#');
    if (pos == std::u8string_view::npos) {
        return line;
    }

    return line.substr(0, pos);
}

[[nodiscard]] inline auto toml_find_top_level_comma(
    std::u8string_view value,
    std::size_t start) noexcept -> std::size_t
{
    enum class state {
        ordinary,
        basic_string,
        literal_string,
        multiline_basic_string,
        multiline_literal_string,
    } current = state::ordinary;

    bool escaped = false;
    int array_depth = 0;

    for (std::size_t i = start; i < value.size(); ++i) {
        const char8_t ch = value[i];

        switch (current) {
        case state::ordinary:
            if (toml_starts_with(value.substr(i), u8"\"\"\"")) {
                current = state::multiline_basic_string;
                i += 2;
                continue;
            }

            if (toml_starts_with(value.substr(i), u8"'''")) {
                current = state::multiline_literal_string;
                i += 2;
                continue;
            }

            if (ch == u8'"') {
                current = state::basic_string;
                escaped = false;
                continue;
            }

            if (ch == u8'\'') {
                current = state::literal_string;
                continue;
            }

            if (ch == u8'[') {
                ++array_depth;
                continue;
            }

            if (ch == u8']') {
                --array_depth;
                continue;
            }

            if (ch == u8',' && array_depth == 0) {
                return i;
            }

            break;

        case state::basic_string:
            if (escaped) {
                escaped = false;
                continue;
            }

            if (ch == u8'\\') {
                escaped = true;
                continue;
            }

            if (ch == u8'"') {
                current = state::ordinary;
            }

            break;

        case state::literal_string:
            if (ch == u8'\'') {
                current = state::ordinary;
            }

            break;

        case state::multiline_basic_string:
            if (escaped) {
                escaped = false;
                continue;
            }

            if (ch == u8'\\') {
                escaped = true;
                continue;
            }

            if (toml_starts_with(value.substr(i), u8"\"\"\"")) {
                current = state::ordinary;
                i += 2;
            }

            break;

        case state::multiline_literal_string:
            if (toml_starts_with(value.substr(i), u8"'''")) {
                current = state::ordinary;
                i += 2;
            }

            break;
        }
    }

    return std::u8string_view::npos;
}

[[nodiscard]] constexpr auto toml_hex_digit_value(char8_t ch) noexcept -> int
{
    if (ch >= u8'0' && ch <= u8'9') {
        return static_cast<int>(ch - u8'0');
    }

    if (ch >= u8'a' && ch <= u8'f') {
        return static_cast<int>(ch - u8'a') + 10;
    }

    if (ch >= u8'A' && ch <= u8'F') {
        return static_cast<int>(ch - u8'A') + 10;
    }

    return -1;
}

inline auto toml_append_utf8_code_point(
    std::u8string& out,
    char32_t code_point) -> result<void>
{
    if (code_point > U'\U0010FFFF' ||
        (code_point >= static_cast<char32_t>(0xD800) && code_point <= static_cast<char32_t>(0xDFFF))) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (code_point <= U'\u007F') {
        out.push_back(static_cast<char8_t>(code_point));
        return {};
    }

    if (code_point <= U'\u07FF') {
        out.push_back(static_cast<char8_t>(0xC0u | (code_point >> 6)));
        out.push_back(static_cast<char8_t>(0x80u | (code_point & 0x3Fu)));
        return {};
    }

    if (code_point <= U'\uFFFF') {
        out.push_back(static_cast<char8_t>(0xE0u | (code_point >> 12)));
        out.push_back(static_cast<char8_t>(0x80u | ((code_point >> 6) & 0x3Fu)));
        out.push_back(static_cast<char8_t>(0x80u | (code_point & 0x3Fu)));
        return {};
    }

    out.push_back(static_cast<char8_t>(0xF0u | (code_point >> 18)));
    out.push_back(static_cast<char8_t>(0x80u | ((code_point >> 12) & 0x3Fu)));
    out.push_back(static_cast<char8_t>(0x80u | ((code_point >> 6) & 0x3Fu)));
    out.push_back(static_cast<char8_t>(0x80u | (code_point & 0x3Fu)));
    return {};
}

[[nodiscard]] inline auto toml_parse_unicode_escape(
    std::u8string_view value,
    std::size_t& index,
    std::size_t digits) -> result<char32_t>
{
    if (index + digits > value.size()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    char32_t code_point = 0;
    for (std::size_t i = 0; i < digits; ++i) {
        const int digit = toml_hex_digit_value(value[index + i]);
        if (digit < 0) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        code_point = static_cast<char32_t>((code_point << 4) |
                                           static_cast<char32_t>(digit));
    }

    index += digits;
    return code_point;
}

[[nodiscard]] inline auto toml_parse_basic_string_body(
    std::u8string_view body,
    bool multiline) -> result<std::u8string>
{
    std::u8string out;

    for (std::size_t i = 0; i < body.size();) {
        const char8_t ch = body[i++];

        if (ch != u8'\\') {
            if (!multiline && (ch == u8'"' || ch == u8'\n' || ch == u8'\r')) {
                return std::unexpected(make_error(error_t::invalid_argument));
            }

            out.push_back(ch);
            continue;
        }

        if (i >= body.size()) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        const char8_t escaped = body[i++];
        switch (escaped) {
        case u8'"':
            out.push_back(u8'"');
            break;
        case u8'\\':
            out.push_back(u8'\\');
            break;
        case u8'b':
            out.push_back(u8'\b');
            break;
        case u8't':
            out.push_back(u8'\t');
            break;
        case u8'n':
            out.push_back(u8'\n');
            break;
        case u8'f':
            out.push_back(u8'\f');
            break;
        case u8'r':
            out.push_back(u8'\r');
            break;
        case u8'u': {
            auto code_point = toml_parse_unicode_escape(body, i, 4);
            if (!code_point.has_value()) {
                return std::unexpected(code_point.error());
            }

            auto appended = toml_append_utf8_code_point(out, *code_point);
            if (!appended.has_value()) {
                return std::unexpected(appended.error());
            }
            break;
        }
        case u8'U': {
            auto code_point = toml_parse_unicode_escape(body, i, 8);
            if (!code_point.has_value()) {
                return std::unexpected(code_point.error());
            }

            auto appended = toml_append_utf8_code_point(out, *code_point);
            if (!appended.has_value()) {
                return std::unexpected(appended.error());
            }
            break;
        }
        case u8'\n':
            if (!multiline) {
                return std::unexpected(make_error(error_t::invalid_argument));
            }

            while (i < body.size() && toml_is_ascii_space(body[i])) {
                ++i;
            }
            break;
        default:
            return std::unexpected(make_error(error_t::invalid_argument));
        }
    }

    return out;
}

[[nodiscard]] inline auto toml_parse_basic_string(std::u8string_view value)
    -> result<std::u8string>
{
    if (value.size() < 2 || value.front() != u8'"' || value.back() != u8'"') {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const std::u8string_view body = value.substr(1, value.size() - 2);
    return toml_parse_basic_string_body(body, false);
}

[[nodiscard]] inline auto toml_parse_multiline_basic_string(
    std::u8string_view value) -> result<std::u8string>
{
    if (value.size() < 6 || !toml_starts_with(value, u8"\"\"\"") ||
        !toml_starts_with(value.substr(value.size() - 3), u8"\"\"\"")) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::u8string_view body = value.substr(3, value.size() - 6);
    if (!body.empty() && body.front() == u8'\n') {
        body.remove_prefix(1);
    }

    if (body.find(u8"\"\"\"") != std::u8string_view::npos) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return toml_parse_basic_string_body(body, true);
}

[[nodiscard]] inline auto toml_parse_literal_string(std::u8string_view value)
    -> result<std::u8string>
{
    if (value.size() < 2 || value.front() != u8'\'' || value.back() != u8'\'') {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const std::u8string_view body = value.substr(1, value.size() - 2);
    if (body.find(u8'\'') != std::u8string_view::npos ||
        body.find(u8'\n') != std::u8string_view::npos ||
        body.find(u8'\r') != std::u8string_view::npos) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return std::u8string(body);
}

[[nodiscard]] inline auto toml_parse_multiline_literal_string(
    std::u8string_view value) -> result<std::u8string>
{
    if (value.size() < 6 || !toml_starts_with(value, u8"'''") ||
        !toml_starts_with(value.substr(value.size() - 3), u8"'''")) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::u8string_view body = value.substr(3, value.size() - 6);
    if (!body.empty() && body.front() == u8'\n') {
        body.remove_prefix(1);
    }

    if (body.find(u8"'''") != std::u8string_view::npos) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return std::u8string(body);
}

[[nodiscard]] inline auto toml_parse_string(std::u8string_view value)
    -> result<std::u8string>
{
    if (toml_starts_with(value, u8"\"\"\"")) {
        return toml_parse_multiline_basic_string(value);
    }

    if (toml_starts_with(value, u8"'''")) {
        return toml_parse_multiline_literal_string(value);
    }

    if (!value.empty() && value.front() == u8'\'') {
        return toml_parse_literal_string(value);
    }

    return toml_parse_basic_string(value);
}


[[nodiscard]] inline auto toml_find_key(
    toml_table& table,
    std::u8string_view key) noexcept -> toml_value*;

[[nodiscard]] inline auto toml_find_key(
    const toml_table& table,
    std::u8string_view key) noexcept -> const toml_value*;

using toml_key_path = std::vector<std::u8string>;

[[nodiscard]] inline auto toml_parse_key_segment(std::u8string_view value)
    -> result<std::u8string>
{
    value = toml_trim_ascii_space(value);

    if (value.empty()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (toml_is_bare_key(value)) {
        return std::u8string(value);
    }

    if (toml_starts_with(value, u8"\"\"\"") ||
        toml_starts_with(value, u8"'''")) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (value.front() == u8'"' || value.front() == u8'\'') {
        auto segment = toml_parse_string(value);
        if (!segment.has_value()) {
            return std::unexpected(segment.error());
        }

        return *segment;
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

[[nodiscard]] inline auto toml_parse_key_path(std::u8string_view value)
    -> result<toml_key_path>
{
    value = toml_trim_ascii_space(value);

    toml_key_path path;
    std::size_t start = 0;

    while (start <= value.size()) {
        const std::size_t dot = toml_find_unquoted(value.substr(start), u8'.');
        const std::size_t end = dot == std::u8string_view::npos
            ? value.size()
            : start + dot;

        auto segment = toml_parse_key_segment(value.substr(start, end - start));
        if (!segment.has_value()) {
            return std::unexpected(segment.error());
        }

        path.push_back(std::move(*segment));

        if (dot == std::u8string_view::npos) {
            break;
        }

        start = end + 1;
        if (start > value.size()) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }
    }

    if (path.empty()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return path;
}

[[nodiscard]] inline auto toml_key_path_equal(
    const toml_key_path& lhs,
    const toml_key_path& rhs) noexcept -> bool
{
    if (lhs.size() != rhs.size()) {
        return false;
    }

    for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }

    return true;
}

[[nodiscard]] inline auto toml_get_or_create_table(
    toml_table& base,
    const toml_key_path& path,
    std::size_t count) -> result<toml_table*>
{
    toml_table* table = &base;

    for (std::size_t i = 0; i < count; ++i) {
        toml_value* existing = toml_find_key(*table, path[i]);
        if (existing == nullptr) {
            table->push_back({path[i], toml_value(toml_table{})});
            existing = &table->back().second;
        }

        table = existing->as_table();
        if (table == nullptr) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }
    }

    return table;
}

[[nodiscard]] inline auto toml_parse_value(std::u8string_view value)
    -> result<toml_value>;

[[nodiscard]] inline auto toml_parse_array(std::u8string_view value)
    -> result<toml_value>
{
    if (value.size() < 2 || value.front() != u8'[' || value.back() != u8']') {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    toml_array array;
    std::u8string_view body = value.substr(1, value.size() - 2);
    body = toml_trim_ascii_space(body);

    if (body.empty()) {
        return toml_value(std::move(array));
    }

    std::size_t start = 0;
    while (start <= body.size()) {
        const std::size_t comma = toml_find_top_level_comma(body, start);
        const std::size_t end =
            comma == std::u8string_view::npos ? body.size() : comma;
        const std::u8string_view token =
            toml_trim_ascii_space(body.substr(start, end - start));

        if (token.empty()) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        auto element = toml_parse_value(token);
        if (!element.has_value()) {
            return std::unexpected(element.error());
        }

        array.push_back(std::move(*element));

        if (comma == std::u8string_view::npos) {
            break;
        }

        start = comma + 1;
    }

    return toml_value(std::move(array));
}

[[nodiscard]] constexpr auto toml_is_decimal_digit(char8_t ch) noexcept -> bool
{
    return ch >= u8'0' && ch <= u8'9';
}

[[nodiscard]] constexpr auto toml_is_hex_digit(char8_t ch) noexcept -> bool
{
    return (ch >= u8'0' && ch <= u8'9') ||
           (ch >= u8'a' && ch <= u8'f') ||
           (ch >= u8'A' && ch <= u8'F');
}

[[nodiscard]] constexpr auto toml_is_octal_digit(char8_t ch) noexcept -> bool
{
    return ch >= u8'0' && ch <= u8'7';
}

[[nodiscard]] constexpr auto toml_is_binary_digit(char8_t ch) noexcept -> bool
{
    return ch == u8'0' || ch == u8'1';
}

template <class Pred>
[[nodiscard]] inline auto toml_digits_with_separators_are_valid(
    std::u8string_view value,
    Pred is_digit) noexcept -> bool
{
    if (value.empty()) {
        return false;
    }

    bool previous_was_digit = false;

    for (std::size_t i = 0; i < value.size(); ++i) {
        const char8_t ch = value[i];

        if (is_digit(ch)) {
            previous_was_digit = true;
            continue;
        }

        if (ch == u8'_') {
            if (!previous_was_digit || i + 1 >= value.size() ||
                !is_digit(value[i + 1])) {
                return false;
            }

            previous_was_digit = false;
            continue;
        }

        return false;
    }

    return previous_was_digit;
}

template <class Pred>
[[nodiscard]] inline auto toml_remove_digit_separators(
    std::u8string_view value,
    Pred is_digit) -> result<std::u8string>
{
    if (!toml_digits_with_separators_are_valid(value, is_digit)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::u8string out;
    out.reserve(value.size());

    for (const char8_t ch : value) {
        if (ch != u8'_') {
            out.push_back(ch);
        }
    }

    return out;
}

[[nodiscard]] constexpr auto toml_digit_value(char8_t ch) noexcept -> int
{
    if (ch >= u8'0' && ch <= u8'9') {
        return static_cast<int>(ch - u8'0');
    }

    if (ch >= u8'a' && ch <= u8'f') {
        return static_cast<int>(ch - u8'a') + 10;
    }

    if (ch >= u8'A' && ch <= u8'F') {
        return static_cast<int>(ch - u8'A') + 10;
    }

    return -1;
}

[[nodiscard]] inline auto toml_parse_integer_digits(
    std::u8string_view digits,
    int base,
    bool negative) -> result<toml_value>
{
    if (digits.empty() || base < 2 || base > 16) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    constexpr auto max_value =
        static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());
    constexpr auto min_magnitude = max_value + UINT64_C(1);
    const std::uint64_t limit = negative ? min_magnitude : max_value;

    std::uint64_t magnitude = 0;

    for (const char8_t ch : digits) {
        const int digit = toml_digit_value(ch);
        if (digit < 0 || digit >= base) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        const auto value = static_cast<std::uint64_t>(digit);
        if (magnitude > (limit - value) / static_cast<std::uint64_t>(base)) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        magnitude = magnitude * static_cast<std::uint64_t>(base) + value;
    }

    if (negative) {
        if (magnitude == min_magnitude) {
            return toml_value(std::numeric_limits<std::int64_t>::min());
        }

        return toml_value(-static_cast<std::int64_t>(magnitude));
    }

    return toml_value(static_cast<std::int64_t>(magnitude));
}
[[nodiscard]] inline auto toml_parse_integer(std::u8string_view value)
    -> result<toml_value>
{
    bool negative = false;

    if (!value.empty() && (value.front() == u8'+' || value.front() == u8'-')) {
        negative = value.front() == u8'-';
        value.remove_prefix(1);
    }

    if (value.size() >= 2 && value[0] == u8'0') {
        if (value[1] == u8'x' || value[1] == u8'X') {
            auto digits =
                toml_remove_digit_separators(value.substr(2), toml_is_hex_digit);
            if (!digits.has_value()) {
                return std::unexpected(digits.error());
            }

            return toml_parse_integer_digits(*digits, 16, negative);
        }

        if (value[1] == u8'o' || value[1] == u8'O') {
            auto digits =
                toml_remove_digit_separators(value.substr(2), toml_is_octal_digit);
            if (!digits.has_value()) {
                return std::unexpected(digits.error());
            }

            return toml_parse_integer_digits(*digits, 8, negative);
        }

        if (value[1] == u8'b' || value[1] == u8'B') {
            auto digits =
                toml_remove_digit_separators(value.substr(2), toml_is_binary_digit);
            if (!digits.has_value()) {
                return std::unexpected(digits.error());
            }

            return toml_parse_integer_digits(*digits, 2, negative);
        }
    }

    auto digits = toml_remove_digit_separators(value, toml_is_decimal_digit);
    if (!digits.has_value()) {
        return std::unexpected(digits.error());
    }

    return toml_parse_integer_digits(*digits, 10, negative);
}

[[nodiscard]] inline auto toml_float_syntax_is_valid(
    std::u8string_view value) noexcept -> bool
{
    if (value.empty()) {
        return false;
    }

    if (value.front() == u8'+' || value.front() == u8'-') {
        value.remove_prefix(1);
    }

    const std::size_t exponent_pos = value.find_first_of(u8"eE");
    const std::u8string_view significand =
        exponent_pos == std::u8string_view::npos ? value
                                                 : value.substr(0, exponent_pos);

    if (significand.empty()) {
        return false;
    }

    const std::size_t dot_pos = significand.find(u8'.');
    if (dot_pos == std::u8string_view::npos &&
        exponent_pos == std::u8string_view::npos) {
        return false;
    }

    if (dot_pos != std::u8string_view::npos &&
        significand.find(u8'.', dot_pos + 1) != std::u8string_view::npos) {
        return false;
    }

    if (dot_pos == std::u8string_view::npos) {
        if (!toml_digits_with_separators_are_valid(
                significand, toml_is_decimal_digit)) {
            return false;
        }
    } else {
        const std::u8string_view whole = significand.substr(0, dot_pos);
        const std::u8string_view fraction = significand.substr(dot_pos + 1);

        if (!toml_digits_with_separators_are_valid(
                whole, toml_is_decimal_digit) ||
            !toml_digits_with_separators_are_valid(
                fraction, toml_is_decimal_digit)) {
            return false;
        }
    }

    if (exponent_pos == std::u8string_view::npos) {
        return true;
    }

    std::u8string_view exponent = value.substr(exponent_pos + 1);
    if (!exponent.empty() &&
        (exponent.front() == u8'+' || exponent.front() == u8'-')) {
        exponent.remove_prefix(1);
    }

    return toml_digits_with_separators_are_valid(
        exponent, toml_is_decimal_digit);
}

[[nodiscard]] inline auto toml_parse_float(std::u8string_view value)
    -> result<toml_value>
{
    if (!toml_float_syntax_is_valid(value)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::string narrow;
    narrow.reserve(value.size());

    for (const char8_t ch : value) {
        if (static_cast<unsigned char>(ch) > 0x7f) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        if (ch != u8'_') {
            narrow.push_back(static_cast<char>(ch));
        }
    }

    char* end = nullptr;
    errno = 0;
    const double out = std::strtod(narrow.c_str(), &end);

    if (errno == ERANGE || end == nullptr || *end != '\0' ||
        !std::isfinite(out)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return toml_value(out);
}

[[nodiscard]] inline auto toml_parse_special_float(std::u8string_view value)
    -> result<toml_value>
{
    bool negative = false;

    if (!value.empty() && (value.front() == u8'+' || value.front() == u8'-')) {
        negative = value.front() == u8'-';
        value.remove_prefix(1);
    }

    if (value == u8"inf") {
        double out = std::numeric_limits<double>::infinity();
        if (negative) {
            out = -out;
        }

        return toml_value(out);
    }

    if (value == u8"nan") {
        double out = std::numeric_limits<double>::quiet_NaN();
        if (negative) {
            out = -out;
        }

        return toml_value(out);
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

[[nodiscard]] inline auto toml_is_special_float(std::u8string_view value) noexcept
    -> bool
{
    if (!value.empty() && (value.front() == u8'+' || value.front() == u8'-')) {
        value.remove_prefix(1);
    }

    return value == u8"inf" || value == u8"nan";
}

[[nodiscard]] inline auto toml_has_integer_base_prefix(
    std::u8string_view value) noexcept -> bool
{
    if (!value.empty() && (value.front() == u8'+' || value.front() == u8'-')) {
        value.remove_prefix(1);
    }

    return value.size() >= 2 && value[0] == u8'0' &&
           (value[1] == u8'x' || value[1] == u8'X' ||
            value[1] == u8'o' || value[1] == u8'O' ||
            value[1] == u8'b' || value[1] == u8'B');
}

[[nodiscard]] inline auto toml_parse_value(std::u8string_view value)
    -> result<toml_value>
{
    value = toml_trim_ascii_space(value);

    if (value.empty()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (value.front() == u8'"' || value.front() == u8'\'') {
        auto string = toml_parse_string(value);
        if (!string.has_value()) {
            return std::unexpected(string.error());
        }

        return toml_value(std::move(*string));
    }

    if (value.front() == u8'[') {
        return toml_parse_array(value);
    }

    if (value == u8"true") {
        return toml_value(true);
    }

    if (value == u8"false") {
        return toml_value(false);
    }

    if (toml_is_special_float(value)) {
        return toml_parse_special_float(value);
    }

    if (toml_has_integer_base_prefix(value)) {
        return toml_parse_integer(value);
    }

    if (value.find(u8'.') != std::u8string_view::npos ||
        value.find(u8'e') != std::u8string_view::npos ||
        value.find(u8'E') != std::u8string_view::npos) {
        return toml_parse_float(value);
    }

    return toml_parse_integer(value);
}

[[nodiscard]] inline auto toml_find_key(
    const toml_table& table,
    std::u8string_view key) noexcept -> const toml_value*
{
    for (const auto& entry : table) {
        if (entry.first == key) {
            return &entry.second;
        }
    }

    return nullptr;
}

[[nodiscard]] inline auto toml_find_key(
    toml_table& table,
    std::u8string_view key) noexcept -> toml_value*
{
    for (auto& entry : table) {
        if (entry.first == key) {
            return &entry.second;
        }
    }

    return nullptr;
}


class toml_decoder {
public:
    explicit toml_decoder(std::u8string_view text_) noexcept
        : text(text_), pos(0)
    {
    }

    [[nodiscard]] auto parse() -> result<toml_value>
    {
        if (!toml_is_valid_utf8(text)) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        toml_value root(toml_table{});
        current_table = root.as_table();

        while (pos < text.size()) {
            std::u8string raw_line(read_line());
            while (toml_line_has_unclosed_multiline_string(raw_line)) {
                if (pos >= text.size()) {
                    return std::unexpected(make_error(error_t::invalid_argument));
                }

                raw_line.push_back(u8'\n');
                raw_line.append(read_line());
            }

            auto parsed = parse_line(root, raw_line);
            if (!parsed.has_value()) {
                return std::unexpected(parsed.error());
            }
        }

        return root;
    }

private:
    std::u8string_view text;
    std::size_t pos;
    toml_table* current_table = nullptr;
    std::vector<toml_key_path> declared_tables;

    [[nodiscard]] auto read_line() noexcept -> std::u8string_view
    {
        const std::size_t begin = pos;

        while (pos < text.size() && text[pos] != u8'\n' &&
               text[pos] != u8'\r') {
            ++pos;
        }

        const std::size_t end = pos;

        if (pos < text.size()) {
            if (text[pos] == u8'\r' && (pos + 1) < text.size() &&
                text[pos + 1] == u8'\n') {
                pos += 2;
            } else {
                ++pos;
            }
        }

        return text.substr(begin, end - begin);
    }

    [[nodiscard]] auto table_was_declared(
        const toml_key_path& path) const noexcept -> bool
    {
        for (const auto& declared : declared_tables) {
            if (toml_key_path_equal(declared, path)) {
                return true;
            }
        }

        return false;
    }

    [[nodiscard]] auto parse_line(
        toml_value& root,
        std::u8string_view line) -> result<void>
    {
        line = toml_trim_ascii_space(toml_strip_comment(line));

        if (line.empty()) {
            return {};
        }

        if (line.front() == u8'[') {
            return parse_table(root, line);
        }

        return parse_entry(line);
    }

    [[nodiscard]] auto parse_table(
        toml_value& root,
        std::u8string_view line) -> result<void>
    {
        if (line.size() < 3 || line.back() != u8']' ||
            (line.size() >= 2 && line[1] == u8'[')) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        const std::u8string_view raw_name =
            toml_trim_ascii_space(line.substr(1, line.size() - 2));
        auto path = toml_parse_key_path(raw_name);
        if (!path.has_value()) {
            return std::unexpected(path.error());
        }

        if (table_was_declared(*path)) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        auto* root_table = root.as_table();
        if (root_table == nullptr) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        auto table = toml_get_or_create_table(*root_table, *path, path->size());
        if (!table.has_value()) {
            return std::unexpected(table.error());
        }

        declared_tables.push_back(std::move(*path));
        current_table = *table;
        return {};
    }

    [[nodiscard]] auto parse_entry(std::u8string_view line) -> result<void>
    {
        const std::size_t separator = toml_find_unquoted(line, u8'=');
        if (separator == std::u8string_view::npos) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        const std::u8string_view raw_key =
            toml_trim_ascii_space(line.substr(0, separator));
        const std::u8string_view raw_value =
            toml_trim_ascii_space(line.substr(separator + 1));

        if (current_table == nullptr) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        auto path = toml_parse_key_path(raw_key);
        if (!path.has_value()) {
            return std::unexpected(path.error());
        }

        auto* destination = current_table;
        if (path->size() > 1) {
            auto table = toml_get_or_create_table(
                *current_table,
                *path,
                path->size() - 1);
            if (!table.has_value()) {
                return std::unexpected(table.error());
            }

            destination = *table;
        }

        const auto& key = path->back();
        if (toml_find_key(*destination, key) != nullptr) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        auto value = toml_parse_value(raw_value);
        if (!value.has_value()) {
            return std::unexpected(value.error());
        }

        destination->push_back({key, std::move(*value)});
        return {};
    }
};

[[nodiscard]] inline auto toml_value_can_encode(const toml_value& value) -> bool;

[[nodiscard]] inline auto toml_key_can_encode(std::u8string_view key) -> bool
{
    return toml_is_valid_utf8(key) &&
           key.find(u8'\n') == std::u8string_view::npos &&
           key.find(u8'\r') == std::u8string_view::npos;
}

[[nodiscard]] inline auto toml_table_can_encode_as_section(
    const toml_table& table) -> bool
{
    for (const auto& entry : table) {
        if (!toml_key_can_encode(entry.first) ||
            !toml_value_can_encode(entry.second)) {
            return false;
        }
    }

    return true;
}

[[nodiscard]] inline auto toml_value_can_encode(const toml_value& value) -> bool
{
    if (value.is_table()) {
        return toml_table_can_encode_as_section(*value.as_table());
    }

    if (const auto* array = value.as_array()) {
        for (const auto& element : *array) {
            if (element.is_table() || !toml_value_can_encode(element)) {
                return false;
            }
        }
    }

    if (const auto* string = value.as_string()) {
        return toml_is_valid_utf8(*string);
    }

    if (const auto* number = value.as_float()) {
        return std::isfinite(*number) || std::isinf(*number) ||
               std::isnan(*number);
    }

    return true;
}

inline void toml_encode_string(std::u8string& out, std::u8string_view value)
{
    out.push_back(u8'"');

    for (const char8_t ch : value) {
        switch (ch) {
        case u8'"':
            out.append(u8"\\\"");
            break;
        case u8'\\':
            out.append(u8"\\\\");
            break;
        case u8'\b':
            out.append(u8"\\b");
            break;
        case u8'\t':
            out.append(u8"\\t");
            break;
        case u8'\n':
            out.append(u8"\\n");
            break;
        case u8'\f':
            out.append(u8"\\f");
            break;
        case u8'\r':
            out.append(u8"\\r");
            break;
        default:
            out.push_back(ch);
            break;
        }
    }

    out.push_back(u8'"');
}

[[nodiscard]] inline auto toml_encode_value(
    std::u8string& out,
    const toml_value& value) -> result<void>
{
    if (const auto* boolean = value.as_bool()) {
        out.append(*boolean ? u8"true" : u8"false");
        return {};
    }

    if (const auto* integer = value.as_integer()) {
        char buffer[64]{};
        const auto length = std::snprintf(
            buffer,
            sizeof(buffer),
            "%lld",
            static_cast<long long>(*integer));
        if (length < 0 ||
            static_cast<std::size_t>(length) >= sizeof(buffer)) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        out.append(
            reinterpret_cast<const char8_t*>(buffer),
            static_cast<std::size_t>(length));
        return {};
    }

    if (const auto* number = value.as_float()) {
        if (std::isnan(*number)) {
            out.append(u8"nan");
            return {};
        }

        if (std::isinf(*number)) {
            out.append(*number < 0.0 ? u8"-inf" : u8"inf");
            return {};
        }

        char buffer[128]{};
        const auto length = std::snprintf(
            buffer,
            sizeof(buffer),
            "%.17g",
            *number);
        if (length < 0 ||
            static_cast<std::size_t>(length) >= sizeof(buffer)) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        out.append(
            reinterpret_cast<const char8_t*>(buffer),
            static_cast<std::size_t>(length));
        return {};
    }

    if (const auto* string = value.as_string()) {
        if (!toml_is_valid_utf8(*string)) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        toml_encode_string(out, *string);
        return {};
    }

    if (const auto* array = value.as_array()) {
        out.push_back(u8'[');

        for (std::size_t i = 0; i < array->size(); ++i) {
            if (i != 0) {
                out.append(u8", ");
            }

            if ((*array)[i].is_table()) {
                return std::unexpected(make_error(error_t::invalid_argument));
            }

            auto encoded = toml_encode_value(out, (*array)[i]);
            if (!encoded.has_value()) {
                return encoded;
            }
        }

        out.push_back(u8']');
        return {};
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

inline void toml_encode_key_segment(std::u8string& out, std::u8string_view key)
{
    if (toml_is_bare_key(key)) {
        out.append(key);
        return;
    }

    toml_encode_string(out, key);
}

inline void toml_encode_key_path(
    std::u8string& out,
    const std::vector<std::u8string_view>& path)
{
    for (std::size_t i = 0; i < path.size(); ++i) {
        if (i != 0) {
            out.push_back(u8'.');
        }

        toml_encode_key_segment(out, path[i]);
    }
}

[[nodiscard]] inline auto toml_encode_entry(
    std::u8string& out,
    const std::pair<std::u8string, toml_value>& entry) -> result<void>
{
    if (!toml_key_can_encode(entry.first) || entry.second.is_table()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    toml_encode_key_segment(out, entry.first);
    out.append(u8" = ");

    auto encoded = toml_encode_value(out, entry.second);
    if (!encoded.has_value()) {
        return encoded;
    }

    out.push_back(u8'\n');
    return {};
}

[[nodiscard]] inline auto toml_encode_table_section(
    std::u8string& out,
    const toml_table& table,
    std::vector<std::u8string_view>& path) -> result<void>
{
    if (!out.empty()) {
        out.push_back(u8'\n');
    }

    out.push_back(u8'[');
    toml_encode_key_path(out, path);
    out.push_back(u8']');
    out.push_back(u8'\n');

    for (const auto& entry : table) {
        if (entry.second.is_table()) {
            continue;
        }

        auto encoded = toml_encode_entry(out, entry);
        if (!encoded.has_value()) {
            return std::unexpected(encoded.error());
        }
    }

    for (const auto& entry : table) {
        const auto* child = entry.second.as_table();
        if (child == nullptr) {
            continue;
        }

        if (!toml_key_can_encode(entry.first) ||
            !toml_table_can_encode_as_section(*child)) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        path.push_back(entry.first);
        auto encoded = toml_encode_table_section(out, *child, path);
        path.pop_back();

        if (!encoded.has_value()) {
            return std::unexpected(encoded.error());
        }
    }

    return {};
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Decodes TOML text into an ordered TOML representation.
 *
 * The initial implementation supports a practical subset: bare keys, ordinary
 * tables, basic strings, literal strings, multiline strings, signed integers,
 * finite and special floating-point numbers, numeric separators, booleans,
 * arrays, comments, and blank lines.
 *
 * Integers may use decimal, hexadecimal, octal, or binary notation. Dotted
 * keys, quoted keys, inline tables, array-of-tables, and date/time values are
 * intentionally deferred.
 *
 * @param text UTF-8 TOML text.
 * @return Parsed TOML value. On success, the returned value is a table.
 */
[[nodiscard]] inline auto toml_decode(std::u8string_view text) -> result<toml_value>
{
    detail::toml_decoder decoder(text);
    return decoder.parse();
}

/**
 * @brief Encodes a TOML representation into UTF-8 TOML text.
 *
 * The value to encode must be a top-level table. Nested tables, inline tables,
 * array-of-tables, and other deferred TOML features are not emitted by the
 * initial implementation.
 *
 * @param value TOML value to encode. It must hold a table.
 * @return UTF-8 TOML text on success.
 */
[[nodiscard]] inline auto toml_encode(const toml_value& value)
    -> result<std::u8string>
{
    const auto* root = value.as_table();
    if (root == nullptr) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (!detail::toml_table_can_encode_as_section(*root)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::u8string out;

    for (const auto& entry : *root) {
        if (entry.second.is_table()) {
            continue;
        }

        auto encoded = detail::toml_encode_entry(out, entry);
        if (!encoded.has_value()) {
            return std::unexpected(encoded.error());
        }
    }

    for (const auto& entry : *root) {
        const auto* section = entry.second.as_table();
        if (section == nullptr) {
            continue;
        }

        if (!detail::toml_key_can_encode(entry.first)) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        std::vector<std::u8string_view> path{entry.first};
        auto encoded = detail::toml_encode_table_section(out, *section, path);
        if (!encoded.has_value()) {
            return std::unexpected(encoded.error());
        }
    }

    return out;
}

} // namespace xer

#endif /* XER_BITS_TOML_H_INCLUDED_ */

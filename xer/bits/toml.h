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
 * TOML date/time values, inline tables, dotted keys, quoted keys, literal
 * strings, multiline strings, and array-of-tables are intentionally deferred.
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

[[nodiscard]] inline auto toml_find_unquoted(
    std::u8string_view value,
    char8_t target) noexcept -> std::size_t
{
    bool in_string = false;
    bool escaped = false;

    for (std::size_t i = 0; i < value.size(); ++i) {
        const char8_t ch = value[i];

        if (in_string) {
            if (escaped) {
                escaped = false;
                continue;
            }

            if (ch == u8'\\') {
                escaped = true;
                continue;
            }

            if (ch == u8'"') {
                in_string = false;
            }

            continue;
        }

        if (ch == u8'"') {
            in_string = true;
            continue;
        }

        if (ch == target) {
            return i;
        }
    }

    return std::u8string_view::npos;
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
    bool in_string = false;
    bool escaped = false;
    int array_depth = 0;

    for (std::size_t i = start; i < value.size(); ++i) {
        const char8_t ch = value[i];

        if (in_string) {
            if (escaped) {
                escaped = false;
                continue;
            }

            if (ch == u8'\\') {
                escaped = true;
                continue;
            }

            if (ch == u8'"') {
                in_string = false;
            }

            continue;
        }

        if (ch == u8'"') {
            in_string = true;
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
    }

    return std::u8string_view::npos;
}

[[nodiscard]] inline auto toml_parse_string(std::u8string_view value)
    -> result<std::u8string>
{
    if (value.size() < 2 || value.front() != u8'"' || value.back() != u8'"') {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::u8string out;

    for (std::size_t i = 1; i + 1 < value.size(); ++i) {
        const char8_t ch = value[i];

        if (ch != u8'\\') {
            if (ch == u8'"' || ch == u8'\n' || ch == u8'\r') {
                return std::unexpected(make_error(error_t::invalid_argument));
            }

            out.push_back(ch);
            continue;
        }

        if (i + 2 >= value.size()) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        const char8_t escaped = value[++i];
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
        default:
            return std::unexpected(make_error(error_t::invalid_argument));
        }
    }

    return out;
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

    if (value.front() == u8'"') {
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
            const std::u8string_view raw_line = read_line();
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
        if (line.size() < 3 || line.back() != u8']') {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        const std::u8string_view name =
            toml_trim_ascii_space(line.substr(1, line.size() - 2));
        if (!toml_is_bare_key(name)) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        auto* root_table = root.as_table();
        if (root_table == nullptr || toml_find_key(*root_table, name) != nullptr) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        root_table->push_back({std::u8string(name), toml_value(toml_table{})});
        current_table = root_table->back().second.as_table();
        return {};
    }

    [[nodiscard]] auto parse_entry(std::u8string_view line) -> result<void>
    {
        const std::size_t separator = toml_find_unquoted(line, u8'=');
        if (separator == std::u8string_view::npos) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        const std::u8string_view key =
            toml_trim_ascii_space(line.substr(0, separator));
        const std::u8string_view raw_value =
            toml_trim_ascii_space(line.substr(separator + 1));

        if (!toml_is_bare_key(key) || current_table == nullptr ||
            toml_find_key(*current_table, key) != nullptr) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        auto value = toml_parse_value(raw_value);
        if (!value.has_value()) {
            return std::unexpected(value.error());
        }

        current_table->push_back({std::u8string(key), std::move(*value)});
        return {};
    }
};

[[nodiscard]] inline auto toml_value_can_encode(const toml_value& value) -> bool;

[[nodiscard]] inline auto toml_table_can_encode_as_section(
    const toml_table& table) -> bool
{
    for (const auto& entry : table) {
        if (!toml_is_bare_key(entry.first) || entry.second.is_table() ||
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
        return std::isfinite(*number);
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
        if (!std::isfinite(*number)) {
            return std::unexpected(make_error(error_t::invalid_argument));
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

[[nodiscard]] inline auto toml_encode_entry(
    std::u8string& out,
    const std::pair<std::u8string, toml_value>& entry) -> result<void>
{
    if (!toml_is_bare_key(entry.first)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    out.append(entry.first);
    out.append(u8" = ");

    auto encoded = toml_encode_value(out, entry.second);
    if (!encoded.has_value()) {
        return encoded;
    }

    out.push_back(u8'\n');
    return {};
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Decodes TOML text into an ordered TOML representation.
 *
 * The initial implementation supports a practical subset: bare keys, ordinary
 * tables, basic double-quoted strings, signed integers, finite decimal
 * floating-point numbers, numeric separators, booleans, arrays, comments, and
 * blank lines.
 *
 * Integers may use decimal, hexadecimal, octal, or binary notation. Dotted
 * keys, quoted keys, inline tables, array-of-tables, date/time values, literal
 * strings, multiline strings, and special floating-point values are
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

    std::u8string out;

    for (const auto& entry : *root) {
        if (entry.second.is_table()) {
            continue;
        }

        if (!detail::toml_value_can_encode(entry.second)) {
            return std::unexpected(make_error(error_t::invalid_argument));
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

        if (!detail::toml_is_bare_key(entry.first) ||
            !detail::toml_table_can_encode_as_section(*section)) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        if (!out.empty()) {
            out.push_back(u8'\n');
        }

        out.push_back(u8'[');
        out.append(entry.first);
        out.push_back(u8']');
        out.push_back(u8'\n');

        for (const auto& section_entry : *section) {
            auto encoded = detail::toml_encode_entry(out, section_entry);
            if (!encoded.has_value()) {
                return std::unexpected(encoded.error());
            }
        }
    }

    return out;
}

} // namespace xer

#endif /* XER_BITS_TOML_H_INCLUDED_ */

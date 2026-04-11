/**
 * @file xer/bits/scanf_format.h
 * @brief Internal scanf-format parsing support.
 */

#pragma once

#ifndef XER_BITS_SCANF_FORMAT_H_INCLUDED_
#define XER_BITS_SCANF_FORMAT_H_INCLUDED_

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <xer/bits/common.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Specifies argument-selection mode for a scanf-style format.
 */
enum class scan_argument_mode_t {
    unresolved,
    sequential,
    positional
};

/**
 * @brief Specifies control-token kind.
 */
enum class scan_control_kind_t {
    at
};

/**
 * @brief Specifies length modifier for a scanf-style conversion.
 */
enum class scan_length_t {
    none,
    hh,
    h,
    l,
    ll,
    j,
    z,
    t,
    capital_l
};

/**
 * @brief Specifies conversion kind for a scanf-style conversion.
 */
enum class scan_conversion_t {
    percent,
    d,
    u,
    x,
    capital_x,
    o,
    f,
    capital_f,
    e,
    capital_e,
    g,
    capital_g,
    c,
    s,
    scanset
};

/**
 * @brief Represents a literal-text token.
 */
struct scan_literal_token {
    std::u8string text;
};

/**
 * @brief Represents a whitespace token.
 */
struct scan_whitespace_token {};

/**
 * @brief Represents a control token such as %@ or %n$@.
 */
struct scan_control_token {
    scan_argument_mode_t argument_mode = scan_argument_mode_t::sequential;
    std::size_t argument_index = 0; // 1-origin for positional mode
    scan_control_kind_t kind = scan_control_kind_t::at;
};

/**
 * @brief Represents a parsed scanset for %[...].
 */
struct scan_scanset_t {
    bool negated = false;
    bool ascii_table[128] = {};
    std::u32string extra_chars;
};

/**
 * @brief Represents a conversion token.
 */
struct scan_conversion_token {
    scan_argument_mode_t argument_mode = scan_argument_mode_t::sequential;
    std::size_t argument_index = 0; // 1-origin for positional mode

    bool suppress_assignment = false;
    std::size_t field_width = 0; // 0 means unlimited

    scan_length_t length = scan_length_t::none;
    scan_conversion_t conversion = scan_conversion_t::d;

    scan_scanset_t scanset;
};

/**
 * @brief Represents one parsed token from a scanf-style format string.
 */
using scan_token_t = std::variant<
    scan_literal_token,
    scan_whitespace_token,
    scan_control_token,
    scan_conversion_token>;

/**
 * @brief Represents a parsed scanf-style format string.
 */
struct scan_format_t {
    scan_argument_mode_t argument_mode = scan_argument_mode_t::unresolved;
    std::vector<scan_token_t> tokens;
};

/**
 * @brief Returns whether the specified byte is an ASCII digit.
 *
 * @param ch Source byte.
 * @return true if the byte is an ASCII digit.
 * @return false otherwise.
 */
[[nodiscard]] constexpr bool scan_is_ascii_digit(char8_t ch) noexcept {
    return ch >= u8'0' && ch <= u8'9';
}

/**
 * @brief Returns whether the specified byte is an ASCII whitespace character.
 *
 * @param ch Source byte.
 * @return true if the byte is an ASCII whitespace character.
 * @return false otherwise.
 */
[[nodiscard]] constexpr bool scan_is_ascii_space(char8_t ch) noexcept {
    return ch == u8' ' || ch == u8'\t' || ch == u8'\n' || ch == u8'\r' || ch == u8'\v' ||
           ch == u8'\f';
}

/**
 * @brief Returns whether the specified byte is ASCII.
 *
 * @param ch Source byte.
 * @return true if the byte is ASCII.
 * @return false otherwise.
 */
[[nodiscard]] constexpr bool scan_is_ascii_byte(char8_t ch) noexcept {
    return static_cast<unsigned char>(ch) <= 0x7fu;
}

/**
 * @brief Decodes one UTF-8 code point from the specified string.
 *
 * @param text Source UTF-8 string.
 * @param index In/out byte index.
 * @return Decoded code point on success.
 */
[[nodiscard]] inline std::expected<char32_t, error<void>> scan_decode_one_utf8(
    std::u8string_view text,
    std::size_t& index) {
    if (index >= text.size()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const unsigned char c0 = static_cast<unsigned char>(text[index]);

    if (c0 <= 0x7f) {
        ++index;
        return static_cast<char32_t>(c0);
    }

    if (c0 >= 0xc2 && c0 <= 0xdf) {
        if (index + 1 >= text.size()) {
            return std::unexpected(make_error(error_t::ilseq));
        }

        const unsigned char c1 = static_cast<unsigned char>(text[index + 1]);
        if ((c1 & 0xc0u) != 0x80u) {
            return std::unexpected(make_error(error_t::ilseq));
        }

        index += 2;
        return static_cast<char32_t>(((c0 & 0x1fu) << 6) | (c1 & 0x3fu));
    }

    if (c0 >= 0xe0 && c0 <= 0xef) {
        if (index + 2 >= text.size()) {
            return std::unexpected(make_error(error_t::ilseq));
        }

        const unsigned char c1 = static_cast<unsigned char>(text[index + 1]);
        const unsigned char c2 = static_cast<unsigned char>(text[index + 2]);

        if ((c1 & 0xc0u) != 0x80u || (c2 & 0xc0u) != 0x80u) {
            return std::unexpected(make_error(error_t::ilseq));
        }

        const char32_t value =
            (static_cast<char32_t>(c0 & 0x0fu) << 12) |
            (static_cast<char32_t>(c1 & 0x3fu) << 6) |
            static_cast<char32_t>(c2 & 0x3fu);

        if (value < 0x800u || (value >= 0xd800u && value <= 0xdfffu)) {
            return std::unexpected(make_error(error_t::ilseq));
        }

        index += 3;
        return value;
    }

    if (c0 >= 0xf0 && c0 <= 0xf4) {
        if (index + 3 >= text.size()) {
            return std::unexpected(make_error(error_t::ilseq));
        }

        const unsigned char c1 = static_cast<unsigned char>(text[index + 1]);
        const unsigned char c2 = static_cast<unsigned char>(text[index + 2]);
        const unsigned char c3 = static_cast<unsigned char>(text[index + 3]);

        if ((c1 & 0xc0u) != 0x80u || (c2 & 0xc0u) != 0x80u ||
            (c3 & 0xc0u) != 0x80u) {
            return std::unexpected(make_error(error_t::ilseq));
        }

        const char32_t value =
            (static_cast<char32_t>(c0 & 0x07u) << 18) |
            (static_cast<char32_t>(c1 & 0x3fu) << 12) |
            (static_cast<char32_t>(c2 & 0x3fu) << 6) |
            static_cast<char32_t>(c3 & 0x3fu);

        if (value < 0x10000u || value > 0x10ffffu) {
            return std::unexpected(make_error(error_t::ilseq));
        }

        index += 4;
        return value;
    }

    return std::unexpected(make_error(error_t::ilseq));
}

/**
 * @brief Parses a decimal number.
 *
 * @param format Source format string.
 * @param index In/out byte index.
 * @param value Output parsed value.
 * @return true if a decimal number was parsed.
 * @return false otherwise.
 */
[[nodiscard]] inline bool scan_parse_decimal_number(
    std::u8string_view format,
    std::size_t& index,
    std::size_t& value) noexcept {
    if (index >= format.size() || !scan_is_ascii_digit(format[index])) {
        return false;
    }

    std::size_t result = 0;

    while (index < format.size() && scan_is_ascii_digit(format[index])) {
        result = result * 10u + static_cast<std::size_t>(format[index] - u8'0');
        ++index;
    }

    value = result;
    return true;
}

/**
 * @brief Applies a token argument mode to the format-wide argument mode.
 *
 * @param format_mode In/out format-wide mode.
 * @param token_mode Token mode to apply.
 * @return Success on success.
 */
[[nodiscard]] inline std::expected<void, error<void>> scan_apply_argument_mode(
    scan_argument_mode_t& format_mode,
    scan_argument_mode_t token_mode) {
    if (token_mode == scan_argument_mode_t::unresolved) {
        return {};
    }

    if (format_mode == scan_argument_mode_t::unresolved) {
        format_mode = token_mode;
        return {};
    }

    if (format_mode != token_mode) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return {};
}

/**
 * @brief Parses a length modifier.
 *
 * @param format Source format string.
 * @param index In/out byte index.
 * @param length Output length modifier.
 */
inline void scan_parse_length_modifier(
    std::u8string_view format,
    std::size_t& index,
    scan_length_t& length) noexcept {
    length = scan_length_t::none;

    if (index >= format.size()) {
        return;
    }

    if (format[index] == u8'h') {
        if (index + 1 < format.size() && format[index + 1] == u8'h') {
            length = scan_length_t::hh;
            index += 2;
            return;
        }

        length = scan_length_t::h;
        ++index;
        return;
    }

    if (format[index] == u8'l') {
        if (index + 1 < format.size() && format[index + 1] == u8'l') {
            length = scan_length_t::ll;
            index += 2;
            return;
        }

        length = scan_length_t::l;
        ++index;
        return;
    }

    if (format[index] == u8'j') {
        length = scan_length_t::j;
        ++index;
        return;
    }

    if (format[index] == u8'z') {
        length = scan_length_t::z;
        ++index;
        return;
    }

    if (format[index] == u8't') {
        length = scan_length_t::t;
        ++index;
        return;
    }

    if (format[index] == u8'L') {
        length = scan_length_t::capital_l;
        ++index;
        return;
    }
}

/**
 * @brief Adds one code point to a scanset.
 *
 * @param scanset Target scanset.
 * @param value Code point to add.
 */
inline void scan_scanset_add_char(scan_scanset_t& scanset, char32_t value) noexcept {
    if (value <= 0x7f) {
        scanset.ascii_table[static_cast<unsigned char>(value)] = true;
        return;
    }

    scanset.extra_chars.push_back(value);
}

/**
 * @brief Parses a scanset body after the opening '['.
 *
 * @param format Source format string.
 * @param index In/out byte index positioned after '['.
 * @param scanset Output scanset.
 * @return Success on success.
 */
[[nodiscard]] inline std::expected<void, error<void>> scan_parse_scanset_body(
    std::u8string_view format,
    std::size_t& index,
    scan_scanset_t& scanset) {
    scanset = scan_scanset_t{};

    if (index >= format.size()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (format[index] == u8'^') {
        scanset.negated = true;
        ++index;
    }

    bool has_any_item = false;
    bool has_prev = false;
    char32_t prev = U'\0';
    bool prev_is_ascii = false;
    unsigned char prev_ascii = 0;

    if (index < format.size() && format[index] == u8']') {
        scan_scanset_add_char(scanset, U']');
        has_any_item = true;
        has_prev = true;
        prev = U']';
        prev_is_ascii = true;
        prev_ascii = static_cast<unsigned char>(']');
        ++index;
    }

    while (index < format.size()) {
        if (format[index] == u8']') {
            ++index;
            if (!has_any_item) {
                return std::unexpected(make_error(error_t::invalid_argument));
            }

            return {};
        }

        if (format[index] == u8'-' && has_prev) {
            if (index + 1 < format.size() && format[index + 1] != u8']') {
                const std::size_t next_start = index + 1;
                char32_t next_value = U'\0';
                bool next_is_ascii = false;
                unsigned char next_ascii = 0;

                if (scan_is_ascii_byte(format[next_start])) {
                    next_value = static_cast<char32_t>(format[next_start]);
                    next_is_ascii = true;
                    next_ascii = static_cast<unsigned char>(format[next_start]);
                    index = next_start + 1;
                } else {
                    std::size_t temp = next_start;
                    auto decoded = scan_decode_one_utf8(format, temp);
                    if (!decoded.has_value()) {
                        return std::unexpected(decoded.error());
                    }

                    next_value = *decoded;
                    index = temp;
                }

                if (prev_is_ascii && next_is_ascii) {
                    const unsigned char first = prev_ascii <= next_ascii ? prev_ascii : next_ascii;
                    const unsigned char last = prev_ascii <= next_ascii ? next_ascii : prev_ascii;

                    for (unsigned char ch = first; ch <= last; ++ch) {
                        scanset.ascii_table[ch] = true;
                    }

                    has_any_item = true;
                    has_prev = true;
                    prev = next_value;
                    prev_is_ascii = true;
                    prev_ascii = next_ascii;
                    continue;
                }

                scan_scanset_add_char(scanset, U'-');
                scan_scanset_add_char(scanset, next_value);
                has_any_item = true;
                has_prev = true;
                prev = next_value;
                prev_is_ascii = next_is_ascii;
                prev_ascii = next_ascii;
                continue;
            }

            scan_scanset_add_char(scanset, U'-');
            has_any_item = true;
            has_prev = true;
            prev = U'-';
            prev_is_ascii = true;
            prev_ascii = static_cast<unsigned char>('-');
            ++index;
            continue;
        }

        char32_t value = U'\0';
        bool value_is_ascii = false;
        unsigned char value_ascii = 0;

        if (scan_is_ascii_byte(format[index])) {
            value = static_cast<char32_t>(format[index]);
            value_is_ascii = true;
            value_ascii = static_cast<unsigned char>(format[index]);
            ++index;
        } else {
            auto decoded = scan_decode_one_utf8(format, index);
            if (!decoded.has_value()) {
                return std::unexpected(decoded.error());
            }

            value = *decoded;
        }

        scan_scanset_add_char(scanset, value);
        has_any_item = true;
        has_prev = true;
        prev = value;
        prev_is_ascii = value_is_ascii;
        prev_ascii = value_ascii;
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

/**
 * @brief Parses one percent token after the leading '%'.
 *
 * @param format Source format string.
 * @param index In/out byte index positioned after '%'.
 * @param result Output token.
 * @return Success on success.
 */
[[nodiscard]] inline std::expected<void, error<void>> scan_parse_percent_token(
    std::u8string_view format,
    std::size_t& index,
    scan_token_t& result) {
    if (index >= format.size()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (format[index] == u8'%') {
        ++index;
        scan_conversion_token token;
        token.conversion = scan_conversion_t::percent;
        result = token;
        return {};
    }

    std::size_t saved = index;
    std::size_t first_number = 0;
    bool has_positional_prefix = false;
    std::size_t positional_index = 0;

    if (scan_parse_decimal_number(format, index, first_number)) {
        if (index < format.size() && format[index] == u8'$') {
            if (first_number == 0) {
                return std::unexpected(make_error(error_t::invalid_argument));
            }

            has_positional_prefix = true;
            positional_index = first_number;
            ++index;
        } else {
            index = saved;
        }
    }

    if (index < format.size() && format[index] == u8'@') {
        ++index;

        scan_control_token token;
        token.kind = scan_control_kind_t::at;

        if (has_positional_prefix) {
            token.argument_mode = scan_argument_mode_t::positional;
            token.argument_index = positional_index;
        } else {
            token.argument_mode = scan_argument_mode_t::sequential;
        }

        result = token;
        return {};
    }

    scan_conversion_token token;

    if (has_positional_prefix) {
        token.argument_mode = scan_argument_mode_t::positional;
        token.argument_index = positional_index;
    } else {
        token.argument_mode = scan_argument_mode_t::sequential;
    }

    if (index < format.size() && format[index] == u8'*') {
        token.suppress_assignment = true;
        ++index;
    }

    if (index < format.size() && scan_is_ascii_digit(format[index])) {
        if (format[index] == u8'0') {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        std::size_t width = 0;
        (void)scan_parse_decimal_number(format, index, width);
        token.field_width = width;
    }

    scan_parse_length_modifier(format, index, token.length);

    if (index >= format.size()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const char8_t ch = format[index];

    switch (ch) {
        case u8'd':
            token.conversion = scan_conversion_t::d;
            ++index;
            result = token;
            return {};
        case u8'u':
            token.conversion = scan_conversion_t::u;
            ++index;
            result = token;
            return {};
        case u8'x':
            token.conversion = scan_conversion_t::x;
            ++index;
            result = token;
            return {};
        case u8'X':
            token.conversion = scan_conversion_t::capital_x;
            ++index;
            result = token;
            return {};
        case u8'o':
            token.conversion = scan_conversion_t::o;
            ++index;
            result = token;
            return {};
        case u8'f':
            token.conversion = scan_conversion_t::f;
            ++index;
            result = token;
            return {};
        case u8'F':
            token.conversion = scan_conversion_t::capital_f;
            ++index;
            result = token;
            return {};
        case u8'e':
            token.conversion = scan_conversion_t::e;
            ++index;
            result = token;
            return {};
        case u8'E':
            token.conversion = scan_conversion_t::capital_e;
            ++index;
            result = token;
            return {};
        case u8'g':
            token.conversion = scan_conversion_t::g;
            ++index;
            result = token;
            return {};
        case u8'G':
            token.conversion = scan_conversion_t::capital_g;
            ++index;
            result = token;
            return {};
        case u8'c':
            token.conversion = scan_conversion_t::c;
            ++index;
            result = token;
            return {};
        case u8's':
            token.conversion = scan_conversion_t::s;
            ++index;
            result = token;
            return {};
        case u8'[': {
            if (token.length != scan_length_t::none) {
                return std::unexpected(make_error(error_t::invalid_argument));
            }

            ++index;
            token.conversion = scan_conversion_t::scanset;

            auto scanset_result = scan_parse_scanset_body(format, index, token.scanset);
            if (!scanset_result.has_value()) {
                return std::unexpected(scanset_result.error());
            }

            result = token;
            return {};
        }
        default:
            return std::unexpected(make_error(error_t::invalid_argument));
    }
}

/**
 * @brief Parses a scanf-style format string into token form.
 *
 * @param format Source UTF-8 format string.
 * @return Parsed format on success.
 */
[[nodiscard]] inline std::expected<scan_format_t, error<void>> parse_scan_format(
    std::u8string_view format) {
    scan_format_t result;
    std::size_t index = 0;

    bool pending_control = false;
    scan_argument_mode_t pending_control_mode = scan_argument_mode_t::sequential;

    while (index < format.size()) {
        if (format[index] == u8'%') {
            ++index;
            scan_token_t token;
            auto token_result = scan_parse_percent_token(format, index, token);
            if (!token_result.has_value()) {
                return std::unexpected(token_result.error());
            }

            if (std::holds_alternative<scan_control_token>(token)) {
                if (pending_control) {
                    return std::unexpected(make_error(error_t::invalid_argument));
                }

                const auto& control = std::get<scan_control_token>(token);
                auto mode_result =
                    scan_apply_argument_mode(result.argument_mode, control.argument_mode);
                if (!mode_result.has_value()) {
                    return std::unexpected(mode_result.error());
                }

                pending_control = true;
                pending_control_mode = control.argument_mode;
                result.tokens.push_back(std::move(token));
                continue;
            }

            if (std::holds_alternative<scan_conversion_token>(token)) {
                const auto& conversion = std::get<scan_conversion_token>(token);

                scan_argument_mode_t effective_mode = conversion.argument_mode;

                if (pending_control) {
                    if (conversion.argument_mode == scan_argument_mode_t::positional &&
                        pending_control_mode != scan_argument_mode_t::positional) {
                        return std::unexpected(make_error(error_t::invalid_argument));
                    }

                    if (conversion.argument_mode == scan_argument_mode_t::sequential &&
                        conversion.conversion != scan_conversion_t::percent) {
                        effective_mode = pending_control_mode;
                    }
                }

                if (conversion.conversion != scan_conversion_t::percent) {
                    auto mode_result =
                        scan_apply_argument_mode(result.argument_mode, effective_mode);
                    if (!mode_result.has_value()) {
                        return std::unexpected(mode_result.error());
                    }
                }

                pending_control = false;
                result.tokens.push_back(std::move(token));
                continue;
            }

            result.tokens.push_back(std::move(token));
            continue;
        }

        if (scan_is_ascii_space(format[index])) {
            while (index < format.size() && scan_is_ascii_space(format[index])) {
                ++index;
            }

            result.tokens.push_back(scan_whitespace_token{});
            continue;
        }

        std::u8string literal;

        while (index < format.size() && format[index] != u8'%' &&
               !scan_is_ascii_space(format[index])) {
            literal.push_back(format[index]);
            ++index;
        }

        if (!literal.empty()) {
            result.tokens.push_back(scan_literal_token{std::move(literal)});
        }
    }

    if (pending_control) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return result;
}

} // namespace xer::detail

#endif // XER_BITS_SCANF_FORMAT_H_INCLUDED_

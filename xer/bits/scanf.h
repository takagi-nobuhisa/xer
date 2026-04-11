/**
 * @file xer/bits/scanf.h
 * @brief Internal scanf-style input functions.
 */

#pragma once

#ifndef XER_BITS_SCANF_H_INCLUDED_
#define XER_BITS_SCANF_H_INCLUDED_

#include <cstddef>
#include <cstdint>
#include <deque>
#include <expected>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include <xer/bits/ferror.h>
#include <xer/bits/scanf_format.h>
#include <xer/bits/standard_streams.h>
#include <xer/bits/text_stream_io.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Intermediate value type for one successful scanf-style conversion.
 */
using scan_intermediate_value_t =
    std::variant<std::int64_t, std::uint64_t, long double, char32_t, std::u8string>;

/**
 * @brief Returns whether the specified code point is an ASCII digit.
 *
 * @param ch Source code point.
 * @return true if the code point is an ASCII digit.
 * @return false otherwise.
 */
[[nodiscard]] constexpr bool scan_is_ascii_digit_cp(char32_t ch) noexcept {
    return ch >= U'0' && ch <= U'9';
}

/**
 * @brief Returns whether the specified code point is an ASCII whitespace character.
 *
 * @param ch Source code point.
 * @return true if the code point is an ASCII whitespace character.
 * @return false otherwise.
 */
[[nodiscard]] constexpr bool scan_is_ascii_space_cp(char32_t ch) noexcept {
    return ch == U' ' || ch == U'\t' || ch == U'\n' || ch == U'\r' || ch == U'\v' ||
           ch == U'\f';
}

/**
 * @brief Returns whether the specified code point is an ASCII hexadecimal digit.
 *
 * @param ch Source code point.
 * @return true if the code point is an ASCII hexadecimal digit.
 * @return false otherwise.
 */
[[nodiscard]] constexpr bool scan_is_ascii_xdigit_cp(char32_t ch) noexcept {
    return scan_is_ascii_digit_cp(ch) || (ch >= U'a' && ch <= U'f') || (ch >= U'A' && ch <= U'F');
}

/**
 * @brief Returns whether the specified code point is an ASCII octal digit.
 *
 * @param ch Source code point.
 * @return true if the code point is an ASCII octal digit.
 * @return false otherwise.
 */
[[nodiscard]] constexpr bool scan_is_ascii_octal_digit_cp(char32_t ch) noexcept {
    return ch >= U'0' && ch <= U'7';
}

/**
 * @brief Converts a UTF-8 string view to a byte string.
 *
 * @param value Source UTF-8 string.
 * @return Converted byte string.
 */
[[nodiscard]] inline std::string scan_to_byte_string(std::u8string_view value) {
    return std::string(
        reinterpret_cast<const char*>(value.data()),
        reinterpret_cast<const char*>(value.data() + value.size()));
}

/**
 * @brief Converts a byte string view to a UTF-8 string.
 *
 * @param value Source byte string.
 * @return Converted UTF-8 string.
 */
[[nodiscard]] inline std::u8string scan_to_u8string(std::string_view value) {
    std::u8string result;
    result.reserve(value.size());

    for (unsigned char ch : value) {
        result.push_back(static_cast<char8_t>(ch));
    }

    return result;
}

/**
 * @brief Appends one code point to a UTF-8 string.
 *
 * @param out Output UTF-8 string.
 * @param value Source code point.
 * @return Success on success.
 */
[[nodiscard]] inline std::expected<void, error<void>> scan_append_utf8(
    std::u8string& out,
    char32_t value) {
    if (value > 0x10ffffu || (value >= 0xd800u && value <= 0xdfffu)) {
        return std::unexpected(make_error(error_t::ilseq));
    }

    if (value <= 0x7fu) {
        out.push_back(static_cast<char8_t>(value));
        return {};
    }

    if (value <= 0x7ffu) {
        out.push_back(static_cast<char8_t>(0xc0u | ((value >> 6) & 0x1fu)));
        out.push_back(static_cast<char8_t>(0x80u | (value & 0x3fu)));
        return {};
    }

    if (value <= 0xffffu) {
        out.push_back(static_cast<char8_t>(0xe0u | ((value >> 12) & 0x0fu)));
        out.push_back(static_cast<char8_t>(0x80u | ((value >> 6) & 0x3fu)));
        out.push_back(static_cast<char8_t>(0x80u | (value & 0x3fu)));
        return {};
    }

    out.push_back(static_cast<char8_t>(0xf0u | ((value >> 18) & 0x07u)));
    out.push_back(static_cast<char8_t>(0x80u | ((value >> 12) & 0x3fu)));
    out.push_back(static_cast<char8_t>(0x80u | ((value >> 6) & 0x3fu)));
    out.push_back(static_cast<char8_t>(0x80u | (value & 0x3fu)));
    return {};
}

/**
 * @brief Returns whether the specified code point belongs to a scanset.
 *
 * @param scanset Source scanset.
 * @param ch Source code point.
 * @return true if the code point belongs to the scanset.
 * @return false otherwise.
 */
[[nodiscard]] inline bool scan_scanset_contains(
    const scan_scanset_t& scanset,
    char32_t ch) noexcept {
    bool matched = false;

    if (ch <= 0x7f) {
        matched = scanset.ascii_table[static_cast<unsigned char>(ch)];
    } else {
        for (char32_t candidate : scanset.extra_chars) {
            if (candidate == ch) {
                matched = true;
                break;
            }
        }
    }

    return scanset.negated ? !matched : matched;
}

/**
 * @brief Buffered reader for UTF-8 strings.
 */
class scan_string_reader {
public:
    /**
     * @brief Constructs a reader from a UTF-8 string.
     *
     * @param input Source UTF-8 input string.
     */
    explicit scan_string_reader(std::u8string_view input)
        : input_(input) {}

    /**
     * @brief Peeks the specified lookahead character.
     *
     * @param offset Lookahead offset.
     * @return Optional code point on success.
     */
    [[nodiscard]] std::expected<std::optional<char32_t>, error<void>> peek(
        std::size_t offset = 0) {
        auto fill_result = fill_to(offset);
        if (!fill_result.has_value()) {
            return std::unexpected(fill_result.error());
        }

        if (buffer_.size() <= offset) {
            return std::optional<char32_t>();
        }

        return buffer_[offset];
    }

    /**
     * @brief Consumes the specified number of already-buffered characters.
     *
     * @param count Number of characters to consume.
     */
    void consume(std::size_t count = 1) noexcept {
        while (count > 0 && !buffer_.empty()) {
            buffer_.pop_front();
            --count;
        }
    }

private:
    /**
     * @brief Fills the internal buffer up to the specified offset.
     *
     * @param offset Required lookahead offset.
     * @return Success on success.
     */
    [[nodiscard]] std::expected<void, error<void>> fill_to(std::size_t offset) {
        while (!eof_ && buffer_.size() <= offset) {
            if (source_index_ >= input_.size()) {
                eof_ = true;
                break;
            }

            std::size_t temp = source_index_;
            auto decoded = scan_decode_one_utf8(input_, temp);
            if (!decoded.has_value()) {
                return std::unexpected(decoded.error());
            }

            buffer_.push_back(*decoded);
            source_index_ = temp;
        }

        return {};
    }

    std::u8string_view input_;
    std::size_t source_index_ = 0;
    bool eof_ = false;
    std::deque<char32_t> buffer_;
};

/**
 * @brief Buffered reader for text streams.
 */
class scan_text_stream_reader {
public:
    /**
     * @brief Constructs a reader from a text stream.
     *
     * @param input Source text stream.
     */
    explicit scan_text_stream_reader(text_stream& input)
        : input_(input) {}

    /**
     * @brief Peeks the specified lookahead character.
     *
     * @param offset Lookahead offset.
     * @return Optional code point on success.
     */
    [[nodiscard]] std::expected<std::optional<char32_t>, error<void>> peek(
        std::size_t offset = 0) {
        auto fill_result = fill_to(offset);
        if (!fill_result.has_value()) {
            return std::unexpected(fill_result.error());
        }

        if (buffer_.size() <= offset) {
            return std::optional<char32_t>();
        }

        return buffer_[offset];
    }

    /**
     * @brief Consumes the specified number of already-buffered characters.
     *
     * @param count Number of characters to consume.
     */
    void consume(std::size_t count = 1) noexcept {
        while (count > 0 && !buffer_.empty()) {
            buffer_.pop_front();
            --count;
        }
    }

private:
    /**
     * @brief Fills the internal buffer up to the specified offset.
     *
     * @param offset Required lookahead offset.
     * @return Success on success.
     */
    [[nodiscard]] std::expected<void, error<void>> fill_to(std::size_t offset) {
        while (!eof_ && buffer_.size() <= offset) {
            auto ch = xer::fgetc(input_);
            if (!ch.has_value()) {
                if (xer::ferror(input_)) {
                    return std::unexpected(ch.error());
                }

                eof_ = true;
                break;
            }

            buffer_.push_back(*ch);
        }

        return {};
    }

    text_stream& input_;
    bool eof_ = false;
    std::deque<char32_t> buffer_;
};

/**
 * @brief Reads one character and consumes it.
 *
 * @tparam Reader Reader type.
 * @param reader Source reader.
 * @return Optional code point on success.
 */
template<typename Reader>
[[nodiscard]] std::expected<std::optional<char32_t>, error<void>> scan_read_one(
    Reader& reader) {
    auto ch = reader.peek();
    if (!ch.has_value()) {
        return std::unexpected(ch.error());
    }

    if (!ch->has_value()) {
        return std::optional<char32_t>();
    }

    reader.consume();
    return *ch;
}

/**
 * @brief Skips ASCII whitespace characters.
 *
 * @tparam Reader Reader type.
 * @param reader Source reader.
 * @return Success on success.
 */
template<typename Reader>
[[nodiscard]] std::expected<void, error<void>> scan_skip_ascii_whitespace(
    Reader& reader) {
    while (true) {
        auto ch = reader.peek();
        if (!ch.has_value()) {
            return std::unexpected(ch.error());
        }

        if (!ch->has_value() || !scan_is_ascii_space_cp(**ch)) {
            return {};
        }

        reader.consume();
    }
}

/**
 * @brief Matches a literal token.
 *
 * @tparam Reader Reader type.
 * @param reader Source reader.
 * @param text Source literal UTF-8 text.
 * @return true if the literal matched.
 * @return false otherwise.
 */
template<typename Reader>
[[nodiscard]] std::expected<bool, error<void>> scan_match_literal(
    Reader& reader,
    std::u8string_view text) {
    std::size_t index = 0;

    while (index < text.size()) {
        auto expected_ch = scan_decode_one_utf8(text, index);
        if (!expected_ch.has_value()) {
            return std::unexpected(expected_ch.error());
        }

        auto actual_ch = reader.peek();
        if (!actual_ch.has_value()) {
            return std::unexpected(actual_ch.error());
        }

        if (!actual_ch->has_value() || **actual_ch != *expected_ch) {
            return false;
        }

        reader.consume();
    }

    return true;
}

/**
 * @brief Collects a signed decimal lexeme.
 *
 * @tparam Reader Reader type.
 * @param reader Source reader.
 * @param width Maximum field width, or 0 for unlimited.
 * @return Collected lexeme on success. Empty optional means match failure.
 */
template<typename Reader>
[[nodiscard]] std::expected<std::optional<std::string>, error<void>>
scan_collect_signed_decimal_lexeme(
    Reader& reader,
    std::size_t width) {
    std::string result;
    std::size_t consumed = 0;
    bool has_digits = false;

    auto first = reader.peek();
    if (!first.has_value()) {
        return std::unexpected(first.error());
    }

    if (first->has_value() && (**first == U'+' || **first == U'-')) {
        if (width != 0 && width <= 1) {
            return std::optional<std::string>();
        }

        auto second = reader.peek(1);
        if (!second.has_value()) {
            return std::unexpected(second.error());
        }

        if (!second->has_value() || !scan_is_ascii_digit_cp(**second)) {
            return std::optional<std::string>();
        }

        result.push_back(static_cast<char>(**first));
        reader.consume();
        ++consumed;
    }

    while (true) {
        if (width != 0 && consumed >= width) {
            break;
        }

        auto ch = reader.peek();
        if (!ch.has_value()) {
            return std::unexpected(ch.error());
        }

        if (!ch->has_value() || !scan_is_ascii_digit_cp(**ch)) {
            break;
        }

        result.push_back(static_cast<char>(**ch));
        reader.consume();
        ++consumed;
        has_digits = true;
    }

    if (!has_digits) {
        return std::optional<std::string>();
    }

    return result;
}

/**
 * @brief Collects an unsigned-decimal lexeme.
 *
 * @tparam Reader Reader type.
 * @param reader Source reader.
 * @param width Maximum field width, or 0 for unlimited.
 * @return Collected lexeme on success. Empty optional means match failure.
 */
template<typename Reader>
[[nodiscard]] std::expected<std::optional<std::string>, error<void>>
scan_collect_unsigned_decimal_lexeme(
    Reader& reader,
    std::size_t width) {
    return scan_collect_signed_decimal_lexeme(reader, width);
}

/**
 * @brief Collects a hexadecimal lexeme.
 *
 * @tparam Reader Reader type.
 * @param reader Source reader.
 * @param width Maximum field width, or 0 for unlimited.
 * @return Collected lexeme on success. Empty optional means match failure.
 */
template<typename Reader>
[[nodiscard]] std::expected<std::optional<std::string>, error<void>>
scan_collect_hex_lexeme(
    Reader& reader,
    std::size_t width) {
    std::string result;
    std::size_t consumed = 0;
    bool has_digits = false;

    auto first = reader.peek();
    if (!first.has_value()) {
        return std::unexpected(first.error());
    }

    if (first->has_value() && (**first == U'+' || **first == U'-')) {
        if (width != 0 && width <= 1) {
            return std::optional<std::string>();
        }

        auto second = reader.peek(1);
        if (!second.has_value()) {
            return std::unexpected(second.error());
        }

        if (!second->has_value()) {
            return std::optional<std::string>();
        }

        const bool valid_after_sign =
            scan_is_ascii_xdigit_cp(**second) || **second == U'0';
        if (!valid_after_sign) {
            return std::optional<std::string>();
        }

        result.push_back(static_cast<char>(**first));
        reader.consume();
        ++consumed;
    }

    auto zero = reader.peek();
    if (!zero.has_value()) {
        return std::unexpected(zero.error());
    }

    if (zero->has_value() && **zero == U'0' && (width == 0 || consumed + 1 < width)) {
        auto x = reader.peek(1);
        if (!x.has_value()) {
            return std::unexpected(x.error());
        }

        if (x->has_value() && (**x == U'x' || **x == U'X') &&
            (width == 0 || consumed + 2 <= width)) {
            auto digit = reader.peek(2);
            if (!digit.has_value()) {
                return std::unexpected(digit.error());
            }

            if (digit->has_value() && scan_is_ascii_xdigit_cp(**digit)) {
                result.push_back('0');
                result.push_back(static_cast<char>(**x));
                reader.consume(2);
                consumed += 2;
            }
        }
    }

    while (true) {
        if (width != 0 && consumed >= width) {
            break;
        }

        auto ch = reader.peek();
        if (!ch.has_value()) {
            return std::unexpected(ch.error());
        }

        if (!ch->has_value() || !scan_is_ascii_xdigit_cp(**ch)) {
            break;
        }

        result.push_back(static_cast<char>(**ch));
        reader.consume();
        ++consumed;
        has_digits = true;
    }

    if (!has_digits) {
        return std::optional<std::string>();
    }

    return result;
}

/**
 * @brief Collects an octal lexeme.
 *
 * @tparam Reader Reader type.
 * @param reader Source reader.
 * @param width Maximum field width, or 0 for unlimited.
 * @return Collected lexeme on success. Empty optional means match failure.
 */
template<typename Reader>
[[nodiscard]] std::expected<std::optional<std::string>, error<void>>
scan_collect_octal_lexeme(
    Reader& reader,
    std::size_t width) {
    std::string result;
    std::size_t consumed = 0;
    bool has_digits = false;

    auto first = reader.peek();
    if (!first.has_value()) {
        return std::unexpected(first.error());
    }

    if (first->has_value() && (**first == U'+' || **first == U'-')) {
        if (width != 0 && width <= 1) {
            return std::optional<std::string>();
        }

        auto second = reader.peek(1);
        if (!second.has_value()) {
            return std::unexpected(second.error());
        }

        if (!second->has_value() || !scan_is_ascii_octal_digit_cp(**second)) {
            return std::optional<std::string>();
        }

        result.push_back(static_cast<char>(**first));
        reader.consume();
        ++consumed;
    }

    while (true) {
        if (width != 0 && consumed >= width) {
            break;
        }

        auto ch = reader.peek();
        if (!ch.has_value()) {
            return std::unexpected(ch.error());
        }

        if (!ch->has_value() || !scan_is_ascii_octal_digit_cp(**ch)) {
            break;
        }

        result.push_back(static_cast<char>(**ch));
        reader.consume();
        ++consumed;
        has_digits = true;
    }

    if (!has_digits) {
        return std::optional<std::string>();
    }

    return result;
}

/**
 * @brief Collects a floating-point lexeme.
 *
 * @tparam Reader Reader type.
 * @param reader Source reader.
 * @param width Maximum field width, or 0 for unlimited.
 * @return Collected lexeme on success. Empty optional means match failure.
 */
template<typename Reader>
[[nodiscard]] std::expected<std::optional<std::string>, error<void>>
scan_collect_float_lexeme(
    Reader& reader,
    std::size_t width) {
    std::string result;
    std::size_t consumed = 0;

    auto first = reader.peek();
    if (!first.has_value()) {
        return std::unexpected(first.error());
    }

    if (!first->has_value()) {
        return std::optional<std::string>();
    }

    bool has_sign = false;
    if (**first == U'+' || **first == U'-') {
        has_sign = true;
        if (width != 0 && width <= 1) {
            return std::optional<std::string>();
        }

        auto second = reader.peek(1);
        if (!second.has_value()) {
            return std::unexpected(second.error());
        }

        if (!second->has_value()) {
            return std::optional<std::string>();
        }

        const bool valid_start =
            scan_is_ascii_digit_cp(**second) ||
            (**second == U'.' && width != 0 ? width > 2 : true);
        if (!valid_start) {
            return std::optional<std::string>();
        }

        result.push_back(static_cast<char>(**first));
        reader.consume();
        ++consumed;
    }

    bool has_integer_digits = false;
    while (true) {
        if (width != 0 && consumed >= width) {
            break;
        }

        auto ch = reader.peek();
        if (!ch.has_value()) {
            return std::unexpected(ch.error());
        }

        if (!ch->has_value() || !scan_is_ascii_digit_cp(**ch)) {
            break;
        }

        result.push_back(static_cast<char>(**ch));
        reader.consume();
        ++consumed;
        has_integer_digits = true;
    }

    bool has_fraction_digits = false;
    auto dot = reader.peek();
    if (!dot.has_value()) {
        return std::unexpected(dot.error());
    }

    if (dot->has_value() && **dot == U'.' && (width == 0 || consumed < width)) {
        auto next = reader.peek(1);
        if (!next.has_value()) {
            return std::unexpected(next.error());
        }

        if (has_integer_digits || (next->has_value() && scan_is_ascii_digit_cp(**next))) {
            result.push_back('.');
            reader.consume();
            ++consumed;

            while (true) {
                if (width != 0 && consumed >= width) {
                    break;
                }

                auto ch = reader.peek();
                if (!ch.has_value()) {
                    return std::unexpected(ch.error());
                }

                if (!ch->has_value() || !scan_is_ascii_digit_cp(**ch)) {
                    break;
                }

                result.push_back(static_cast<char>(**ch));
                reader.consume();
                ++consumed;
                has_fraction_digits = true;
            }
        }
    }

    if (!has_integer_digits && !has_fraction_digits) {
        if (has_sign) {
            return std::optional<std::string>();
        }

        return std::optional<std::string>();
    }

    auto exp = reader.peek();
    if (!exp.has_value()) {
        return std::unexpected(exp.error());
    }

    if (exp->has_value() && (**exp == U'e' || **exp == U'E') && (width == 0 || consumed < width)) {
        std::size_t exp_pos = 1;
        std::size_t extra_needed = 1;

        auto exp_sign_or_digit = reader.peek(exp_pos);
        if (!exp_sign_or_digit.has_value()) {
            return std::unexpected(exp_sign_or_digit.error());
        }

        if (exp_sign_or_digit->has_value() &&
            (**exp_sign_or_digit == U'+' || **exp_sign_or_digit == U'-')) {
            ++exp_pos;
            ++extra_needed;
            exp_sign_or_digit = reader.peek(exp_pos);
            if (!exp_sign_or_digit.has_value()) {
                return std::unexpected(exp_sign_or_digit.error());
            }
        }

        if (exp_sign_or_digit->has_value() && scan_is_ascii_digit_cp(**exp_sign_or_digit) &&
            (width == 0 || consumed + extra_needed <= width)) {
            result.push_back(static_cast<char>(**exp));
            reader.consume();
            ++consumed;

            auto maybe_sign = reader.peek();
            if (!maybe_sign.has_value()) {
                return std::unexpected(maybe_sign.error());
            }

            if (maybe_sign->has_value() && (**maybe_sign == U'+' || **maybe_sign == U'-') &&
                (width == 0 || consumed < width)) {
                result.push_back(static_cast<char>(**maybe_sign));
                reader.consume();
                ++consumed;
            }

            while (true) {
                if (width != 0 && consumed >= width) {
                    break;
                }

                auto ch = reader.peek();
                if (!ch.has_value()) {
                    return std::unexpected(ch.error());
                }

                if (!ch->has_value() || !scan_is_ascii_digit_cp(**ch)) {
                    break;
                }

                result.push_back(static_cast<char>(**ch));
                reader.consume();
                ++consumed;
            }
        }
    }

    return result;
}

/**
 * @brief Collects a non-whitespace UTF-8 string.
 *
 * @tparam Reader Reader type.
 * @param reader Source reader.
 * @param width Maximum field width, or 0 for unlimited.
 * @return Collected string on success. Empty optional means match failure.
 */
template<typename Reader>
[[nodiscard]] std::expected<std::optional<std::u8string>, error<void>>
scan_collect_string_token(
    Reader& reader,
    std::size_t width) {
    std::u8string result;
    std::size_t consumed = 0;

    while (true) {
        if (width != 0 && consumed >= width) {
            break;
        }

        auto ch = reader.peek();
        if (!ch.has_value()) {
            return std::unexpected(ch.error());
        }

        if (!ch->has_value() || scan_is_ascii_space_cp(**ch)) {
            break;
        }

        auto append_result = scan_append_utf8(result, **ch);
        if (!append_result.has_value()) {
            return std::unexpected(append_result.error());
        }

        reader.consume();
        ++consumed;
    }

    if (result.empty()) {
        return std::optional<std::u8string>();
    }

    return result;
}

/**
 * @brief Collects a scanset-matching UTF-8 string.
 *
 * @tparam Reader Reader type.
 * @param reader Source reader.
 * @param token Source conversion token.
 * @return Collected string on success. Empty optional means match failure.
 */
template<typename Reader>
[[nodiscard]] std::expected<std::optional<std::u8string>, error<void>>
scan_collect_scanset_token(
    Reader& reader,
    const scan_conversion_token& token) {
    std::u8string result;
    std::size_t consumed = 0;

    while (true) {
        if (token.field_width != 0 && consumed >= token.field_width) {
            break;
        }

        auto ch = reader.peek();
        if (!ch.has_value()) {
            return std::unexpected(ch.error());
        }

        if (!ch->has_value() || !scan_scanset_contains(token.scanset, **ch)) {
            break;
        }

        auto append_result = scan_append_utf8(result, **ch);
        if (!append_result.has_value()) {
            return std::unexpected(append_result.error());
        }

        reader.consume();
        ++consumed;
    }

    if (result.empty()) {
        return std::optional<std::u8string>();
    }

    return result;
}

/**
 * @brief Parses a signed integer lexeme.
 *
 * @param lexeme Source lexeme.
 * @return Parsed signed integer on success.
 */
[[nodiscard]] inline std::expected<std::int64_t, error<void>> scan_parse_signed_integer(
    std::string_view lexeme) {
    std::istringstream stream{std::string(lexeme)};
    long long value = 0;
    stream >> std::dec >> value;

    if (!stream || !stream.eof()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return static_cast<std::int64_t>(value);
}

/**
 * @brief Parses an unsigned integer lexeme using the specified base.
 *
 * @param lexeme Source lexeme.
 * @param conversion Conversion kind.
 * @return Parsed unsigned integer on success.
 */
[[nodiscard]] inline std::expected<std::uint64_t, error<void>> scan_parse_unsigned_integer(
    std::string_view lexeme,
    scan_conversion_t conversion) {
    if (!lexeme.empty() && (lexeme.front() == '+' || lexeme.front() == '-')) {
        std::istringstream stream{std::string(lexeme)};
        long long signed_value = 0;

        switch (conversion) {
            case scan_conversion_t::u:
                stream >> std::dec >> signed_value;
                break;
            case scan_conversion_t::o:
                stream >> std::oct >> signed_value;
                break;
            case scan_conversion_t::x:
            case scan_conversion_t::capital_x:
                stream >> std::hex >> signed_value;
                break;
            default:
                return std::unexpected(make_error(error_t::invalid_argument));
        }

        if (!stream || !stream.eof()) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        return static_cast<std::uint64_t>(signed_value);
    }

    std::istringstream stream{std::string(lexeme)};
    unsigned long long value = 0;

    switch (conversion) {
        case scan_conversion_t::u:
            stream >> std::dec >> value;
            break;
        case scan_conversion_t::o:
            stream >> std::oct >> value;
            break;
        case scan_conversion_t::x:
        case scan_conversion_t::capital_x:
            stream >> std::hex >> value;
            break;
        default:
            return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (!stream || !stream.eof()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return static_cast<std::uint64_t>(value);
}

/**
 * @brief Parses a floating-point lexeme.
 *
 * @param lexeme Source lexeme.
 * @return Parsed floating-point value on success.
 */
[[nodiscard]] inline std::expected<long double, error<void>> scan_parse_floating(
    std::string_view lexeme) {
    std::istringstream stream{std::string(lexeme)};
    long double value = 0;
    stream >> value;

    if (!stream || !stream.eof()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return value;
}

/**
 * @brief Applies a signed-integer length modifier.
 *
 * @param value Parsed wide signed value.
 * @param length Length modifier.
 * @return Length-adjusted value.
 */
[[nodiscard]] inline std::int64_t scan_apply_signed_length(
    std::int64_t value,
    scan_length_t length) noexcept {
    switch (length) {
        case scan_length_t::hh:
            return static_cast<signed char>(value);
        case scan_length_t::h:
            return static_cast<short>(value);
        case scan_length_t::none:
            return static_cast<int>(value);
        case scan_length_t::l:
            return static_cast<long>(value);
        case scan_length_t::ll:
            return static_cast<long long>(value);
        case scan_length_t::j:
            return static_cast<std::intmax_t>(value);
        case scan_length_t::z:
            return static_cast<std::make_signed_t<std::size_t>>(value);
        case scan_length_t::t:
            return static_cast<std::ptrdiff_t>(value);
        case scan_length_t::capital_l:
            return value;
    }

    return value;
}

/**
 * @brief Applies an unsigned-integer length modifier.
 *
 * @param value Parsed wide unsigned value.
 * @param length Length modifier.
 * @return Length-adjusted value.
 */
[[nodiscard]] inline std::uint64_t scan_apply_unsigned_length(
    std::uint64_t value,
    scan_length_t length) noexcept {
    switch (length) {
        case scan_length_t::hh:
            return static_cast<unsigned char>(value);
        case scan_length_t::h:
            return static_cast<unsigned short>(value);
        case scan_length_t::none:
            return static_cast<unsigned int>(value);
        case scan_length_t::l:
            return static_cast<unsigned long>(value);
        case scan_length_t::ll:
            return static_cast<unsigned long long>(value);
        case scan_length_t::j:
            return static_cast<std::uintmax_t>(value);
        case scan_length_t::z:
            return static_cast<std::size_t>(value);
        case scan_length_t::t:
            return static_cast<std::make_unsigned_t<std::ptrdiff_t>>(value);
        case scan_length_t::capital_l:
            return value;
    }

    return value;
}

/**
 * @brief Applies a floating-point length modifier.
 *
 * @param value Parsed wide floating-point value.
 * @param length Length modifier.
 * @return Length-adjusted value.
 */
[[nodiscard]] inline long double scan_apply_floating_length(
    long double value,
    scan_length_t length) noexcept {
    switch (length) {
        case scan_length_t::capital_l:
            return value;
        case scan_length_t::l:
            return static_cast<double>(value);
        case scan_length_t::none:
            return static_cast<float>(value);
        default:
            return static_cast<float>(value);
    }
}

/**
 * @brief Reads one conversion and returns its intermediate value.
 *
 * @tparam Reader Reader type.
 * @param reader Source reader.
 * @param token Source conversion token.
 * @return Intermediate value on success. Empty optional means match failure.
 */
template<typename Reader>
[[nodiscard]] std::expected<std::optional<scan_intermediate_value_t>, error<void>>
scan_read_conversion(
    Reader& reader,
    const scan_conversion_token& token) {
    switch (token.conversion) {
        case scan_conversion_t::percent: {
            auto ch = reader.peek();
            if (!ch.has_value()) {
                return std::unexpected(ch.error());
            }

            if (!ch->has_value() || **ch != U'%') {
                return std::optional<scan_intermediate_value_t>();
            }

            reader.consume();
            return scan_intermediate_value_t(char32_t(U'%'));
        }

        case scan_conversion_t::d: {
            auto skip_result = scan_skip_ascii_whitespace(reader);
            if (!skip_result.has_value()) {
                return std::unexpected(skip_result.error());
            }

            auto lexeme = scan_collect_signed_decimal_lexeme(reader, token.field_width);
            if (!lexeme.has_value()) {
                return std::unexpected(lexeme.error());
            }

            if (!lexeme->has_value()) {
                return std::optional<scan_intermediate_value_t>();
            }

            auto parsed = scan_parse_signed_integer(**lexeme);
            if (!parsed.has_value()) {
                return std::unexpected(parsed.error());
            }

            return scan_intermediate_value_t(
                scan_apply_signed_length(*parsed, token.length));
        }

        case scan_conversion_t::u:
        case scan_conversion_t::o:
        case scan_conversion_t::x:
        case scan_conversion_t::capital_x: {
            auto skip_result = scan_skip_ascii_whitespace(reader);
            if (!skip_result.has_value()) {
                return std::unexpected(skip_result.error());
            }

            std::expected<std::optional<std::string>, error<void>> lexeme =
                std::optional<std::string>();

            switch (token.conversion) {
                case scan_conversion_t::u:
                    lexeme = scan_collect_unsigned_decimal_lexeme(reader, token.field_width);
                    break;
                case scan_conversion_t::o:
                    lexeme = scan_collect_octal_lexeme(reader, token.field_width);
                    break;
                case scan_conversion_t::x:
                case scan_conversion_t::capital_x:
                    lexeme = scan_collect_hex_lexeme(reader, token.field_width);
                    break;
                default:
                    break;
            }

            if (!lexeme.has_value()) {
                return std::unexpected(lexeme.error());
            }

            if (!lexeme->has_value()) {
                return std::optional<scan_intermediate_value_t>();
            }

            auto parsed = scan_parse_unsigned_integer(**lexeme, token.conversion);
            if (!parsed.has_value()) {
                return std::unexpected(parsed.error());
            }

            return scan_intermediate_value_t(
                scan_apply_unsigned_length(*parsed, token.length));
        }

        case scan_conversion_t::f:
        case scan_conversion_t::capital_f:
        case scan_conversion_t::e:
        case scan_conversion_t::capital_e:
        case scan_conversion_t::g:
        case scan_conversion_t::capital_g: {
            auto skip_result = scan_skip_ascii_whitespace(reader);
            if (!skip_result.has_value()) {
                return std::unexpected(skip_result.error());
            }

            auto lexeme = scan_collect_float_lexeme(reader, token.field_width);
            if (!lexeme.has_value()) {
                return std::unexpected(lexeme.error());
            }

            if (!lexeme->has_value()) {
                return std::optional<scan_intermediate_value_t>();
            }

            auto parsed = scan_parse_floating(**lexeme);
            if (!parsed.has_value()) {
                return std::unexpected(parsed.error());
            }

            return scan_intermediate_value_t(
                scan_apply_floating_length(*parsed, token.length));
        }

        case scan_conversion_t::c: {
            if (token.field_width > 1) {
                return std::unexpected(make_error(error_t::invalid_argument));
            }

            auto ch = scan_read_one(reader);
            if (!ch.has_value()) {
                return std::unexpected(ch.error());
            }

            if (!ch->has_value()) {
                return std::optional<scan_intermediate_value_t>();
            }

            return scan_intermediate_value_t(**ch);
        }

        case scan_conversion_t::s: {
            auto skip_result = scan_skip_ascii_whitespace(reader);
            if (!skip_result.has_value()) {
                return std::unexpected(skip_result.error());
            }

            auto value = scan_collect_string_token(reader, token.field_width);
            if (!value.has_value()) {
                return std::unexpected(value.error());
            }

            if (!value->has_value()) {
                return std::optional<scan_intermediate_value_t>();
            }

            return scan_intermediate_value_t(std::move(**value));
        }

        case scan_conversion_t::scanset: {
            auto value = scan_collect_scanset_token(reader, token);
            if (!value.has_value()) {
                return std::unexpected(value.error());
            }

            if (!value->has_value()) {
                return std::optional<scan_intermediate_value_t>();
            }

            return scan_intermediate_value_t(std::move(**value));
        }
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

/**
 * @brief Converts one intermediate value to text for generic stream extraction.
 *
 * @param value Source intermediate value.
 * @return UTF-8 text representation on success.
 */
[[nodiscard]] inline std::expected<std::u8string, error<void>> scan_intermediate_to_text(
    const scan_intermediate_value_t& value) {
    return std::visit(
        [](const auto& actual) -> std::expected<std::u8string, error<void>> {
            using actual_t = std::remove_cvref_t<decltype(actual)>;

            if constexpr (std::is_same_v<actual_t, std::u8string>) {
                return actual;
            } else if constexpr (std::is_same_v<actual_t, char32_t>) {
                std::u8string result;
                auto append_result = scan_append_utf8(result, actual);
                if (!append_result.has_value()) {
                    return std::unexpected(append_result.error());
                }

                return result;
            } else {
                std::ostringstream stream;
                stream << actual;
                return scan_to_u8string(stream.str());
            }
        },
        value);
}

/**
 * @brief Concept for char32_t targets.
 *
 * @tparam T Target type.
 */
template<typename T>
concept scan_char32_target =
    std::same_as<std::remove_cv_t<T>, char32_t>;

/**
 * @brief Concept for UTF-8 string targets.
 *
 * @tparam T Target type.
 */
template<typename T>
concept scan_u8string_target =
    std::same_as<std::remove_cv_t<T>, std::u8string>;

/**
 * @brief Concept for single-byte character targets.
 *
 * @tparam T Target type.
 */
template<typename T>
concept scan_byte_char_target =
    std::same_as<std::remove_cv_t<T>, char> ||
    std::same_as<std::remove_cv_t<T>, signed char> ||
    std::same_as<std::remove_cv_t<T>, unsigned char>;

/**
 * @brief Concept for other character-like targets.
 *
 * @tparam T Target type.
 */
template<typename T>
concept scan_other_char_target =
    std::same_as<std::remove_cv_t<T>, char8_t> ||
    std::same_as<std::remove_cv_t<T>, char16_t> ||
    std::same_as<std::remove_cv_t<T>, wchar_t>;

/**
 * @brief Concept for numeric-like scalar targets except character-like ones.
 *
 * @tparam T Target type.
 */
template<typename T>
concept scan_numeric_scalar_target =
    (std::is_arithmetic_v<std::remove_cv_t<T>> || std::is_enum_v<std::remove_cv_t<T>>) &&
    !scan_char32_target<T> &&
    !scan_byte_char_target<T> &&
    !scan_other_char_target<T>;

/**
 * @brief Stores one intermediate value into a char32_t target.
 *
 * @param out Output pointer.
 * @param value Source intermediate value.
 * @return Success on success.
 */
[[nodiscard]] inline std::expected<void, error<void>> scan_store_value(
    char32_t* out,
    const scan_intermediate_value_t& value) {
    if (out == nullptr) {
        return {};
    }

    if (!std::holds_alternative<char32_t>(value)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    *out = std::get<char32_t>(value);
    return {};
}

/**
 * @brief Stores one intermediate value into a single-byte character target.
 *
 * @tparam T Target type.
 * @param out Output pointer.
 * @param value Source intermediate value.
 * @return Success on success.
 */
template<scan_byte_char_target T>
[[nodiscard]] inline std::expected<void, error<void>> scan_store_value(
    T* out,
    const scan_intermediate_value_t& value) {
    if (out == nullptr) {
        return {};
    }

    if (!std::holds_alternative<char32_t>(value)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    *out = static_cast<T>(std::get<char32_t>(value));
    return {};
}

/**
 * @brief Stores one intermediate value into another character-like target.
 *
 * @tparam T Target type.
 * @param out Output pointer.
 * @param value Source intermediate value.
 * @return Success on success.
 */
template<scan_other_char_target T>
[[nodiscard]] inline std::expected<void, error<void>> scan_store_value(
    T* out,
    const scan_intermediate_value_t& value) {
    if (out == nullptr) {
        return {};
    }

    if (!std::holds_alternative<char32_t>(value)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    *out = static_cast<T>(std::get<char32_t>(value));
    return {};
}

/**
 * @brief Stores one intermediate value into a UTF-8 string target.
 *
 * @param out Output pointer.
 * @param value Source intermediate value.
 * @return Success on success.
 */
[[nodiscard]] inline std::expected<void, error<void>> scan_store_value(
    std::u8string* out,
    const scan_intermediate_value_t& value) {
    if (out == nullptr) {
        return {};
    }

    if (!std::holds_alternative<std::u8string>(value)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    *out = std::get<std::u8string>(value);
    return {};
}

/**
 * @brief Stores one intermediate value into a numeric scalar target.
 *
 * @tparam T Target type.
 * @param out Output pointer.
 * @param value Source intermediate value.
 * @return Success on success.
 */
template<scan_numeric_scalar_target T>
[[nodiscard]] inline std::expected<void, error<void>> scan_store_value(
    T* out,
    const scan_intermediate_value_t& value) {
    if (out == nullptr) {
        return {};
    }

    return std::visit(
        [out](const auto& actual) -> std::expected<void, error<void>> {
            using actual_t = std::remove_cvref_t<decltype(actual)>;

            if constexpr (std::is_same_v<actual_t, std::u8string> ||
                          std::is_same_v<actual_t, char32_t>) {
                return std::unexpected(make_error(error_t::invalid_argument));
            } else {
                *out = static_cast<T>(actual);
                return {};
            }
        },
        value);
}

/**
 * @brief Stores one intermediate value into a generic target by stream extraction.
 *
 * @tparam T Target type.
 * @param out Output pointer.
 * @param value Source intermediate value.
 * @return Success on success.
 */
template<typename T>
[[nodiscard]] inline std::expected<void, error<void>> scan_store_value(
    T* out,
    const scan_intermediate_value_t& value)
    requires(!scan_char32_target<T> && !scan_u8string_target<T> &&
             !scan_byte_char_target<T> && !scan_other_char_target<T> &&
             !scan_numeric_scalar_target<T>)
{
    if (out == nullptr) {
        return {};
    }

    auto text = scan_intermediate_to_text(value);
    if (!text.has_value()) {
        return std::unexpected(text.error());
    }

    std::istringstream stream{scan_to_byte_string(*text)};
    T temp{};
    stream >> temp;

    if (!stream || !stream.eof()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    *out = std::move(temp);
    return {};
}

/**
 * @brief Reads one conversion and stores it into the specified output pointer.
 *
 * @tparam Reader Reader type.
 * @tparam T Target type.
 * @param reader Source reader.
 * @param token Source conversion token.
 * @param out Output pointer.
 * @return true if the conversion matched.
 * @return false if the conversion did not match.
 */
template<typename Reader, typename T>
[[nodiscard]] std::expected<bool, error<void>> scan_arg(
    Reader& reader,
    const scan_conversion_token& token,
    T* out) {
    auto value = scan_read_conversion(reader, token);
    if (!value.has_value()) {
        return std::unexpected(value.error());
    }

    if (!value->has_value()) {
        return false;
    }

    auto store_result = scan_store_value(out, **value);
    if (!store_result.has_value()) {
        return std::unexpected(store_result.error());
    }

    return true;
}

/**
 * @brief Dispatches one conversion to the runtime-selected argument.
 *
 * @tparam Reader Reader type.
 * @tparam Tuple Tuple type.
 * @tparam Index Current tuple index.
 * @param reader Source reader.
 * @param token Source conversion token.
 * @param args Output-argument tuple.
 * @param target_index Runtime target index.
 * @param assigned_count In/out successful-assignment count.
 * @return true if the conversion matched.
 * @return false if the conversion did not match.
 */
template<typename Reader, typename Tuple, std::size_t Index = 0>
[[nodiscard]] std::expected<bool, error<void>> scan_dispatch_argument(
    Reader& reader,
    const scan_conversion_token& token,
    Tuple& args,
    std::size_t target_index,
    std::size_t& assigned_count) {
    if constexpr (Index >= std::tuple_size_v<Tuple>) {
        return std::unexpected(make_error(error_t::invalid_argument));
    } else {
        if (target_index == Index) {
            auto* out = std::get<Index>(args);
            auto result = scan_arg(reader, token, out);
            if (!result.has_value()) {
                return std::unexpected(result.error());
            }

            if (*result && out != nullptr) {
                ++assigned_count;
            }

            return *result;
        }

        return scan_dispatch_argument<Reader, Tuple, Index + 1>(
            reader,
            token,
            args,
            target_index,
            assigned_count);
    }
}

/**
 * @brief Runs one parsed format against a reader and argument tuple.
 *
 * @tparam Reader Reader type.
 * @tparam Tuple Tuple type.
 * @param reader Source reader.
 * @param format Parsed format.
 * @param args Output-argument tuple.
 * @return Assignment count on success.
 */
template<typename Reader, typename Tuple>
[[nodiscard]] std::expected<std::size_t, error<void>> scan_run_format(
    Reader& reader,
    const scan_format_t& format,
    Tuple& args) {
    std::size_t assigned_count = 0;
    std::size_t next_argument_index = 0;

    bool pending_control = false;
    scan_argument_mode_t pending_mode = scan_argument_mode_t::sequential;
    std::size_t pending_index = 0;

    for (const auto& token_variant : format.tokens) {
        if (std::holds_alternative<scan_literal_token>(token_variant)) {
            const auto& token = std::get<scan_literal_token>(token_variant);
            auto match_result = scan_match_literal(reader, token.text);
            if (!match_result.has_value()) {
                return std::unexpected(match_result.error());
            }

            if (!*match_result) {
                return assigned_count;
            }

            continue;
        }

        if (std::holds_alternative<scan_whitespace_token>(token_variant)) {
            auto skip_result = scan_skip_ascii_whitespace(reader);
            if (!skip_result.has_value()) {
                return std::unexpected(skip_result.error());
            }

            continue;
        }

        if (std::holds_alternative<scan_control_token>(token_variant)) {
            const auto& token = std::get<scan_control_token>(token_variant);
            pending_control = true;
            pending_mode = token.argument_mode;
            pending_index = token.argument_index;
            continue;
        }

        const auto& token = std::get<scan_conversion_token>(token_variant);

        if (token.conversion == scan_conversion_t::percent) {
            auto result = scan_arg(reader, token, static_cast<char32_t*>(nullptr));
            if (!result.has_value()) {
                return std::unexpected(result.error());
            }

            if (!*result) {
                return assigned_count;
            }

            continue;
        }

        scan_conversion_token effective_token = token;

        if (pending_control) {
            effective_token.argument_mode = pending_mode;
            effective_token.argument_index = pending_index;
            pending_control = false;
        }

        if (effective_token.suppress_assignment) {
            auto result = scan_arg(reader, effective_token, static_cast<char32_t*>(nullptr));
            if (!result.has_value()) {
                return std::unexpected(result.error());
            }

            if (!*result) {
                return assigned_count;
            }

            continue;
        }

        std::size_t target_index = 0;

        if (effective_token.argument_mode == scan_argument_mode_t::positional) {
            if (effective_token.argument_index == 0) {
                return std::unexpected(make_error(error_t::invalid_argument));
            }

            target_index = effective_token.argument_index - 1;
        } else {
            target_index = next_argument_index;
            ++next_argument_index;
        }

        auto dispatch_result =
            scan_dispatch_argument(reader, effective_token, args, target_index, assigned_count);
        if (!dispatch_result.has_value()) {
            return std::unexpected(dispatch_result.error());
        }

        if (!*dispatch_result) {
            return assigned_count;
        }
    }

    if (pending_control) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return assigned_count;
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Scans values from a UTF-8 string.
 *
 * @tparam Args Output target types.
 * @param input Source UTF-8 input string.
 * @param format Source UTF-8 format string.
 * @param args Output target pointers.
 * @return Assignment count on success.
 */
template<typename... Args>
[[nodiscard]] std::expected<std::size_t, error<void>> sscanf(
    std::u8string_view input,
    std::u8string_view format,
    Args*... args) {
    auto parsed = detail::parse_scan_format(format);
    if (!parsed.has_value()) {
        return std::unexpected(parsed.error());
    }

    detail::scan_string_reader reader(input);
    auto tuple = std::tuple<Args*...>(args...);
    return detail::scan_run_format(reader, *parsed, tuple);
}

/**
 * @brief Scans values from a text stream.
 *
 * @tparam Args Output target types.
 * @param input Source text stream.
 * @param format Source UTF-8 format string.
 * @param args Output target pointers.
 * @return Assignment count on success.
 */
template<typename... Args>
[[nodiscard]] std::expected<std::size_t, error<void>> fscanf(
    text_stream& input,
    std::u8string_view format,
    Args*... args) {
    auto parsed = detail::parse_scan_format(format);
    if (!parsed.has_value()) {
        return std::unexpected(parsed.error());
    }

    detail::scan_text_stream_reader reader(input);
    auto tuple = std::tuple<Args*...>(args...);
    return detail::scan_run_format(reader, *parsed, tuple);
}

/**
 * @brief Scans values from standard input.
 *
 * @tparam Args Output target types.
 * @param format Source UTF-8 format string.
 * @param args Output target pointers.
 * @return Assignment count on success.
 */
template<typename... Args>
[[nodiscard]] std::expected<std::size_t, error<void>> scanf(
    std::u8string_view format,
    Args*... args) {
    return fscanf(standard_input, format, args...);
}

} // namespace xer

#endif // XER_BITS_SCANF_H_INCLUDED_

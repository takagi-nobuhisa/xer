/**
 * @file xer/bits/unicode_code_point.h
 * @brief Unicode code point traversal utilities.
 */

#pragma once

#ifndef XER_BITS_UNICODE_CODE_POINT_H_INCLUDED_
#define XER_BITS_UNICODE_CODE_POINT_H_INCLUDED_

#include <concepts>
#include <cstddef>
#include <expected>
#include <iterator>
#include <string_view>
#include <type_traits>

#include <xer/error.h>
#include <xer/bits/unicode_common.h>

namespace xer {

/**
 * @brief Represents one decoded Unicode code point in a source string view.
 */
struct code_point {
    /**
     * @brief Offset in source code units.
     */
    std::size_t offset{};

    /**
     * @brief Number of source code units occupied by the code point.
     */
    std::size_t size{};

    /**
     * @brief Decoded Unicode scalar value.
     */
    char32_t value{};
};

namespace detail {

[[nodiscard]] inline auto unicode_unexpected(error_t code)
    -> result<code_point>
{
    return std::unexpected(make_error(code));
}


[[nodiscard]] constexpr auto utf8_continuation_byte(unsigned char byte) noexcept -> bool
{
    return (byte & 0xC0u) == 0x80u;
}

[[nodiscard]] constexpr auto utf8_sequence_length(unsigned char first) noexcept -> std::size_t
{
    if (first <= 0x7Fu) {
        return 1;
    }
    if (first >= 0xC2u && first <= 0xDFu) {
        return 2;
    }
    if (first >= 0xE0u && first <= 0xEFu) {
        return 3;
    }
    if (first >= 0xF0u && first <= 0xF4u) {
        return 4;
    }
    return 0;
}

[[nodiscard]] inline auto decode_next_utf8_code_point(
    std::u8string_view text,
    std::size_t offset) -> result<code_point>
{
    if (offset > text.size()) {
        return unicode_unexpected(error_t::out_of_range);
    }
    if (offset == text.size()) {
        return unicode_unexpected(error_t::out_of_range);
    }

    const auto byte0 = static_cast<unsigned char>(text[offset]);
    const std::size_t length = utf8_sequence_length(byte0);
    if (length == 0) {
        return unicode_unexpected(error_t::encoding_error);
    }
    if (text.size() - offset < length) {
        return unicode_unexpected(error_t::encoding_error);
    }

    unsigned char bytes[4] = {0, 0, 0, 0};
    bytes[0] = byte0;
    for (std::size_t i = 1; i < length; ++i) {
        bytes[i] = static_cast<unsigned char>(text[offset + i]);
        if (!utf8_continuation_byte(bytes[i])) {
            return unicode_unexpected(error_t::encoding_error);
        }
    }

    char32_t value = U'\0';
    if (length == 1) {
        value = static_cast<char32_t>(bytes[0]);
    } else if (length == 2) {
        value = static_cast<char32_t>(((bytes[0] & 0x1Fu) << 6) |
                                      (bytes[1] & 0x3Fu));
    } else if (length == 3) {
        value = static_cast<char32_t>(((bytes[0] & 0x0Fu) << 12) |
                                      ((bytes[1] & 0x3Fu) << 6) |
                                      (bytes[2] & 0x3Fu));
    } else {
        value = static_cast<char32_t>(((bytes[0] & 0x07u) << 18) |
                                      ((bytes[1] & 0x3Fu) << 12) |
                                      ((bytes[2] & 0x3Fu) << 6) |
                                      (bytes[3] & 0x3Fu));
    }

    if ((length == 3 && value < static_cast<char32_t>(0x0800)) ||
        (length == 4 && value < static_cast<char32_t>(0x10000)) ||
        !is_unicode_scalar_value(value)) {
        return unicode_unexpected(error_t::encoding_error);
    }

    return code_point{offset, length, value};
}

[[nodiscard]] inline auto decode_prev_utf8_code_point(
    std::u8string_view text,
    std::size_t offset) -> result<code_point>
{
    if (offset > text.size()) {
        return unicode_unexpected(error_t::out_of_range);
    }
    if (offset == 0) {
        return unicode_unexpected(error_t::out_of_range);
    }

    std::size_t start = offset - 1;
    std::size_t continuation_count = 0;
    while (start > 0 && utf8_continuation_byte(static_cast<unsigned char>(text[start]))) {
        --start;
        ++continuation_count;
        if (continuation_count > 3) {
            return unicode_unexpected(error_t::encoding_error);
        }
    }

    auto decoded = decode_next_utf8_code_point(text, start);
    if (!decoded.has_value()) {
        return decoded;
    }
    if (decoded->offset + decoded->size != offset) {
        return unicode_unexpected(error_t::encoding_error);
    }

    return decoded;
}

[[nodiscard]] inline auto decode_next_utf16_code_point(
    std::u16string_view text,
    std::size_t offset) -> result<code_point>
{
    if (offset > text.size()) {
        return unicode_unexpected(error_t::out_of_range);
    }
    if (offset == text.size()) {
        return unicode_unexpected(error_t::out_of_range);
    }

    const char16_t first = text[offset];
    if (is_unicode_high_surrogate(first)) {
        if (offset + 1 >= text.size()) {
            return unicode_unexpected(error_t::encoding_error);
        }
        const char16_t second = text[offset + 1];
        if (!is_unicode_low_surrogate(second)) {
            return unicode_unexpected(error_t::encoding_error);
        }
        return code_point{offset, 2, combine_unicode_surrogates(first, second)};
    }

    if (is_unicode_low_surrogate(first)) {
        return unicode_unexpected(error_t::encoding_error);
    }

    return code_point{offset, 1, static_cast<char32_t>(first)};
}

[[nodiscard]] inline auto decode_prev_utf16_code_point(
    std::u16string_view text,
    std::size_t offset) -> result<code_point>
{
    if (offset > text.size()) {
        return unicode_unexpected(error_t::out_of_range);
    }
    if (offset == 0) {
        return unicode_unexpected(error_t::out_of_range);
    }

    const char16_t last = text[offset - 1];
    if (is_unicode_low_surrogate(last)) {
        if (offset < 2) {
            return unicode_unexpected(error_t::encoding_error);
        }
        const char16_t first = text[offset - 2];
        if (!is_unicode_high_surrogate(first)) {
            return unicode_unexpected(error_t::encoding_error);
        }
        return code_point{offset - 2, 2, combine_unicode_surrogates(first, last)};
    }

    if (is_unicode_high_surrogate(last)) {
        return unicode_unexpected(error_t::encoding_error);
    }

    return code_point{offset - 1, 1, static_cast<char32_t>(last)};
}

[[nodiscard]] inline auto decode_next_wide_code_point(
    std::wstring_view text,
    std::size_t offset) -> result<code_point>
{
    if constexpr (sizeof(wchar_t) == 2) {
        if (offset > text.size()) {
            return unicode_unexpected(error_t::out_of_range);
        }
        if (offset == text.size()) {
            return unicode_unexpected(error_t::out_of_range);
        }

        const auto first = static_cast<char16_t>(text[offset]);
        if (is_unicode_high_surrogate(first)) {
            if (offset + 1 >= text.size()) {
                return unicode_unexpected(error_t::encoding_error);
            }
            const auto second = static_cast<char16_t>(text[offset + 1]);
            if (!is_unicode_low_surrogate(second)) {
                return unicode_unexpected(error_t::encoding_error);
            }
            return code_point{offset, 2, combine_unicode_surrogates(first, second)};
        }

        if (is_unicode_low_surrogate(first)) {
            return unicode_unexpected(error_t::encoding_error);
        }

        return code_point{offset, 1, static_cast<char32_t>(first)};
    } else if constexpr (sizeof(wchar_t) == 4) {
        if (offset > text.size()) {
            return unicode_unexpected(error_t::out_of_range);
        }
        if (offset == text.size()) {
            return unicode_unexpected(error_t::out_of_range);
        }

        const auto value = static_cast<char32_t>(text[offset]);
        if (!is_unicode_scalar_value(value)) {
            return unicode_unexpected(error_t::encoding_error);
        }
        return code_point{offset, 1, value};
    } else {
        static_assert(sizeof(wchar_t) == 2 || sizeof(wchar_t) == 4, "unsupported wchar_t size");
    }
}

[[nodiscard]] inline auto decode_prev_wide_code_point(
    std::wstring_view text,
    std::size_t offset) -> result<code_point>
{
    if constexpr (sizeof(wchar_t) == 2) {
        if (offset > text.size()) {
            return unicode_unexpected(error_t::out_of_range);
        }
        if (offset == 0) {
            return unicode_unexpected(error_t::out_of_range);
        }

        const auto last = static_cast<char16_t>(text[offset - 1]);
        if (is_unicode_low_surrogate(last)) {
            if (offset < 2) {
                return unicode_unexpected(error_t::encoding_error);
            }
            const auto first = static_cast<char16_t>(text[offset - 2]);
            if (!is_unicode_high_surrogate(first)) {
                return unicode_unexpected(error_t::encoding_error);
            }
            return code_point{offset - 2, 2, combine_unicode_surrogates(first, last)};
        }

        if (is_unicode_high_surrogate(last)) {
            return unicode_unexpected(error_t::encoding_error);
        }

        return code_point{offset - 1, 1, static_cast<char32_t>(last)};
    } else if constexpr (sizeof(wchar_t) == 4) {
        if (offset > text.size()) {
            return unicode_unexpected(error_t::out_of_range);
        }
        if (offset == 0) {
            return unicode_unexpected(error_t::out_of_range);
        }

        const std::size_t index = offset - 1;
        const auto value = static_cast<char32_t>(text[index]);
        if (!is_unicode_scalar_value(value)) {
            return unicode_unexpected(error_t::encoding_error);
        }
        return code_point{index, 1, value};
    } else {
        static_assert(sizeof(wchar_t) == 2 || sizeof(wchar_t) == 4, "unsupported wchar_t size");
    }
}

template<typename CharType>
struct code_point_decoder;

template<>
struct code_point_decoder<char8_t> {
    [[nodiscard]] static auto next(
        std::basic_string_view<char8_t> text,
        std::size_t offset) -> result<code_point>
    {
        return decode_next_utf8_code_point(text, offset);
    }
};

template<>
struct code_point_decoder<char16_t> {
    [[nodiscard]] static auto next(
        std::basic_string_view<char16_t> text,
        std::size_t offset) -> result<code_point>
    {
        return decode_next_utf16_code_point(text, offset);
    }
};

template<>
struct code_point_decoder<wchar_t> {
    [[nodiscard]] static auto next(
        std::basic_string_view<wchar_t> text,
        std::size_t offset) -> result<code_point>
    {
        return decode_next_wide_code_point(text, offset);
    }
};

template<typename CharType>
concept code_point_source_char =
    std::same_as<CharType, char8_t> ||
    std::same_as<CharType, char16_t> ||
    std::same_as<CharType, wchar_t>;

} // namespace detail

/**
 * @brief Decodes the next code point from a UTF-8 string view.
 * @param text Source UTF-8 string view.
 * @param offset Offset in UTF-8 code units.
 * @return Decoded code point or an error.
 */
[[nodiscard]] inline auto next_code_point(
    std::u8string_view text,
    std::size_t offset = 0) -> result<code_point>
{
    return detail::decode_next_utf8_code_point(text, offset);
}

/**
 * @brief Decodes the previous code point from a UTF-8 string view.
 * @param text Source UTF-8 string view.
 * @param offset One-past offset in UTF-8 code units.
 * @return Decoded code point or an error.
 */
[[nodiscard]] inline auto prev_code_point(
    std::u8string_view text,
    std::size_t offset) -> result<code_point>
{
    return detail::decode_prev_utf8_code_point(text, offset);
}

/**
 * @brief Decodes the next code point from a UTF-16 string view.
 * @param text Source UTF-16 string view.
 * @param offset Offset in UTF-16 code units.
 * @return Decoded code point or an error.
 */
[[nodiscard]] inline auto next_code_point(
    std::u16string_view text,
    std::size_t offset = 0) -> result<code_point>
{
    return detail::decode_next_utf16_code_point(text, offset);
}

/**
 * @brief Decodes the previous code point from a UTF-16 string view.
 * @param text Source UTF-16 string view.
 * @param offset One-past offset in UTF-16 code units.
 * @return Decoded code point or an error.
 */
[[nodiscard]] inline auto prev_code_point(
    std::u16string_view text,
    std::size_t offset) -> result<code_point>
{
    return detail::decode_prev_utf16_code_point(text, offset);
}

/**
 * @brief Decodes the next code point from a wide string view.
 * @param text Source wide string view.
 * @param offset Offset in wide code units.
 * @return Decoded code point or an error.
 */
[[nodiscard]] inline auto next_code_point(
    std::wstring_view text,
    std::size_t offset = 0) -> result<code_point>
{
    return detail::decode_next_wide_code_point(text, offset);
}

/**
 * @brief Decodes the previous code point from a wide string view.
 * @param text Source wide string view.
 * @param offset One-past offset in wide code units.
 * @return Decoded code point or an error.
 */
[[nodiscard]] inline auto prev_code_point(
    std::wstring_view text,
    std::size_t offset) -> result<code_point>
{
    return detail::decode_prev_wide_code_point(text, offset);
}

/**
 * @brief End sentinel for code point ranges.
 */
struct code_point_sentinel {};

/**
 * @brief Iterator over Unicode code points.
 *
 * The dereferenced value is `xer::result<xer::code_point>` so that malformed
 * source text is reported explicitly during traversal.
 *
 * @tparam CharType Source code unit type.
 */
template<detail::code_point_source_char CharType>
class code_point_iterator {
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = result<code_point>;
    using difference_type = std::ptrdiff_t;
    using reference = const value_type&;
    using pointer = const value_type*;

    constexpr code_point_iterator() noexcept = default;

    explicit code_point_iterator(std::basic_string_view<CharType> text)
        : text_(text)
    {
        if (text_.empty()) {
            at_end_ = true;
            return;
        }
        at_end_ = false;
        read_current();
    }

    [[nodiscard]] auto operator*() const noexcept -> reference
    {
        return current_;
    }

    [[nodiscard]] auto operator->() const noexcept -> pointer
    {
        return &current_;
    }

    auto operator++() -> code_point_iterator&
    {
        if (at_end_) {
            return *this;
        }
        if (!current_.has_value()) {
            at_end_ = true;
            return *this;
        }

        offset_ = current_->offset + current_->size;
        if (offset_ >= text_.size()) {
            at_end_ = true;
            return *this;
        }

        read_current();
        return *this;
    }

    auto operator++(int) -> void
    {
        ++(*this);
    }

    [[nodiscard]] friend auto operator==(
        const code_point_iterator& iterator,
        code_point_sentinel) noexcept -> bool
    {
        return iterator.at_end_;
    }

    [[nodiscard]] friend auto operator==(
        code_point_sentinel sentinel,
        const code_point_iterator& iterator) noexcept -> bool
    {
        return iterator == sentinel;
    }

    [[nodiscard]] friend auto operator!=(
        const code_point_iterator& iterator,
        code_point_sentinel sentinel) noexcept -> bool
    {
        return !(iterator == sentinel);
    }

    [[nodiscard]] friend auto operator!=(
        code_point_sentinel sentinel,
        const code_point_iterator& iterator) noexcept -> bool
    {
        return !(iterator == sentinel);
    }

private:
    auto read_current() -> void
    {
        current_ = detail::code_point_decoder<CharType>::next(text_, offset_);
    }

    std::basic_string_view<CharType> text_{};
    std::size_t offset_{};
    value_type current_ = std::unexpected(make_error(error_t::out_of_range));
    bool at_end_ = true;
};

/**
 * @brief Range object for code point traversal.
 * @tparam CharType Source code unit type.
 */
template<detail::code_point_source_char CharType>
class code_point_range {
public:
    explicit constexpr code_point_range(std::basic_string_view<CharType> text) noexcept
        : text_(text) {}

    [[nodiscard]] auto begin() const -> code_point_iterator<CharType>
    {
        return code_point_iterator<CharType>{text_};
    }

    [[nodiscard]] constexpr auto end() const noexcept -> code_point_sentinel
    {
        return {};
    }

private:
    std::basic_string_view<CharType> text_{};
};

/**
 * @brief Creates a code point range from a UTF-8 string view.
 * @param text Source UTF-8 string view.
 * @return Range whose elements are `xer::result<xer::code_point>`.
 */
[[nodiscard]] inline auto code_points(std::u8string_view text) noexcept
    -> code_point_range<char8_t>
{
    return code_point_range<char8_t>{text};
}

/**
 * @brief Creates a code point range from a UTF-16 string view.
 * @param text Source UTF-16 string view.
 * @return Range whose elements are `xer::result<xer::code_point>`.
 */
[[nodiscard]] inline auto code_points(std::u16string_view text) noexcept
    -> code_point_range<char16_t>
{
    return code_point_range<char16_t>{text};
}

/**
 * @brief Creates a code point range from a wide string view.
 * @param text Source wide string view.
 * @return Range whose elements are `xer::result<xer::code_point>`.
 */
[[nodiscard]] inline auto code_points(std::wstring_view text) noexcept
    -> code_point_range<wchar_t>
{
    return code_point_range<wchar_t>{text};
}

} // namespace xer

#endif /* XER_BITS_UNICODE_CODE_POINT_H_INCLUDED_ */

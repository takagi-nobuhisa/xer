/**
 * @file xer/bits/ja_to.h
 * @brief Japanese text conversion helpers.
 */

#pragma once

#ifndef XER_BITS_JA_TO_H_INCLUDED_
#define XER_BITS_JA_TO_H_INCLUDED_

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

#include <xer/bits/advanced_encoding.h>
#include <xer/bits/unicode_code_point.h>
#include <xer/error.h>

namespace xer::ja::detail {

[[nodiscard]] constexpr auto katakana_to_hiragana_code_point(
    const char32_t value) noexcept -> char32_t
{
    if (value >= U'ァ' && value <= U'ヶ') {
        return static_cast<char32_t>(value - 0x60);
    }

    if (value == U'ヽ') {
        return U'ゝ';
    }

    if (value == U'ヾ') {
        return U'ゞ';
    }

    return value;
}

[[nodiscard]] constexpr auto hiragana_to_katakana_code_point(
    const char32_t value) noexcept -> char32_t
{
    if (value >= U'ぁ' && value <= U'ゖ') {
        return static_cast<char32_t>(value + 0x60);
    }

    if (value == U'ゝ') {
        return U'ヽ';
    }

    if (value == U'ゞ') {
        return U'ヾ';
    }

    return value;
}

[[nodiscard]] inline auto append_utf8_code_point(
    std::u8string& output,
    const char32_t value) -> result<void>
{
    const std::uint32_t packed = xer::advanced::utf32_to_packed_utf8(value);
    if (packed == xer::advanced::detail::invalid_packed_utf8) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    output.push_back(static_cast<char8_t>(packed & 0xFFu));

    if ((packed >> 8) != 0) {
        output.push_back(static_cast<char8_t>((packed >> 8) & 0xFFu));
    }

    if ((packed >> 16) != 0) {
        output.push_back(static_cast<char8_t>((packed >> 16) & 0xFFu));
    }

    if ((packed >> 24) != 0) {
        output.push_back(static_cast<char8_t>((packed >> 24) & 0xFFu));
    }

    return {};
}

template<typename Transform>
[[nodiscard]] inline auto convert_utf8_text(
    const std::u8string_view text,
    Transform transform) -> result<std::u8string>
{
    std::u8string output;
    output.reserve(text.size());

    for (std::size_t offset = 0; offset < text.size();) {
        const auto decoded = xer::next_code_point(text, offset);
        if (!decoded.has_value()) {
            return std::unexpected(decoded.error());
        }

        const auto appended = append_utf8_code_point(
            output,
            transform(decoded->value));
        if (!appended.has_value()) {
            return std::unexpected(appended.error());
        }

        offset += decoded->size;
    }

    return output;
}

} // namespace xer::ja::detail

namespace xer::ja {

/**
 * @brief Converts fullwidth katakana in a UTF-8 string to hiragana.
 *
 * Katakana code points U+30A1..U+30F6 are mapped to the corresponding
 * hiragana code points U+3041..U+3096. Katakana iteration marks U+30FD and
 * U+30FE are also mapped to U+309D and U+309E. Other code points are kept
 * unchanged.
 *
 * @param text Source UTF-8 text.
 * @return Converted UTF-8 string on success, or an encoding error for invalid
 * UTF-8 input.
 */
[[nodiscard]] inline auto to_hiragana(
    const std::u8string_view text) -> result<std::u8string>
{
    return detail::convert_utf8_text(
        text,
        detail::katakana_to_hiragana_code_point);
}

/**
 * @brief Converts hiragana in a UTF-8 string to fullwidth katakana.
 *
 * Hiragana code points U+3041..U+3096 are mapped to the corresponding
 * fullwidth katakana code points U+30A1..U+30F6. Hiragana iteration marks
 * U+309D and U+309E are also mapped to U+30FD and U+30FE. Other code points
 * are kept unchanged.
 *
 * @param text Source UTF-8 text.
 * @return Converted UTF-8 string on success, or an encoding error for invalid
 * UTF-8 input.
 */
[[nodiscard]] inline auto to_katakana(
    const std::u8string_view text) -> result<std::u8string>
{
    return detail::convert_utf8_text(
        text,
        detail::hiragana_to_katakana_code_point);
}

} // namespace xer::ja

#endif /* XER_BITS_JA_TO_H_INCLUDED_ */

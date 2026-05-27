/**
 * @file xer/bits/unicode_emoji.h
 * @brief Practical emoji detection utilities.
 */

#pragma once

#ifndef XER_BITS_UNICODE_EMOJI_H_INCLUDED_
#define XER_BITS_UNICODE_EMOJI_H_INCLUDED_

#include <cstddef>
#include <expected>
#include <string_view>

#include <xer/error.h>
#include <xer/bits/unicode_code_point.h>
#include <xer/bits/unicode_grapheme_cluster.h>

namespace xer {

namespace detail {

[[nodiscard]] constexpr auto is_unicode_keycap_base(char32_t value) noexcept -> bool
{
    return value == U'#' ||
           value == U'*' ||
           (value >= U'0' && value <= U'9');
}

template<typename CharType>
[[nodiscard]] auto is_single_emoji_grapheme_cluster(
    std::basic_string_view<CharType> text) -> result<bool>
{
    if (text.empty()) {
        return false;
    }

    const auto cluster = next_grapheme_cluster(text, 0);
    if (!cluster.has_value()) {
        return std::unexpected(cluster.error());
    }
    if (cluster->offset != 0 || cluster->size != text.size()) {
        return false;
    }

    bool has_extended_pictographic = false;
    bool has_keycap_base = false;
    bool has_keycap_mark = false;
    std::size_t regional_indicator_count = 0;

    std::size_t offset = 0;
    while (offset < text.size()) {
        const auto point = next_code_point(text, offset);
        if (!point.has_value()) {
            return std::unexpected(point.error());
        }

        const char32_t value = point->value;
        if (is_unicode_extended_pictographic(value)) {
            has_extended_pictographic = true;
        }
        if (is_unicode_regional_indicator(value)) {
            ++regional_indicator_count;
        }
        if (is_unicode_keycap_base(value)) {
            has_keycap_base = true;
        }
        if (value == static_cast<char32_t>(0x20E3)) {
            has_keycap_mark = true;
        }

        offset = point->offset + point->size;
    }

    return has_extended_pictographic ||
           (regional_indicator_count >= 2u) ||
           (has_keycap_base && has_keycap_mark);
}

} // namespace detail

/**
 * @brief Checks whether a Unicode scalar value is treated as an emoji base.
 * @param value Unicode scalar value.
 * @return `true` if the value is a practical emoji base.
 */
[[nodiscard]] constexpr auto is_emoji(char32_t value) noexcept -> bool
{
    return detail::is_unicode_extended_pictographic(value) ||
           detail::is_unicode_regional_indicator(value);
}

/**
 * @brief Checks whether a UTF-8 string view is a single emoji grapheme cluster.
 * @param text Source UTF-8 string view.
 * @return `true` if the whole string is one practical emoji grapheme cluster.
 */
[[nodiscard]] inline auto is_emoji(std::u8string_view text) -> result<bool>
{
    return detail::is_single_emoji_grapheme_cluster(text);
}

/**
 * @brief Checks whether a UTF-16 string view is a single emoji grapheme cluster.
 * @param text Source UTF-16 string view.
 * @return `true` if the whole string is one practical emoji grapheme cluster.
 */
[[nodiscard]] inline auto is_emoji(std::u16string_view text) -> result<bool>
{
    return detail::is_single_emoji_grapheme_cluster(text);
}

/**
 * @brief Checks whether a wide string view is a single emoji grapheme cluster.
 * @param text Source wide string view.
 * @return `true` if the whole string is one practical emoji grapheme cluster.
 */
[[nodiscard]] inline auto is_emoji(std::wstring_view text) -> result<bool>
{
    return detail::is_single_emoji_grapheme_cluster(text);
}

} // namespace xer

#endif /* XER_BITS_UNICODE_EMOJI_H_INCLUDED_ */

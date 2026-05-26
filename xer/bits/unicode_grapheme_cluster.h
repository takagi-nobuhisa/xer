/**
 * @file xer/bits/unicode_grapheme_cluster.h
 * @brief Unicode grapheme cluster traversal utilities.
 */

#pragma once

#ifndef XER_BITS_UNICODE_GRAPHEME_CLUSTER_H_INCLUDED_
#define XER_BITS_UNICODE_GRAPHEME_CLUSTER_H_INCLUDED_

#include <concepts>
#include <cstddef>
#include <expected>
#include <iterator>
#include <string_view>

#include <xer/error.h>
#include <xer/bits/unicode_code_point.h>
#include <xer/bits/unicode_common.h>

namespace xer {

/**
 * @brief Represents one extended grapheme cluster in a source string view.
 */
struct grapheme_cluster {
    /**
     * @brief Offset in source code units.
     */
    std::size_t offset{};

    /**
     * @brief Number of source code units occupied by the cluster.
     */
    std::size_t size{};
};

namespace detail {

[[nodiscard]] inline auto grapheme_unexpected(error_t code)
    -> result<grapheme_cluster>
{
    return std::unexpected(make_error(code));
}

[[nodiscard]] constexpr auto is_unicode_control_for_grapheme(char32_t value) noexcept
    -> bool
{
    return (value <= static_cast<char32_t>(0x001F)) ||
           (value >= static_cast<char32_t>(0x007F) &&
            value <= static_cast<char32_t>(0x009F)) ||
           value == static_cast<char32_t>(0x00AD) ||
           value == static_cast<char32_t>(0x061C) ||
           value == static_cast<char32_t>(0x180E) ||
           value == static_cast<char32_t>(0x200B) ||
           value == static_cast<char32_t>(0x200E) ||
           value == static_cast<char32_t>(0x200F) ||
           (value >= static_cast<char32_t>(0x202A) &&
            value <= static_cast<char32_t>(0x202E)) ||
           (value >= static_cast<char32_t>(0x2060) &&
            value <= static_cast<char32_t>(0x2064)) ||
           (value >= static_cast<char32_t>(0x2066) &&
            value <= static_cast<char32_t>(0x206F)) ||
           value == static_cast<char32_t>(0xFEFF) ||
           (value >= static_cast<char32_t>(0xFFF0) &&
            value <= static_cast<char32_t>(0xFFF8));
}

[[nodiscard]] constexpr auto is_unicode_combining_mark(char32_t value) noexcept -> bool
{
    return (value >= static_cast<char32_t>(0x0300) && value <= static_cast<char32_t>(0x036F)) ||
           (value >= static_cast<char32_t>(0x0483) && value <= static_cast<char32_t>(0x0489)) ||
           (value >= static_cast<char32_t>(0x0591) && value <= static_cast<char32_t>(0x05BD)) ||
           value == static_cast<char32_t>(0x05BF) ||
           (value >= static_cast<char32_t>(0x05C1) && value <= static_cast<char32_t>(0x05C2)) ||
           (value >= static_cast<char32_t>(0x05C4) && value <= static_cast<char32_t>(0x05C5)) ||
           value == static_cast<char32_t>(0x05C7) ||
           (value >= static_cast<char32_t>(0x0610) && value <= static_cast<char32_t>(0x061A)) ||
           (value >= static_cast<char32_t>(0x064B) && value <= static_cast<char32_t>(0x065F)) ||
           value == static_cast<char32_t>(0x0670) ||
           (value >= static_cast<char32_t>(0x06D6) && value <= static_cast<char32_t>(0x06DC)) ||
           (value >= static_cast<char32_t>(0x06DF) && value <= static_cast<char32_t>(0x06E4)) ||
           (value >= static_cast<char32_t>(0x06E7) && value <= static_cast<char32_t>(0x06E8)) ||
           (value >= static_cast<char32_t>(0x06EA) && value <= static_cast<char32_t>(0x06ED)) ||
           (value >= static_cast<char32_t>(0x0711) && value <= static_cast<char32_t>(0x074A)) ||
           (value >= static_cast<char32_t>(0x07A6) && value <= static_cast<char32_t>(0x07B0)) ||
           (value >= static_cast<char32_t>(0x07EB) && value <= static_cast<char32_t>(0x07F3)) ||
           (value >= static_cast<char32_t>(0x0816) && value <= static_cast<char32_t>(0x0819)) ||
           (value >= static_cast<char32_t>(0x081B) && value <= static_cast<char32_t>(0x0823)) ||
           (value >= static_cast<char32_t>(0x0825) && value <= static_cast<char32_t>(0x0827)) ||
           (value >= static_cast<char32_t>(0x0829) && value <= static_cast<char32_t>(0x082D)) ||
           (value >= static_cast<char32_t>(0x0859) && value <= static_cast<char32_t>(0x085B)) ||
           (value >= static_cast<char32_t>(0x08D3) && value <= static_cast<char32_t>(0x08FF)) ||
           (value >= static_cast<char32_t>(0x0900) && value <= static_cast<char32_t>(0x0902)) ||
           value == static_cast<char32_t>(0x093A) ||
           value == static_cast<char32_t>(0x093C) ||
           (value >= static_cast<char32_t>(0x0941) && value <= static_cast<char32_t>(0x0948)) ||
           value == static_cast<char32_t>(0x094D) ||
           (value >= static_cast<char32_t>(0x0951) && value <= static_cast<char32_t>(0x0957)) ||
           (value >= static_cast<char32_t>(0x0962) && value <= static_cast<char32_t>(0x0963)) ||
           (value >= static_cast<char32_t>(0x0981) && value <= static_cast<char32_t>(0x0983)) ||
           value == static_cast<char32_t>(0x09BC) ||
           value == static_cast<char32_t>(0x09BE) ||
           (value >= static_cast<char32_t>(0x09C1) && value <= static_cast<char32_t>(0x09C4)) ||
           value == static_cast<char32_t>(0x09CD) ||
           value == static_cast<char32_t>(0x09D7) ||
           (value >= static_cast<char32_t>(0x0A01) && value <= static_cast<char32_t>(0x0A03)) ||
           value == static_cast<char32_t>(0x0A3C) ||
           (value >= static_cast<char32_t>(0x0A41) && value <= static_cast<char32_t>(0x0A42)) ||
           (value >= static_cast<char32_t>(0x0A47) && value <= static_cast<char32_t>(0x0A48)) ||
           (value >= static_cast<char32_t>(0x0A4B) && value <= static_cast<char32_t>(0x0A4D)) ||
           (value >= static_cast<char32_t>(0x0A70) && value <= static_cast<char32_t>(0x0A71)) ||
           (value >= static_cast<char32_t>(0x0A81) && value <= static_cast<char32_t>(0x0A83)) ||
           value == static_cast<char32_t>(0x0ABC) ||
           (value >= static_cast<char32_t>(0x0AC1) && value <= static_cast<char32_t>(0x0AC8)) ||
           value == static_cast<char32_t>(0x0ACD) ||
           (value >= static_cast<char32_t>(0x0B01) && value <= static_cast<char32_t>(0x0B03)) ||
           value == static_cast<char32_t>(0x0B3C) ||
           (value >= static_cast<char32_t>(0x0B3E) && value <= static_cast<char32_t>(0x0B44)) ||
           value == static_cast<char32_t>(0x0B4D) ||
           value == static_cast<char32_t>(0x0B57) ||
           (value >= static_cast<char32_t>(0x0BCD) && value <= static_cast<char32_t>(0x0BCD)) ||
           (value >= static_cast<char32_t>(0x0C00) && value <= static_cast<char32_t>(0x0C04)) ||
           (value >= static_cast<char32_t>(0x0C3E) && value <= static_cast<char32_t>(0x0C44)) ||
           value == static_cast<char32_t>(0x0C4D) ||
           (value >= static_cast<char32_t>(0x0D00) && value <= static_cast<char32_t>(0x0D03)) ||
           (value >= static_cast<char32_t>(0x0D3B) && value <= static_cast<char32_t>(0x0D44)) ||
           value == static_cast<char32_t>(0x0D4D) ||
           value == static_cast<char32_t>(0x0D57) ||
           (value >= static_cast<char32_t>(0x0E31) && value <= static_cast<char32_t>(0x0E3A)) ||
           (value >= static_cast<char32_t>(0x0E47) && value <= static_cast<char32_t>(0x0E4E)) ||
           (value >= static_cast<char32_t>(0x0EB1) && value <= static_cast<char32_t>(0x0EBC)) ||
           (value >= static_cast<char32_t>(0x0EC8) && value <= static_cast<char32_t>(0x0ECD)) ||
           (value >= static_cast<char32_t>(0x0F18) && value <= static_cast<char32_t>(0x0F19)) ||
           value == static_cast<char32_t>(0x0F35) ||
           value == static_cast<char32_t>(0x0F37) ||
           value == static_cast<char32_t>(0x0F39) ||
           (value >= static_cast<char32_t>(0x0F3E) && value <= static_cast<char32_t>(0x0F3F)) ||
           (value >= static_cast<char32_t>(0x0F71) && value <= static_cast<char32_t>(0x0F84)) ||
           (value >= static_cast<char32_t>(0x0F86) && value <= static_cast<char32_t>(0x0F87)) ||
           (value >= static_cast<char32_t>(0x0F8D) && value <= static_cast<char32_t>(0x0FBC)) ||
           (value >= static_cast<char32_t>(0x102B) && value <= static_cast<char32_t>(0x103E)) ||
           (value >= static_cast<char32_t>(0x1056) && value <= static_cast<char32_t>(0x1059)) ||
           (value >= static_cast<char32_t>(0x135D) && value <= static_cast<char32_t>(0x135F)) ||
           (value >= static_cast<char32_t>(0x1712) && value <= static_cast<char32_t>(0x1715)) ||
           (value >= static_cast<char32_t>(0x1732) && value <= static_cast<char32_t>(0x1734)) ||
           (value >= static_cast<char32_t>(0x1752) && value <= static_cast<char32_t>(0x1753)) ||
           (value >= static_cast<char32_t>(0x1772) && value <= static_cast<char32_t>(0x1773)) ||
           (value >= static_cast<char32_t>(0x1AB0) && value <= static_cast<char32_t>(0x1AFF)) ||
           (value >= static_cast<char32_t>(0x1DC0) && value <= static_cast<char32_t>(0x1DFF)) ||
           (value >= static_cast<char32_t>(0x20D0) && value <= static_cast<char32_t>(0x20FF)) ||
           (value >= static_cast<char32_t>(0xFE20) && value <= static_cast<char32_t>(0xFE2F));
}

[[nodiscard]] constexpr auto is_unicode_variation_selector(char32_t value) noexcept
    -> bool
{
    return (value >= static_cast<char32_t>(0xFE00) && value <= static_cast<char32_t>(0xFE0F)) ||
           (value >= static_cast<char32_t>(0xE0100) && value <= static_cast<char32_t>(0xE01EF));
}

[[nodiscard]] constexpr auto is_unicode_emoji_modifier(char32_t value) noexcept -> bool
{
    return value >= static_cast<char32_t>(0x1F3FB) &&
           value <= static_cast<char32_t>(0x1F3FF);
}

[[nodiscard]] constexpr auto is_unicode_tag_character(char32_t value) noexcept -> bool
{
    return value >= static_cast<char32_t>(0xE0020) &&
           value <= static_cast<char32_t>(0xE007F);
}

[[nodiscard]] constexpr auto is_unicode_extend_for_grapheme(char32_t value) noexcept
    -> bool
{
    return is_unicode_combining_mark(value) ||
           is_unicode_variation_selector(value) ||
           is_unicode_emoji_modifier(value) ||
           is_unicode_tag_character(value) ||
           value == static_cast<char32_t>(0x034F) ||
           value == static_cast<char32_t>(0x200C);
}

[[nodiscard]] constexpr auto is_unicode_spacing_mark_for_grapheme(char32_t value) noexcept
    -> bool
{
    return value == static_cast<char32_t>(0x0903) ||
           (value >= static_cast<char32_t>(0x093B) && value <= static_cast<char32_t>(0x0940)) ||
           (value >= static_cast<char32_t>(0x0949) && value <= static_cast<char32_t>(0x094C)) ||
           (value >= static_cast<char32_t>(0x0982) && value <= static_cast<char32_t>(0x0983)) ||
           (value >= static_cast<char32_t>(0x09BE) && value <= static_cast<char32_t>(0x09C0)) ||
           (value >= static_cast<char32_t>(0x09C7) && value <= static_cast<char32_t>(0x09CC)) ||
           (value >= static_cast<char32_t>(0x0A3E) && value <= static_cast<char32_t>(0x0A40)) ||
           (value >= static_cast<char32_t>(0x0ABE) && value <= static_cast<char32_t>(0x0AC0)) ||
           (value >= static_cast<char32_t>(0x0AC9) && value <= static_cast<char32_t>(0x0ACC)) ||
           (value >= static_cast<char32_t>(0x0B3E) && value <= static_cast<char32_t>(0x0B40)) ||
           (value >= static_cast<char32_t>(0x0B47) && value <= static_cast<char32_t>(0x0B4C)) ||
           (value >= static_cast<char32_t>(0x0BBE) && value <= static_cast<char32_t>(0x0BC2)) ||
           (value >= static_cast<char32_t>(0x0BC6) && value <= static_cast<char32_t>(0x0BCC)) ||
           value == static_cast<char32_t>(0x0BD7) ||
           (value >= static_cast<char32_t>(0x102B) && value <= static_cast<char32_t>(0x1031)) ||
           (value >= static_cast<char32_t>(0x1038) && value <= static_cast<char32_t>(0x1038)) ||
           (value >= static_cast<char32_t>(0x1A19) && value <= static_cast<char32_t>(0x1A1A));
}

[[nodiscard]] constexpr auto is_unicode_prepend_for_grapheme(char32_t value) noexcept
    -> bool
{
    return (value >= static_cast<char32_t>(0x0600) && value <= static_cast<char32_t>(0x0605)) ||
           value == static_cast<char32_t>(0x06DD) ||
           value == static_cast<char32_t>(0x070F) ||
           (value >= static_cast<char32_t>(0x0890) && value <= static_cast<char32_t>(0x0891));
}

[[nodiscard]] constexpr auto is_unicode_regional_indicator(char32_t value) noexcept
    -> bool
{
    return value >= static_cast<char32_t>(0x1F1E6) &&
           value <= static_cast<char32_t>(0x1F1FF);
}

[[nodiscard]] constexpr auto is_unicode_extended_pictographic(char32_t value) noexcept
    -> bool
{
    return (value >= static_cast<char32_t>(0x00A9) && value <= static_cast<char32_t>(0x00AE)) ||
           value == static_cast<char32_t>(0x203C) ||
           value == static_cast<char32_t>(0x2049) ||
           value == static_cast<char32_t>(0x2122) ||
           value == static_cast<char32_t>(0x2139) ||
           (value >= static_cast<char32_t>(0x2194) && value <= static_cast<char32_t>(0x21AA)) ||
           (value >= static_cast<char32_t>(0x231A) && value <= static_cast<char32_t>(0x231B)) ||
           value == static_cast<char32_t>(0x2328) ||
           value == static_cast<char32_t>(0x23CF) ||
           (value >= static_cast<char32_t>(0x23E9) && value <= static_cast<char32_t>(0x23F3)) ||
           (value >= static_cast<char32_t>(0x23F8) && value <= static_cast<char32_t>(0x23FA)) ||
           value == static_cast<char32_t>(0x24C2) ||
           (value >= static_cast<char32_t>(0x25AA) && value <= static_cast<char32_t>(0x25AB)) ||
           value == static_cast<char32_t>(0x25B6) ||
           value == static_cast<char32_t>(0x25C0) ||
           (value >= static_cast<char32_t>(0x25FB) && value <= static_cast<char32_t>(0x25FE)) ||
           (value >= static_cast<char32_t>(0x2600) && value <= static_cast<char32_t>(0x27BF)) ||
           (value >= static_cast<char32_t>(0x2934) && value <= static_cast<char32_t>(0x2935)) ||
           (value >= static_cast<char32_t>(0x2B05) && value <= static_cast<char32_t>(0x2B55)) ||
           value == static_cast<char32_t>(0x3030) ||
           value == static_cast<char32_t>(0x303D) ||
           value == static_cast<char32_t>(0x3297) ||
           value == static_cast<char32_t>(0x3299) ||
           (value >= static_cast<char32_t>(0x1F000) && value <= static_cast<char32_t>(0x1FAFF));
}

enum class grapheme_hangul_class {
    other,
    l,
    v,
    t,
    lv,
    lvt,
};

[[nodiscard]] constexpr auto get_grapheme_hangul_class(char32_t value) noexcept
    -> grapheme_hangul_class
{
    if ((value >= static_cast<char32_t>(0x1100) && value <= static_cast<char32_t>(0x115F)) ||
        (value >= static_cast<char32_t>(0xA960) && value <= static_cast<char32_t>(0xA97C))) {
        return grapheme_hangul_class::l;
    }
    if ((value >= static_cast<char32_t>(0x1160) && value <= static_cast<char32_t>(0x11A7)) ||
        (value >= static_cast<char32_t>(0xD7B0) && value <= static_cast<char32_t>(0xD7C6))) {
        return grapheme_hangul_class::v;
    }
    if ((value >= static_cast<char32_t>(0x11A8) && value <= static_cast<char32_t>(0x11FF)) ||
        (value >= static_cast<char32_t>(0xD7CB) && value <= static_cast<char32_t>(0xD7FB))) {
        return grapheme_hangul_class::t;
    }
    if (value >= static_cast<char32_t>(0xAC00) && value <= static_cast<char32_t>(0xD7A3)) {
        const auto index = static_cast<unsigned int>(value - static_cast<char32_t>(0xAC00));
        return (index % 28u) == 0u ? grapheme_hangul_class::lv :
                                     grapheme_hangul_class::lvt;
    }
    return grapheme_hangul_class::other;
}

struct grapheme_state {
    std::size_t regional_indicator_count{};
    bool saw_extended_pictographic{};
    bool saw_extended_pictographic_before_zwj{};
};

[[nodiscard]] constexpr auto should_break_grapheme_cluster(
    const code_point& previous,
    const code_point& current,
    const grapheme_state& state) noexcept -> bool
{
    const char32_t prev = previous.value;
    const char32_t curr = current.value;

    if (prev == static_cast<char32_t>(0x000D) && curr == static_cast<char32_t>(0x000A)) {
        return false;
    }
    if (prev == static_cast<char32_t>(0x000D) || prev == static_cast<char32_t>(0x000A) ||
        curr == static_cast<char32_t>(0x000D) || curr == static_cast<char32_t>(0x000A)) {
        return true;
    }
    if (is_unicode_control_for_grapheme(prev) || is_unicode_control_for_grapheme(curr)) {
        return true;
    }

    const auto prev_hangul = get_grapheme_hangul_class(prev);
    const auto curr_hangul = get_grapheme_hangul_class(curr);
    if (prev_hangul == grapheme_hangul_class::l &&
        (curr_hangul == grapheme_hangul_class::l ||
         curr_hangul == grapheme_hangul_class::v ||
         curr_hangul == grapheme_hangul_class::lv ||
         curr_hangul == grapheme_hangul_class::lvt)) {
        return false;
    }
    if ((prev_hangul == grapheme_hangul_class::lv ||
         prev_hangul == grapheme_hangul_class::v) &&
        (curr_hangul == grapheme_hangul_class::v ||
         curr_hangul == grapheme_hangul_class::t)) {
        return false;
    }
    if ((prev_hangul == grapheme_hangul_class::lvt ||
         prev_hangul == grapheme_hangul_class::t) &&
        curr_hangul == grapheme_hangul_class::t) {
        return false;
    }

    if (is_unicode_extend_for_grapheme(curr) || curr == static_cast<char32_t>(0x200D)) {
        return false;
    }
    if (is_unicode_spacing_mark_for_grapheme(curr)) {
        return false;
    }
    if (is_unicode_prepend_for_grapheme(prev)) {
        return false;
    }

    if (prev == static_cast<char32_t>(0x200D) &&
        is_unicode_extended_pictographic(curr) &&
        state.saw_extended_pictographic_before_zwj) {
        return false;
    }

    if (is_unicode_regional_indicator(prev) &&
        is_unicode_regional_indicator(curr) &&
        (state.regional_indicator_count % 2u) == 1u) {
        return false;
    }

    return true;
}

inline auto update_grapheme_state(
    grapheme_state& state,
    char32_t value) noexcept -> void
{
    if (is_unicode_regional_indicator(value)) {
        ++state.regional_indicator_count;
    } else {
        state.regional_indicator_count = 0;
    }

    if (value == static_cast<char32_t>(0x200D)) {
        state.saw_extended_pictographic_before_zwj = state.saw_extended_pictographic;
    } else if (is_unicode_extended_pictographic(value)) {
        state.saw_extended_pictographic = true;
    } else if (!is_unicode_extend_for_grapheme(value)) {
        state.saw_extended_pictographic_before_zwj = false;
    }
}

template<typename CharType>
struct grapheme_cluster_decoder {
    [[nodiscard]] static auto next(
        std::basic_string_view<CharType> text,
        std::size_t offset) -> result<grapheme_cluster>
    {
        if (offset > text.size()) {
            return grapheme_unexpected(error_t::out_of_range);
        }
        if (offset == text.size()) {
            return grapheme_unexpected(error_t::out_of_range);
        }

        auto first = code_point_decoder<CharType>::next(text, offset);
        if (!first.has_value()) {
            return std::unexpected(first.error());
        }
        if (first->offset != offset) {
            return grapheme_unexpected(error_t::encoding_error);
        }

        grapheme_state state{};
        update_grapheme_state(state, first->value);

        code_point previous = *first;
        std::size_t end = previous.offset + previous.size;
        while (end < text.size()) {
            auto current = code_point_decoder<CharType>::next(text, end);
            if (!current.has_value()) {
                break;
            }
            if (should_break_grapheme_cluster(previous, *current, state)) {
                break;
            }
            update_grapheme_state(state, current->value);
            previous = *current;
            end = previous.offset + previous.size;
        }

        return grapheme_cluster{offset, end - offset};
    }

    [[nodiscard]] static auto previous(
        std::basic_string_view<CharType> text,
        std::size_t offset) -> result<grapheme_cluster>
    {
        if (offset > text.size()) {
            return grapheme_unexpected(error_t::out_of_range);
        }
        if (offset == 0) {
            return grapheme_unexpected(error_t::out_of_range);
        }

        std::size_t position = 0;
        result<grapheme_cluster> previous = grapheme_unexpected(error_t::out_of_range);
        while (position < offset) {
            auto current = next(text, position);
            if (!current.has_value()) {
                return current;
            }
            const std::size_t end = current->offset + current->size;
            if (end > offset) {
                return grapheme_unexpected(error_t::encoding_error);
            }
            previous = current;
            position = end;
        }

        if (position != offset || !previous.has_value()) {
            return grapheme_unexpected(error_t::encoding_error);
        }
        return previous;
    }
};

} // namespace detail

/**
 * @brief Decodes the next extended grapheme cluster from a UTF-8 string view.
 * @param text Source UTF-8 string view.
 * @param offset Offset in UTF-8 code units.
 * @return Decoded grapheme cluster or an error.
 */
[[nodiscard]] inline auto next_grapheme_cluster(
    std::u8string_view text,
    std::size_t offset = 0) -> result<grapheme_cluster>
{
    return detail::grapheme_cluster_decoder<char8_t>::next(text, offset);
}

/**
 * @brief Decodes the previous extended grapheme cluster from a UTF-8 string view.
 * @param text Source UTF-8 string view.
 * @param offset One-past cluster boundary in UTF-8 code units.
 * @return Decoded grapheme cluster or an error.
 */
[[nodiscard]] inline auto prev_grapheme_cluster(
    std::u8string_view text,
    std::size_t offset) -> result<grapheme_cluster>
{
    return detail::grapheme_cluster_decoder<char8_t>::previous(text, offset);
}

/**
 * @brief Decodes the next extended grapheme cluster from a UTF-16 string view.
 * @param text Source UTF-16 string view.
 * @param offset Offset in UTF-16 code units.
 * @return Decoded grapheme cluster or an error.
 */
[[nodiscard]] inline auto next_grapheme_cluster(
    std::u16string_view text,
    std::size_t offset = 0) -> result<grapheme_cluster>
{
    return detail::grapheme_cluster_decoder<char16_t>::next(text, offset);
}

/**
 * @brief Decodes the previous extended grapheme cluster from a UTF-16 string view.
 * @param text Source UTF-16 string view.
 * @param offset One-past cluster boundary in UTF-16 code units.
 * @return Decoded grapheme cluster or an error.
 */
[[nodiscard]] inline auto prev_grapheme_cluster(
    std::u16string_view text,
    std::size_t offset) -> result<grapheme_cluster>
{
    return detail::grapheme_cluster_decoder<char16_t>::previous(text, offset);
}

/**
 * @brief Decodes the next extended grapheme cluster from a wide string view.
 * @param text Source wide string view.
 * @param offset Offset in wide code units.
 * @return Decoded grapheme cluster or an error.
 */
[[nodiscard]] inline auto next_grapheme_cluster(
    std::wstring_view text,
    std::size_t offset = 0) -> result<grapheme_cluster>
{
    return detail::grapheme_cluster_decoder<wchar_t>::next(text, offset);
}

/**
 * @brief Decodes the previous extended grapheme cluster from a wide string view.
 * @param text Source wide string view.
 * @param offset One-past cluster boundary in wide code units.
 * @return Decoded grapheme cluster or an error.
 */
[[nodiscard]] inline auto prev_grapheme_cluster(
    std::wstring_view text,
    std::size_t offset) -> result<grapheme_cluster>
{
    return detail::grapheme_cluster_decoder<wchar_t>::previous(text, offset);
}

/**
 * @brief End sentinel for grapheme cluster ranges.
 */
struct grapheme_cluster_sentinel {};

/**
 * @brief Iterator over extended grapheme clusters.
 *
 * The dereferenced value is `xer::result<xer::grapheme_cluster>` so that
 * malformed source text is reported explicitly during traversal.
 *
 * @tparam CharType Source code unit type.
 */
template<detail::code_point_source_char CharType>
class grapheme_cluster_iterator {
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = result<grapheme_cluster>;
    using difference_type = std::ptrdiff_t;
    using reference = const value_type&;
    using pointer = const value_type*;

    constexpr grapheme_cluster_iterator() noexcept = default;

    explicit grapheme_cluster_iterator(std::basic_string_view<CharType> text)
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

    auto operator++() -> grapheme_cluster_iterator&
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
        const grapheme_cluster_iterator& iterator,
        grapheme_cluster_sentinel) noexcept -> bool
    {
        return iterator.at_end_;
    }

    [[nodiscard]] friend auto operator==(
        grapheme_cluster_sentinel sentinel,
        const grapheme_cluster_iterator& iterator) noexcept -> bool
    {
        return iterator == sentinel;
    }

    [[nodiscard]] friend auto operator!=(
        const grapheme_cluster_iterator& iterator,
        grapheme_cluster_sentinel sentinel) noexcept -> bool
    {
        return !(iterator == sentinel);
    }

    [[nodiscard]] friend auto operator!=(
        grapheme_cluster_sentinel sentinel,
        const grapheme_cluster_iterator& iterator) noexcept -> bool
    {
        return !(iterator == sentinel);
    }

private:
    auto read_current() -> void
    {
        current_ = detail::grapheme_cluster_decoder<CharType>::next(text_, offset_);
    }

    std::basic_string_view<CharType> text_{};
    std::size_t offset_{};
    value_type current_ = std::unexpected(make_error(error_t::out_of_range));
    bool at_end_ = true;
};

/**
 * @brief Range object for extended grapheme cluster traversal.
 * @tparam CharType Source code unit type.
 */
template<detail::code_point_source_char CharType>
class grapheme_cluster_range {
public:
    explicit constexpr grapheme_cluster_range(
        std::basic_string_view<CharType> text) noexcept
        : text_(text) {}

    [[nodiscard]] auto begin() const -> grapheme_cluster_iterator<CharType>
    {
        return grapheme_cluster_iterator<CharType>{text_};
    }

    [[nodiscard]] constexpr auto end() const noexcept -> grapheme_cluster_sentinel
    {
        return {};
    }

private:
    std::basic_string_view<CharType> text_{};
};

/**
 * @brief Creates a grapheme cluster range from a UTF-8 string view.
 * @param text Source UTF-8 string view.
 * @return Range whose elements are `xer::result<xer::grapheme_cluster>`.
 */
[[nodiscard]] inline auto grapheme_clusters(std::u8string_view text) noexcept
    -> grapheme_cluster_range<char8_t>
{
    return grapheme_cluster_range<char8_t>{text};
}

/**
 * @brief Creates a grapheme cluster range from a UTF-16 string view.
 * @param text Source UTF-16 string view.
 * @return Range whose elements are `xer::result<xer::grapheme_cluster>`.
 */
[[nodiscard]] inline auto grapheme_clusters(std::u16string_view text) noexcept
    -> grapheme_cluster_range<char16_t>
{
    return grapheme_cluster_range<char16_t>{text};
}

/**
 * @brief Creates a grapheme cluster range from a wide string view.
 * @param text Source wide string view.
 * @return Range whose elements are `xer::result<xer::grapheme_cluster>`.
 */
[[nodiscard]] inline auto grapheme_clusters(std::wstring_view text) noexcept
    -> grapheme_cluster_range<wchar_t>
{
    return grapheme_cluster_range<wchar_t>{text};
}

} // namespace xer

#endif /* XER_BITS_UNICODE_GRAPHEME_CLUSTER_H_INCLUDED_ */

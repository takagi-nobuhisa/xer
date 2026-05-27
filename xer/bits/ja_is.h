/**
 * @file xer/bits/ja_is.h
 * @brief Japanese character classification helpers.
 */

#pragma once

#ifndef XER_BITS_JA_IS_H_INCLUDED_
#define XER_BITS_JA_IS_H_INCLUDED_

#include <cstddef>
#include <expected>
#include <string_view>

#include <xer/error.h>
#include <xer/bits/unicode_code_point.h>

namespace xer::ja::detail {

[[nodiscard]] constexpr auto is_in_unicode_range(
    const char32_t value,
    const char32_t first,
    const char32_t last) noexcept -> bool
{
    return value >= first && value <= last;
}

[[nodiscard]] constexpr auto is_hiragana_prolonged_sound_mark(
    const char32_t value) noexcept -> bool
{
    return value == U'ー';
}

[[nodiscard]] constexpr auto is_katakana_prolonged_sound_mark(
    const char32_t value) noexcept -> bool
{
    return value == U'ー' || value == U'ｰ';
}

template<typename Predicate>
[[nodiscard]] inline auto is_all_utf8_code_points(
    const std::u8string_view text,
    Predicate predicate) -> result<bool>
{
    if (text.empty()) {
        return false;
    }

    for (std::size_t offset = 0; offset < text.size();) {
        const auto decoded = xer::next_code_point(text, offset);
        if (!decoded.has_value()) {
            return std::unexpected(decoded.error());
        }

        if (!predicate(decoded->value)) {
            return false;
        }

        offset += decoded->size;
    }

    return true;
}

template<typename Predicate>
[[nodiscard]] inline auto contains_utf8_code_point(
    const std::u8string_view text,
    Predicate predicate) -> result<bool>
{
    if (text.empty()) {
        return false;
    }

    for (std::size_t offset = 0; offset < text.size();) {
        const auto decoded = xer::next_code_point(text, offset);
        if (!decoded.has_value()) {
            return std::unexpected(decoded.error());
        }

        if (predicate(decoded->value)) {
            return true;
        }

        offset += decoded->size;
    }

    return false;
}

} // namespace xer::ja::detail

namespace xer::ja {

/**
 * @brief Checks whether a code point is practical hiragana.
 *
 * This function covers the Unicode Hiragana block U+3040..U+309F and the
 * prolonged sound mark U+30FC. The prolonged sound mark is shared by
 * hiragana and katakana in practical Japanese text, so it is accepted by
 * both is_hiragana and is_katakana.
 */
[[nodiscard]] constexpr auto is_hiragana(
    const char32_t value) noexcept -> bool
{
    return detail::is_in_unicode_range(value, U'぀', U'ゟ')
        || detail::is_hiragana_prolonged_sound_mark(value);
}

/**
 * @brief Checks whether a code point is practical katakana.
 *
 * This function covers the Unicode Katakana block, Katakana Phonetic
 * Extensions, Kana Supplement, Kana Extended-A, Kana Extended-B, and
 * Halfwidth Katakana. It also accepts the halfwidth prolonged sound mark
 * U+FF70 because the fullwidth prolonged sound mark U+30FC is already in
 * the Katakana block.
 */
[[nodiscard]] constexpr auto is_katakana(
    const char32_t value) noexcept -> bool
{
    return detail::is_in_unicode_range(value, U'゠', U'ヿ')
        || detail::is_katakana_prolonged_sound_mark(value)
        || detail::is_in_unicode_range(value, U'ㇰ', U'ㇿ')
        || detail::is_in_unicode_range(value, U'𛀀', U'𛃿')
        || detail::is_in_unicode_range(value, U'𛄀', U'𛄯')
        || detail::is_in_unicode_range(value, U'𚿰', U'𚿿')
        || detail::is_in_unicode_range(value, U'ｦ', U'ﾟ');
}

/**
 * @brief Checks whether a code point is hiragana or katakana.
 */
[[nodiscard]] constexpr auto is_kana(const char32_t value) noexcept -> bool
{
    return is_hiragana(value) || is_katakana(value);
}

/**
 * @brief Checks whether a code point belongs to a CJK ideograph range.
 *
 * This function covers the common CJK unified ideograph ranges and CJK
 * compatibility ideograph ranges used by Japanese text. It does not attempt
 * to distinguish Japanese kanji usage from Chinese or Korean hanzi/hanja
 * usage because Unicode shares those code points.
 */
[[nodiscard]] constexpr auto is_kanji(const char32_t value) noexcept -> bool
{
    return detail::is_in_unicode_range(value, U'㐀', U'䶿')
        || detail::is_in_unicode_range(value, U'一', U'鿿')
        || detail::is_in_unicode_range(value, U'豈', U'﫿')
        || detail::is_in_unicode_range(value, U'𠀀', U'𪛟')
        || detail::is_in_unicode_range(value, U'𪜀', U'𫜿')
        || detail::is_in_unicode_range(value, U'𫝀', U'𫠟')
        || detail::is_in_unicode_range(value, U'𫠠', U'𬺯')
        || detail::is_in_unicode_range(value, U'𬺰', U'𮯯')
        || detail::is_in_unicode_range(value, U'丽', U'𯨟')
        || detail::is_in_unicode_range(value, U'𰀀', U'𱍏')
        || detail::is_in_unicode_range(value, U'𱍐', U'𲎯');
}

/**
 * @brief Checks whether a code point is punctuation commonly used in Japanese.
 *
 * This function covers CJK symbols and punctuation, fullwidth punctuation,
 * halfwidth Japanese punctuation, and common general punctuation marks used
 * in Japanese text.
 */
[[nodiscard]] constexpr auto is_japanese_punctuation(
    const char32_t value) noexcept -> bool
{
    return detail::is_in_unicode_range(value, U'　', U'〿')
        || detail::is_in_unicode_range(value, U'！', U'／')
        || detail::is_in_unicode_range(value, U'：', U'＠')
        || detail::is_in_unicode_range(value, U'［', U'｀')
        || detail::is_in_unicode_range(value, U'｛', U'･')
        || value == U'‐'
        || value == U'—'
        || value == U'―'
        || value == U'‘'
        || value == U'’'
        || value == U'“'
        || value == U'”'
        || value == U'‥'
        || value == U'…';
}

/**
 * @brief Checks whether a code point is Japanese kana, kanji, or punctuation.
 */
[[nodiscard]] constexpr auto is_japanese(
    const char32_t value) noexcept -> bool
{
    return is_kana(value)
        || is_kanji(value)
        || is_japanese_punctuation(value);
}


/**
 * @brief Checks whether a UTF-8 string contains practical hiragana.
 *
 * Empty input returns false. Invalid UTF-8 returns encoding_error.
 */
[[nodiscard]] inline auto contains_hiragana(
    const std::u8string_view text) -> result<bool>
{
    return detail::contains_utf8_code_point(
        text,
        [](const char32_t value) noexcept {
            return is_hiragana(value);
        });
}

/**
 * @brief Checks whether a UTF-8 string contains practical katakana.
 *
 * Empty input returns false. Invalid UTF-8 returns encoding_error.
 */
[[nodiscard]] inline auto contains_katakana(
    const std::u8string_view text) -> result<bool>
{
    return detail::contains_utf8_code_point(
        text,
        [](const char32_t value) noexcept {
            return is_katakana(value);
        });
}

/**
 * @brief Checks whether a UTF-8 string contains practical kana.
 *
 * Empty input returns false. Invalid UTF-8 returns encoding_error.
 */
[[nodiscard]] inline auto contains_kana(
    const std::u8string_view text) -> result<bool>
{
    return detail::contains_utf8_code_point(
        text,
        [](const char32_t value) noexcept {
            return is_kana(value);
        });
}

/**
 * @brief Checks whether a UTF-8 string contains a CJK ideograph.
 *
 * Empty input returns false. Invalid UTF-8 returns encoding_error.
 */
[[nodiscard]] inline auto contains_kanji(
    const std::u8string_view text) -> result<bool>
{
    return detail::contains_utf8_code_point(
        text,
        [](const char32_t value) noexcept {
            return is_kanji(value);
        });
}

/**
 * @brief Checks whether a UTF-8 string contains Japanese text.
 *
 * The accepted set is the same as is_japanese. Empty input returns false.
 * Invalid UTF-8 returns encoding_error.
 */
[[nodiscard]] inline auto contains_japanese(
    const std::u8string_view text) -> result<bool>
{
    return detail::contains_utf8_code_point(
        text,
        [](const char32_t value) noexcept {
            return is_japanese(value);
        });
}

/**
 * @brief Checks whether all UTF-8 code points are practical hiragana text.
 *
 * The accepted set is the same as is_hiragana. Empty input returns false.
 * Invalid UTF-8 returns encoding_error.
 */
[[nodiscard]] inline auto is_all_hiragana(
    const std::u8string_view text) -> result<bool>
{
    return detail::is_all_utf8_code_points(
        text,
        [](const char32_t value) noexcept {
            return is_hiragana(value);
        });
}

/**
 * @brief Checks whether all UTF-8 code points are practical katakana text.
 *
 * The accepted set is the same as is_katakana. Empty input returns false.
 * Invalid UTF-8 returns encoding_error.
 */
[[nodiscard]] inline auto is_all_katakana(
    const std::u8string_view text) -> result<bool>
{
    return detail::is_all_utf8_code_points(
        text,
        [](const char32_t value) noexcept {
            return is_katakana(value);
        });
}

/**
 * @brief Checks whether all UTF-8 code points are practical kana text.
 *
 * The accepted set is the same as is_kana. Empty input returns false.
 * Invalid UTF-8 returns encoding_error.
 */
[[nodiscard]] inline auto is_all_kana(
    const std::u8string_view text) -> result<bool>
{
    return detail::is_all_utf8_code_points(
        text,
        [](const char32_t value) noexcept {
            return is_kana(value);
        });
}

} // namespace xer::ja

#endif /* XER_BITS_JA_IS_H_INCLUDED_ */

/**
 * @file xer/bits/ja_is.h
 * @brief Japanese character classification helpers.
 */

#pragma once

#ifndef XER_BITS_JA_IS_H_INCLUDED_
#define XER_BITS_JA_IS_H_INCLUDED_

namespace xer::ja::detail {

[[nodiscard]] constexpr auto is_in_unicode_range(
    const char32_t value,
    const char32_t first,
    const char32_t last) noexcept -> bool
{
    return value >= first && value <= last;
}

} // namespace xer::ja::detail

namespace xer::ja {

/**
 * @brief Checks whether a code point belongs to the Hiragana block.
 *
 * This function uses the Unicode Hiragana block U+3040..U+309F.
 */
[[nodiscard]] constexpr auto is_hiragana(
    const char32_t value) noexcept -> bool
{
    return detail::is_in_unicode_range(value, U'぀', U'ゟ');
}

/**
 * @brief Checks whether a code point is katakana or halfwidth katakana.
 *
 * This function covers the Unicode Katakana block, Katakana Phonetic
 * Extensions, Kana Supplement, Kana Extended-A, Kana Extended-B, and
 * Halfwidth Katakana.
 */
[[nodiscard]] constexpr auto is_katakana(
    const char32_t value) noexcept -> bool
{
    return detail::is_in_unicode_range(value, U'゠', U'ヿ')
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

} // namespace xer::ja

#endif /* XER_BITS_JA_IS_H_INCLUDED_ */

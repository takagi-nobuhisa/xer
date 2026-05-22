/**
 * @file xer/bits/braille_chars.h
 * @brief Basic one-character braille conversion helpers.
 */

#pragma once

#ifndef XER_BITS_BRAILLE_CHARS_H_INCLUDED_
#define XER_BITS_BRAILLE_CHARS_H_INCLUDED_

#include <array>
#include <expected>
#include <string_view>

#include <xer/error.h>

namespace xer::braille {

namespace detail {

inline constexpr std::array<std::u8string_view, 26> alpha_braille_table = {
    u8"⠁", // a
    u8"⠃", // b
    u8"⠉", // c
    u8"⠙", // d
    u8"⠑", // e
    u8"⠋", // f
    u8"⠛", // g
    u8"⠓", // h
    u8"⠊", // i
    u8"⠚", // j
    u8"⠅", // k
    u8"⠇", // l
    u8"⠍", // m
    u8"⠝", // n
    u8"⠕", // o
    u8"⠏", // p
    u8"⠟", // q
    u8"⠗", // r
    u8"⠎", // s
    u8"⠞", // t
    u8"⠥", // u
    u8"⠧", // v
    u8"⠺", // w
    u8"⠭", // x
    u8"⠽", // y
    u8"⠵", // z
};

} // namespace detail

/**
 * @brief Converts one alphabetic character to a braille cell.
 *
 * This function assumes that the caller has already emitted any required
 * alphabetic, capital, or other mode indicator. Uppercase and lowercase letters
 * are mapped to the same braille cell.
 *
 * @param c ASCII alphabetic character.
 * @return Corresponding braille cell, or invalid_argument for unsupported input.
 */
[[nodiscard]] constexpr auto alpha_to_braille(char32_t c)
    -> result<std::u8string_view>
{
    if (c >= U'A' && c <= U'Z') {
        c = static_cast<char32_t>(c - U'A' + U'a');
    }

    if (c < U'a' || c > U'z') {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return detail::alpha_braille_table[static_cast<std::size_t>(c - U'a')];
}

/**
 * @brief Converts one digit character to a braille cell.
 *
 * This function assumes that the caller has already emitted any required
 * numeric indicator. Digits 1 to 9 are mapped to the same cells as a to i, and
 * digit 0 is mapped to the same cell as j.
 *
 * @param c ASCII digit character.
 * @return Corresponding braille cell, or invalid_argument for unsupported input.
 */
[[nodiscard]] constexpr auto digit_to_braille(char32_t c)
    -> result<std::u8string_view>
{
    if (c >= U'1' && c <= U'9') {
        return detail::alpha_braille_table[static_cast<std::size_t>(c - U'1')];
    }

    if (c == U'0') {
        return detail::alpha_braille_table[9];
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

/**
 * @brief Converts one alphanumeric character to a braille cell.
 *
 * This function is a small dispatcher for alpha_to_braille and
 * digit_to_braille. It does not emit alphabetic, capital, numeric, or other
 * mode indicators.
 *
 * @param c ASCII alphanumeric character.
 * @return Corresponding braille cell, or invalid_argument for unsupported input.
 */
[[nodiscard]] constexpr auto alnum_to_braille(char32_t c)
    -> result<std::u8string_view>
{
    if ((c >= U'A' && c <= U'Z') || (c >= U'a' && c <= U'z')) {
        return alpha_to_braille(c);
    }

    if (c >= U'0' && c <= U'9') {
        return digit_to_braille(c);
    }

    return std::unexpected(make_error(error_t::invalid_argument));
}

/**
 * @brief Converts one English braille punctuation character to a braille cell.
 *
 * This function maps the basic Grade 1 English braille punctuation marks. It
 * does not infer the position of ASCII quotation marks; use U+201C or U+201D
 * when an opening or closing quotation mark must be distinguished.
 *
 * @param c Punctuation character.
 * @return Corresponding braille cell, or invalid_argument for unsupported input.
 */
[[nodiscard]] constexpr auto punct_to_braille(char32_t c)
    -> result<std::u8string_view>
{
    switch (c) {
    case U',':
        return u8"⠂";
    case U';':
        return u8"⠆";
    case U':':
        return u8"⠒";
    case U'.':
        return u8"⠲";
    case U'!':
        return u8"⠖";
    case U'(':
    case U')':
        return u8"⠶";
    case U'?':
    case U'“':
        return u8"⠦";
    case U'*':
        return u8"⠔";
    case U'”':
        return u8"⠴";
    case U'\'':
        return u8"⠄";
    case U'-':
    case U'‐':
        return u8"⠤";
    default:
        return std::unexpected(make_error(error_t::invalid_argument));
    }
}


/**
 * @brief Converts one Japanese kana character to braille cells.
 *
 * This function maps one hiragana or katakana character to Japanese braille.
 * It handles basic kana, voiced kana, semi-voiced kana, the syllabic nasal,
 * the prolonged sound mark, and the sokuon. It does not combine multiple input
 * characters, so yoon such as きゃ or しゃ is intentionally out of scope.
 *
 * @param c Hiragana or katakana character.
 * @return Corresponding braille cells, or invalid_argument for unsupported input.
 */
[[nodiscard]] constexpr auto kana_to_braille(char32_t c)
    -> result<std::u8string_view>
{
    switch (c) {
    case U'あ':
    case U'ア':
        return u8"⠁";
    case U'い':
    case U'イ':
        return u8"⠃";
    case U'う':
    case U'ウ':
        return u8"⠉";
    case U'え':
    case U'エ':
        return u8"⠋";
    case U'お':
    case U'オ':
        return u8"⠊";

    case U'か':
    case U'カ':
        return u8"⠡";
    case U'き':
    case U'キ':
        return u8"⠣";
    case U'く':
    case U'ク':
        return u8"⠩";
    case U'け':
    case U'ケ':
        return u8"⠫";
    case U'こ':
    case U'コ':
        return u8"⠪";

    case U'さ':
    case U'サ':
        return u8"⠱";
    case U'し':
    case U'シ':
        return u8"⠳";
    case U'す':
    case U'ス':
        return u8"⠹";
    case U'せ':
    case U'セ':
        return u8"⠻";
    case U'そ':
    case U'ソ':
        return u8"⠺";

    case U'た':
    case U'タ':
        return u8"⠕";
    case U'ち':
    case U'チ':
        return u8"⠗";
    case U'つ':
    case U'ツ':
        return u8"⠝";
    case U'て':
    case U'テ':
        return u8"⠟";
    case U'と':
    case U'ト':
        return u8"⠞";

    case U'な':
    case U'ナ':
        return u8"⠅";
    case U'に':
    case U'ニ':
        return u8"⠇";
    case U'ぬ':
    case U'ヌ':
        return u8"⠍";
    case U'ね':
    case U'ネ':
        return u8"⠏";
    case U'の':
    case U'ノ':
        return u8"⠎";

    case U'は':
    case U'ハ':
        return u8"⠥";
    case U'ひ':
    case U'ヒ':
        return u8"⠧";
    case U'ふ':
    case U'フ':
        return u8"⠭";
    case U'へ':
    case U'ヘ':
        return u8"⠯";
    case U'ほ':
    case U'ホ':
        return u8"⠮";

    case U'ま':
    case U'マ':
        return u8"⠵";
    case U'み':
    case U'ミ':
        return u8"⠷";
    case U'む':
    case U'ム':
        return u8"⠽";
    case U'め':
    case U'メ':
        return u8"⠿";
    case U'も':
    case U'モ':
        return u8"⠾";

    case U'や':
    case U'ヤ':
        return u8"⠌";
    case U'ゆ':
    case U'ユ':
        return u8"⠬";
    case U'よ':
    case U'ヨ':
        return u8"⠜";

    case U'ら':
    case U'ラ':
        return u8"⠑";
    case U'り':
    case U'リ':
        return u8"⠓";
    case U'る':
    case U'ル':
        return u8"⠙";
    case U'れ':
    case U'レ':
        return u8"⠛";
    case U'ろ':
    case U'ロ':
        return u8"⠚";

    case U'わ':
    case U'ワ':
        return u8"⠄";
    case U'ゐ':
    case U'ヰ':
        return u8"⠆";
    case U'ゑ':
    case U'ヱ':
        return u8"⠖";
    case U'を':
    case U'ヲ':
        return u8"⠔";
    case U'ん':
    case U'ン':
        return u8"⠴";

    case U'ー':
        return u8"⠒";
    case U'っ':
    case U'ッ':
        return u8"⠂";

    case U'が':
    case U'ガ':
        return u8"⠐⠡";
    case U'ぎ':
    case U'ギ':
        return u8"⠐⠣";
    case U'ぐ':
    case U'グ':
        return u8"⠐⠩";
    case U'げ':
    case U'ゲ':
        return u8"⠐⠫";
    case U'ご':
    case U'ゴ':
        return u8"⠐⠪";

    case U'ざ':
    case U'ザ':
        return u8"⠐⠱";
    case U'じ':
    case U'ジ':
        return u8"⠐⠳";
    case U'ず':
    case U'ズ':
        return u8"⠐⠹";
    case U'ぜ':
    case U'ゼ':
        return u8"⠐⠻";
    case U'ぞ':
    case U'ゾ':
        return u8"⠐⠺";

    case U'だ':
    case U'ダ':
        return u8"⠐⠕";
    case U'ぢ':
    case U'ヂ':
        return u8"⠐⠗";
    case U'づ':
    case U'ヅ':
        return u8"⠐⠝";
    case U'で':
    case U'デ':
        return u8"⠐⠟";
    case U'ど':
    case U'ド':
        return u8"⠐⠞";

    case U'ば':
    case U'バ':
        return u8"⠐⠥";
    case U'び':
    case U'ビ':
        return u8"⠐⠧";
    case U'ぶ':
    case U'ブ':
        return u8"⠐⠭";
    case U'べ':
    case U'ベ':
        return u8"⠐⠯";
    case U'ぼ':
    case U'ボ':
        return u8"⠐⠮";

    case U'ぱ':
    case U'パ':
        return u8"⠠⠥";
    case U'ぴ':
    case U'ピ':
        return u8"⠠⠧";
    case U'ぷ':
    case U'プ':
        return u8"⠠⠭";
    case U'ぺ':
    case U'ペ':
        return u8"⠠⠯";
    case U'ぽ':
    case U'ポ':
        return u8"⠠⠮";

    default:
        return std::unexpected(make_error(error_t::invalid_argument));
    }
}


} // namespace xer::braille

#endif /* XER_BITS_BRAILLE_CHARS_H_INCLUDED_ */

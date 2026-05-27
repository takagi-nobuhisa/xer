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
#include <xer/bits/kana_width.h>
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

enum class kana_voice_mark {
    none,
    voiced,
    semi_voiced,
};

[[nodiscard]] constexpr auto kana_voice_mark_of(
    const char32_t value) noexcept -> kana_voice_mark
{
    if (value == U'゙' || value == U'゛' || value == U'ﾞ') {
        return kana_voice_mark::voiced;
    }

    if (value == U'゚' || value == U'゜' || value == U'ﾟ') {
        return kana_voice_mark::semi_voiced;
    }

    return kana_voice_mark::none;
}

[[nodiscard]] constexpr auto compose_hiragana_voice_mark(
    const char32_t base,
    const kana_voice_mark mark) noexcept -> char32_t
{
    if (mark == kana_voice_mark::voiced) {
        switch (base) {
            case U'う': return U'ゔ';
            case U'か': return U'が';
            case U'き': return U'ぎ';
            case U'く': return U'ぐ';
            case U'け': return U'げ';
            case U'こ': return U'ご';
            case U'さ': return U'ざ';
            case U'し': return U'じ';
            case U'す': return U'ず';
            case U'せ': return U'ぜ';
            case U'そ': return U'ぞ';
            case U'た': return U'だ';
            case U'ち': return U'ぢ';
            case U'つ': return U'づ';
            case U'て': return U'で';
            case U'と': return U'ど';
            case U'は': return U'ば';
            case U'ひ': return U'び';
            case U'ふ': return U'ぶ';
            case U'へ': return U'べ';
            case U'ほ': return U'ぼ';
            default: return U'\0';
        }
    }

    if (mark == kana_voice_mark::semi_voiced) {
        switch (base) {
            case U'は': return U'ぱ';
            case U'ひ': return U'ぴ';
            case U'ふ': return U'ぷ';
            case U'へ': return U'ぺ';
            case U'ほ': return U'ぽ';
            default: return U'\0';
        }
    }

    return U'\0';
}

[[nodiscard]] constexpr auto compose_katakana_voice_mark(
    const char32_t base,
    const kana_voice_mark mark) noexcept -> char32_t
{
    if (mark == kana_voice_mark::voiced) {
        switch (base) {
            case U'ウ': return U'ヴ';
            case U'カ': return U'ガ';
            case U'キ': return U'ギ';
            case U'ク': return U'グ';
            case U'ケ': return U'ゲ';
            case U'コ': return U'ゴ';
            case U'サ': return U'ザ';
            case U'シ': return U'ジ';
            case U'ス': return U'ズ';
            case U'セ': return U'ゼ';
            case U'ソ': return U'ゾ';
            case U'タ': return U'ダ';
            case U'チ': return U'ヂ';
            case U'ツ': return U'ヅ';
            case U'テ': return U'デ';
            case U'ト': return U'ド';
            case U'ハ': return U'バ';
            case U'ヒ': return U'ビ';
            case U'フ': return U'ブ';
            case U'ヘ': return U'ベ';
            case U'ホ': return U'ボ';
            case U'ワ': return U'ヷ';
            case U'ヰ': return U'ヸ';
            case U'ヱ': return U'ヹ';
            case U'ヲ': return U'ヺ';
            default: return U'\0';
        }
    }

    if (mark == kana_voice_mark::semi_voiced) {
        switch (base) {
            case U'ハ': return U'パ';
            case U'ヒ': return U'ピ';
            case U'フ': return U'プ';
            case U'ヘ': return U'ペ';
            case U'ホ': return U'ポ';
            default: return U'\0';
        }
    }

    return U'\0';
}

[[nodiscard]] constexpr auto compose_kana_voice_mark(
    const char32_t base,
    const kana_voice_mark mark) noexcept -> char32_t
{
    const char32_t hiragana = compose_hiragana_voice_mark(base, mark);
    if (hiragana != U'\0') {
        return hiragana;
    }

    return compose_katakana_voice_mark(base, mark);
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

/**
 * @brief Normalizes practical kana spelling in a UTF-8 string.
 *
 * This function keeps hiragana and katakana script choice unchanged. It
 * normalizes common kana spelling variants by converting halfwidth katakana to
 * fullwidth katakana and composing separated voiced or semi-voiced sound marks
 * with the preceding kana when a composed kana exists.
 *
 * Examples:
 *
 * - `ｶﾞ` becomes `ガ`.
 * - `ﾊﾟ` becomes `パ`.
 * - `か` followed by a combining or spacing voiced mark becomes `が`.
 * - `ハ` followed by a combining or spacing semi-voiced mark becomes `パ`.
 *
 * Other code points are kept unchanged.
 *
 * @param text Source UTF-8 text.
 * @return Normalized UTF-8 string on success, or an encoding error for invalid
 * UTF-8 input.
 */
[[nodiscard]] inline auto normalize_kana(
    const std::u8string_view text) -> result<std::u8string>
{
    std::u8string output;
    output.reserve(text.size());

    for (std::size_t offset = 0; offset < text.size();) {
        const auto decoded = xer::next_code_point(text, offset);
        if (!decoded.has_value()) {
            return std::unexpected(decoded.error());
        }

        const char32_t value = decoded->value;
        const std::size_t next_offset = offset + decoded->size;

        if (xer::detail::is_halfwidth_kana(value)) {
            if (next_offset < text.size()) {
                const auto next = xer::next_code_point(text, next_offset);
                if (!next.has_value()) {
                    return std::unexpected(next.error());
                }

                if (xer::detail::is_halfwidth_voiced_sound_mark(next->value) ||
                    xer::detail::is_halfwidth_semivoiced_sound_mark(next->value)) {
                    const char32_t composed = xer::detail::compose_halfwidth_kana(
                        value,
                        next->value);
                    if (composed != U'\0') {
                        const auto appended = detail::append_utf8_code_point(
                            output,
                            composed);
                        if (!appended.has_value()) {
                            return std::unexpected(appended.error());
                        }

                        offset = next_offset + next->size;
                        continue;
                    }
                }
            }

            const auto appended = detail::append_utf8_code_point(
                output,
                xer::detail::halfwidth_kana_to_fullwidth_base(value));
            if (!appended.has_value()) {
                return std::unexpected(appended.error());
            }

            offset = next_offset;
            continue;
        }

        if (next_offset < text.size()) {
            const auto next = xer::next_code_point(text, next_offset);
            if (!next.has_value()) {
                return std::unexpected(next.error());
            }

            const detail::kana_voice_mark mark = detail::kana_voice_mark_of(next->value);
            if (mark != detail::kana_voice_mark::none) {
                const char32_t composed = detail::compose_kana_voice_mark(
                    value,
                    mark);
                if (composed != U'\0') {
                    const auto appended = detail::append_utf8_code_point(
                        output,
                        composed);
                    if (!appended.has_value()) {
                        return std::unexpected(appended.error());
                    }

                    offset = next_offset + next->size;
                    continue;
                }
            }
        }

        const char32_t normalized = xer::detail::is_halfwidth_voiced_sound_mark(value) ||
                xer::detail::is_halfwidth_semivoiced_sound_mark(value)
            ? xer::detail::halfwidth_kana_to_fullwidth_base(value)
            : value;

        const auto appended = detail::append_utf8_code_point(output, normalized);
        if (!appended.has_value()) {
            return std::unexpected(appended.error());
        }

        offset = next_offset;
    }

    return output;
}

} // namespace xer::ja

#endif /* XER_BITS_JA_TO_H_INCLUDED_ */

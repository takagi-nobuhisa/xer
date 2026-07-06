/**
 * @file xer/bits/string_case.h
 * @brief Internal string character transformation implementations.
 */

#pragma once

#ifndef XER_BITS_STRING_CASE_H_INCLUDED_
#define XER_BITS_STRING_CASE_H_INCLUDED_

#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <string>
#include <string_view>
#include <xer/bits/kana_width.h>
#include <type_traits>
#include <vector>

#include <xer/bits/advanced_encoding.h>
#include <xer/bits/common.h>
#include <xer/bits/ja_to.h>
#include <xer/bits/string_character.h>
#include <xer/bits/unicode_common.h>
#include <xer/bits/string_read.h>
#include <xer/bits/toctrans.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Returns the effective length of an array string for transformation.
 *
 * If the final code unit is NUL, it is excluded. Otherwise, the whole array is
 * used. Embedded NUL code units are treated as ordinary code units.
 *
 * @tparam CharT Character type.
 * @tparam N Array size.
 * @param source Source array.
 * @return Effective code-unit length.
 */
template<typename CharT, std::size_t N>
    requires supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto transformation_array_string_length(
    const CharT (&source)[N]) noexcept -> std::size_t
{
    if constexpr (N == 0) {
        return 0;
    } else {
        using bare_char_t = std::remove_cv_t<CharT>;

        if (source[N - 1] == static_cast<bare_char_t>(0)) {
            return N - 1;
        }

        return N;
    }
}

/**
 * @brief Creates a string view for transformation from an array string.
 *
 * @tparam CharT Character type.
 * @tparam N Array size.
 * @param source Source array.
 * @return String view over the effective array contents.
 */
template<typename CharT, std::size_t N>
    requires supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] constexpr auto transformation_array_string_view(
    const CharT (&source)[N]) noexcept
    -> std::basic_string_view<std::remove_cv_t<CharT>>
{
    using bare_char_t = std::remove_cv_t<CharT>;
    return std::basic_string_view<bare_char_t>(
        source,
        transformation_array_string_length(source));
}

/**
 * @brief Appends a transformed byte-sized code point to a string.
 *
 * @tparam CharT Destination character type.
 * @param output Destination string.
 * @param value Code point to append.
 * @return Success or error.
 */
template<typename CharT>
    requires(std::same_as<CharT, char> || std::same_as<CharT, unsigned char>)
[[nodiscard]] inline auto append_byte_character(
    std::basic_string<CharT>& output,
    const char32_t value) -> result<void>
{
    if (value > static_cast<char32_t>((std::numeric_limits<unsigned char>::max)())) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    output.push_back(static_cast<CharT>(static_cast<unsigned char>(value)));
    return {};
}

/**
 * @brief Appends a UTF-8 encoded code point to a string.
 *
 * @param output Destination UTF-8 string.
 * @param value Code point to append.
 * @return Success or error.
 */
[[nodiscard]] inline auto append_utf8_character(
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

/**
 * @brief Appends a UTF-16 encoded code point to a string.
 *
 * @param output Destination UTF-16 string.
 * @param value Code point to append.
 * @return Success or error.
 */
[[nodiscard]] inline auto append_utf16_character(
    std::u16string& output,
    const char32_t value) -> result<void>
{
    const std::uint32_t packed = xer::advanced::utf32_to_packed_utf16(value);
    if (packed == xer::advanced::detail::invalid_packed_utf16) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    output.push_back(static_cast<char16_t>(packed & 0xFFFFu));

    if ((packed >> 16) != 0) {
        output.push_back(static_cast<char16_t>((packed >> 16) & 0xFFFFu));
    }

    return {};
}

/**
 * @brief Appends a UTF-32 code point to a string.
 *
 * @param output Destination UTF-32 string.
 * @param value Code point to append.
 * @return Success or error.
 */
[[nodiscard]] inline auto append_utf32_character(
    std::u32string& output,
    const char32_t value) -> result<void>
{
    if (!xer::detail::is_unicode_scalar_value(value)) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    output.push_back(value);
    return {};
}

/**
 * @brief Appends a code point encoded for the destination string character type.
 *
 * @tparam CharT Destination character type.
 * @param output Destination string.
 * @param value Code point to append.
 * @return Success or error.
 */
template<supported_string_character CharT>
[[nodiscard]] inline auto append_transformed_code_point(
    std::basic_string<CharT>& output,
    const char32_t value) -> result<void>
{
    if constexpr (std::same_as<CharT, char> ||
                  std::same_as<CharT, unsigned char>) {
        return append_byte_character(output, value);
    } else if constexpr (std::same_as<CharT, char8_t>) {
        return append_utf8_character(output, value);
    } else if constexpr (std::same_as<CharT, char16_t>) {
        return append_utf16_character(output, value);
    } else {
        return append_utf32_character(output, value);
    }
}


/**
 * @brief Decodes a string into Unicode code points for string-level transforms.
 *
 * @tparam CharT Source character type.
 * @param source Source string.
 * @return Decoded code points on success.
 */
template<supported_string_character CharT>
[[nodiscard]] inline auto decode_string_code_points(
    const std::basic_string_view<CharT> source) -> result<std::vector<char32_t>>
{
    std::vector<char32_t> output;
    output.reserve(source.size());

    if constexpr (std::same_as<CharT, char> ||
                  std::same_as<CharT, unsigned char>) {
        using unsigned_char_type = std::make_unsigned_t<CharT>;

        for (const CharT unit : source) {
            output.push_back(static_cast<char32_t>(
                static_cast<unsigned_char_type>(unit)));
        }
    } else if constexpr (std::same_as<CharT, char8_t>) {
        for (std::size_t index = 0; index < source.size();) {
            const auto decoded = decode_utf8_at(source, index);
            if (!decoded.has_value()) {
                return std::unexpected(decoded.error());
            }

            output.push_back(decoded->value);
            index += decoded->size;
        }
    } else if constexpr (std::same_as<CharT, char16_t>) {
        for (std::size_t index = 0; index < source.size();) {
            const auto decoded = decode_utf16_at(source, index);
            if (!decoded.has_value()) {
                return std::unexpected(decoded.error());
            }

            output.push_back(decoded->value);
            index += decoded->size;
        }
    } else {
        for (const char32_t value : source) {
            if (!xer::detail::is_unicode_scalar_value(value)) {
                return std::unexpected(make_error(error_t::encoding_error));
            }

            output.push_back(value);
        }
    }

    return output;
}


/**
 * @brief Normalizes ordinary fullwidth Katakana to Hiragana for romanization.
 *
 * Hiragana is returned unchanged. Fullwidth Katakana is mapped by the same
 * code-point helper used by xer::ja::to_hiragana(). The prolonged sound mark
 * is preserved. Unsupported code points are returned unchanged so the caller
 * can reject them through the normal romanization validation path.
 *
 * @param value Source code point.
 * @return Normalized code point.
 */
[[nodiscard]] constexpr auto normalize_romaji_kana(
    const char32_t value) noexcept -> char32_t
{
    return xer::ja::detail::katakana_to_hiragana_code_point(value);
}

/**
 * @brief Returns whether a code point is a small Y-row Hiragana.
 *
 * @param value Code point to test.
 * @return True for small ya, small yu, and small yo.
 */
[[nodiscard]] constexpr auto is_romaji_small_y(
    const char32_t value) noexcept -> bool
{
    return value == U'ゃ' || value == U'ゅ' || value == U'ょ';
}

/**
 * @brief Returns the vowel represented by an ordinary Hiragana vowel.
 *
 * @param value Hiragana vowel.
 * @return ASCII vowel or NUL when the code point is not an ordinary vowel.
 */
[[nodiscard]] constexpr auto romaji_vowel_kana(
    const char32_t value) noexcept -> char
{
    switch (value) {
        case U'あ':
            return 'a';
        case U'い':
            return 'i';
        case U'う':
            return 'u';
        case U'え':
            return 'e';
        case U'お':
            return 'o';
        default:
            return '\0';
    }
}

/**
 * @brief Returns whether a kana vowel extends the previous romanized vowel.
 *
 * This follows the long-vowel spellings represented in the 2025 Cabinet
 * Notification "ローマ字のつづり方".
 *
 * @param previous Previous romanized vowel.
 * @param current Current kana vowel.
 * @return True when current extends previous as a long vowel.
 */
[[nodiscard]] constexpr auto is_romaji_long_vowel_extension(
    const char previous,
    const char current) noexcept -> bool
{
    switch (previous) {
        case 'a':
            return current == 'a';
        case 'i':
            return current == 'i';
        case 'u':
            return current == 'u';
        case 'e':
            return current == 'e' || current == 'i';
        case 'o':
            return current == 'o' || current == 'u';
        default:
            return false;
    }
}

/**
 * @brief Returns the macron-bearing code point for an ASCII vowel.
 *
 * @param value ASCII vowel.
 * @return Lowercase Latin vowel with macron, or NUL if not a supported vowel.
 */
[[nodiscard]] constexpr auto romaji_macron_vowel(
    const char value) noexcept -> char32_t
{
    switch (value) {
        case 'a':
            return U'ā';
        case 'i':
            return U'ī';
        case 'u':
            return U'ū';
        case 'e':
            return U'ē';
        case 'o':
            return U'ō';
        default:
            return U'\0';
    }
}

/**
 * @brief Raw romanized syllable lookup result.
 */
struct romaji_syllable {
    std::string_view text;
    std::size_t consumed = 0;
};

/**
 * @brief Returns a romanized basic mora from ordinary Hiragana.
 *
 * @param value Hiragana code point.
 * @return Romanized text or an empty string when unsupported.
 */
[[nodiscard]] constexpr auto romaji_basic_syllable(
    const char32_t value) noexcept -> std::string_view
{
    switch (value) {
        case U'あ':
            return "a";
        case U'い':
            return "i";
        case U'う':
            return "u";
        case U'え':
            return "e";
        case U'お':
            return "o";
        case U'か':
            return "ka";
        case U'き':
            return "ki";
        case U'く':
            return "ku";
        case U'け':
            return "ke";
        case U'こ':
            return "ko";
        case U'さ':
            return "sa";
        case U'し':
            return "shi";
        case U'す':
            return "su";
        case U'せ':
            return "se";
        case U'そ':
            return "so";
        case U'た':
            return "ta";
        case U'ち':
            return "chi";
        case U'つ':
            return "tsu";
        case U'て':
            return "te";
        case U'と':
            return "to";
        case U'な':
            return "na";
        case U'に':
            return "ni";
        case U'ぬ':
            return "nu";
        case U'ね':
            return "ne";
        case U'の':
            return "no";
        case U'は':
            return "ha";
        case U'ひ':
            return "hi";
        case U'ふ':
            return "fu";
        case U'へ':
            return "he";
        case U'ほ':
            return "ho";
        case U'ま':
            return "ma";
        case U'み':
            return "mi";
        case U'む':
            return "mu";
        case U'め':
            return "me";
        case U'も':
            return "mo";
        case U'や':
            return "ya";
        case U'ゆ':
            return "yu";
        case U'よ':
            return "yo";
        case U'ら':
            return "ra";
        case U'り':
            return "ri";
        case U'る':
            return "ru";
        case U'れ':
            return "re";
        case U'ろ':
            return "ro";
        case U'わ':
            return "wa";
        case U'を':
            return "o";
        case U'が':
            return "ga";
        case U'ぎ':
            return "gi";
        case U'ぐ':
            return "gu";
        case U'げ':
            return "ge";
        case U'ご':
            return "go";
        case U'ざ':
            return "za";
        case U'じ':
            return "ji";
        case U'ず':
            return "zu";
        case U'ぜ':
            return "ze";
        case U'ぞ':
            return "zo";
        case U'だ':
            return "da";
        case U'ぢ':
            return "ji";
        case U'づ':
            return "zu";
        case U'で':
            return "de";
        case U'ど':
            return "do";
        case U'ば':
            return "ba";
        case U'び':
            return "bi";
        case U'ぶ':
            return "bu";
        case U'べ':
            return "be";
        case U'ぼ':
            return "bo";
        case U'ぱ':
            return "pa";
        case U'ぴ':
            return "pi";
        case U'ぷ':
            return "pu";
        case U'ぺ':
            return "pe";
        case U'ぽ':
            return "po";
        case U'ん':
            return "n";
        default:
            return {};
    }
}

/**
 * @brief Returns a romanized yoon mora from ordinary Hiragana.
 *
 * @param base Base Hiragana.
 * @param small_y Small ya, small yu, or small yo.
 * @return Romanized text or an empty string when unsupported.
 */
[[nodiscard]] constexpr auto romaji_yoon_syllable(
    const char32_t base,
    const char32_t small_y) noexcept -> std::string_view
{
    switch (base) {
        case U'き':
            return small_y == U'ゃ' ? "kya" :
                   small_y == U'ゅ' ? "kyu" :
                   small_y == U'ょ' ? "kyo" : std::string_view {};
        case U'し':
            return small_y == U'ゃ' ? "sha" :
                   small_y == U'ゅ' ? "shu" :
                   small_y == U'ょ' ? "sho" : std::string_view {};
        case U'ち':
            return small_y == U'ゃ' ? "cha" :
                   small_y == U'ゅ' ? "chu" :
                   small_y == U'ょ' ? "cho" : std::string_view {};
        case U'に':
            return small_y == U'ゃ' ? "nya" :
                   small_y == U'ゅ' ? "nyu" :
                   small_y == U'ょ' ? "nyo" : std::string_view {};
        case U'ひ':
            return small_y == U'ゃ' ? "hya" :
                   small_y == U'ゅ' ? "hyu" :
                   small_y == U'ょ' ? "hyo" : std::string_view {};
        case U'み':
            return small_y == U'ゃ' ? "mya" :
                   small_y == U'ゅ' ? "myu" :
                   small_y == U'ょ' ? "myo" : std::string_view {};
        case U'り':
            return small_y == U'ゃ' ? "rya" :
                   small_y == U'ゅ' ? "ryu" :
                   small_y == U'ょ' ? "ryo" : std::string_view {};
        case U'ぎ':
            return small_y == U'ゃ' ? "gya" :
                   small_y == U'ゅ' ? "gyu" :
                   small_y == U'ょ' ? "gyo" : std::string_view {};
        case U'じ':
        case U'ぢ':
            return small_y == U'ゃ' ? "ja" :
                   small_y == U'ゅ' ? "ju" :
                   small_y == U'ょ' ? "jo" : std::string_view {};
        case U'び':
            return small_y == U'ゃ' ? "bya" :
                   small_y == U'ゅ' ? "byu" :
                   small_y == U'ょ' ? "byo" : std::string_view {};
        case U'ぴ':
            return small_y == U'ゃ' ? "pya" :
                   small_y == U'ゅ' ? "pyu" :
                   small_y == U'ょ' ? "pyo" : std::string_view {};
        default:
            return {};
    }
}

/**
 * @brief Returns one romanized syllable at the requested decoded index.
 *
 * @param decoded Source code points.
 * @param index Source index.
 * @return Romanized syllable. consumed is zero when unsupported.
 */
[[nodiscard]] inline auto romaji_syllable_at(
    const std::vector<char32_t>& decoded,
    const std::size_t index) noexcept -> romaji_syllable
{
    const char32_t base = normalize_romaji_kana(decoded[index]);

    if (index + 1 < decoded.size()) {
        const char32_t next = normalize_romaji_kana(decoded[index + 1]);
        if (is_romaji_small_y(next)) {
            const std::string_view yoon = romaji_yoon_syllable(base, next);
            if (!yoon.empty()) {
                return romaji_syllable {
                    .text = yoon,
                    .consumed = 2,
                };
            }

            return {};
        }
    }

    const std::string_view basic = romaji_basic_syllable(base);
    if (basic.empty()) {
        return {};
    }

    return romaji_syllable {
        .text = basic,
        .consumed = 1,
    };
}

/**
 * @brief Returns the final vowel in an ASCII romanized syllable.
 *
 * @param text Romanized syllable.
 * @return Lowercase ASCII vowel or NUL if no vowel is present.
 */
[[nodiscard]] constexpr auto romaji_final_vowel(
    const std::string_view text) noexcept -> char
{
    if (text.empty()) {
        return '\0';
    }

    const char value = text.back();
    return value == 'a' || value == 'i' || value == 'u' ||
           value == 'e' || value == 'o'
               ? value
               : '\0';
}

/**
 * @brief Returns whether a syllable begins with a vowel or y.
 *
 * This is used for the apostrophe after syllabic n.
 *
 * @param text Romanized syllable.
 * @return True when an apostrophe is required after n.
 */
[[nodiscard]] constexpr auto romaji_requires_n_separator(
    const std::string_view text) noexcept -> bool
{
    if (text.empty()) {
        return false;
    }

    const char initial = text.front();
    return initial == 'a' || initial == 'i' || initial == 'u' ||
           initial == 'e' || initial == 'o' || initial == 'y';
}

/**
 * @brief Appends ASCII romanized text to a code-point buffer.
 *
 * @param output Destination code-point buffer.
 * @param text ASCII text to append.
 */
inline auto append_romaji_ascii(
    std::vector<char32_t>& output,
    const std::string_view text) -> void
{
    for (const char value : text) {
        output.push_back(static_cast<char32_t>(
            static_cast<unsigned char>(value)));
    }
}

/**
 * @brief Returns the code-point index of the last appended roman vowel.
 *
 * @param output Romanized code-point buffer.
 * @param text Most recently appended ASCII syllable.
 * @return Index of the final vowel in output, or output.size() if absent.
 */
[[nodiscard]] constexpr auto romaji_last_vowel_index(
    const std::vector<char32_t>& output,
    const std::string_view text) noexcept -> std::size_t
{
    return romaji_final_vowel(text) == '\0'
               ? output.size()
               : output.size() - 1;
}

/**
 * @brief Performs kana-to-romaji string conversion.
 *
 * @tparam CharT String character type.
 * @param source Source string.
 * @param id Romanization transformation identifier.
 * @return Romanized string on success.
 */
template<supported_string_character CharT>
[[nodiscard]] inline auto strtoctrans_romaji(
    const std::basic_string_view<CharT> source,
    const ctrans_id id) -> result<std::basic_string<CharT>>
{
    const auto decoded = decode_string_code_points(source);
    if (!decoded.has_value()) {
        return std::unexpected(decoded.error());
    }

    std::vector<char32_t> romanized;
    romanized.reserve(source.size());

    char previous_vowel = '\0';
    std::size_t previous_vowel_index = 0;
    bool has_previous_vowel = false;
    bool previous_was_n = false;

    for (std::size_t index = 0; index < decoded->size();) {
        const char32_t normalized = normalize_romaji_kana((*decoded)[index]);

        if (normalized == U'ー') {
            if (!has_previous_vowel) {
                return std::unexpected(make_error(error_t::invalid_argument));
            }

            if (id == ctrans_id::romaji) {
                const char32_t macron = romaji_macron_vowel(previous_vowel);
                if (macron == U'\0') {
                    return std::unexpected(make_error(error_t::invalid_argument));
                }

                romanized[previous_vowel_index] = macron;
            } else {
                romanized.push_back(static_cast<char32_t>(previous_vowel));
                previous_vowel_index = romanized.size() - 1;
            }

            previous_was_n = false;
            ++index;
            continue;
        }

        if (normalized == U'っ') {
            if (index + 1 >= decoded->size()) {
                return std::unexpected(make_error(error_t::invalid_argument));
            }

            const romaji_syllable next = romaji_syllable_at(*decoded, index + 1);
            if (next.consumed == 0 || next.text.empty()) {
                return std::unexpected(make_error(error_t::invalid_argument));
            }

            const char initial = next.text.front();
            if (initial == 'a' || initial == 'i' || initial == 'u' ||
                initial == 'e' || initial == 'o' || initial == 'n') {
                return std::unexpected(make_error(error_t::invalid_argument));
            }

            romanized.push_back(static_cast<char32_t>(
                static_cast<unsigned char>(initial)));
            previous_was_n = false;
            ++index;
            continue;
        }

        if (id == ctrans_id::romaji && has_previous_vowel) {
            const char current_vowel = romaji_vowel_kana(normalized);
            if (current_vowel != '\0' &&
                is_romaji_long_vowel_extension(
                    previous_vowel,
                    current_vowel)) {
                const char32_t macron = romaji_macron_vowel(previous_vowel);
                if (macron == U'\0') {
                    return std::unexpected(make_error(error_t::invalid_argument));
                }

                romanized[previous_vowel_index] = macron;
                previous_was_n = false;
                ++index;
                continue;
            }
        }

        const romaji_syllable syllable = romaji_syllable_at(*decoded, index);
        if (syllable.consumed == 0 || syllable.text.empty()) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        if (previous_was_n && romaji_requires_n_separator(syllable.text)) {
            romanized.push_back(U'’');
        }

        append_romaji_ascii(romanized, syllable.text);

        const char final_vowel = romaji_final_vowel(syllable.text);
        has_previous_vowel = final_vowel != '\0';
        if (has_previous_vowel) {
            previous_vowel = final_vowel;
            previous_vowel_index = romaji_last_vowel_index(
                romanized,
                syllable.text);
        }

        previous_was_n = syllable.text == "n";
        index += syllable.consumed;
    }

    std::basic_string<CharT> output;
    output.reserve(romanized.size());

    for (const char32_t value : romanized) {
        const auto appended = append_transformed_code_point(output, value);
        if (!appended.has_value()) {
            return std::unexpected(appended.error());
        }
    }

    return output;
}

/**
 * @brief Performs string-level fullwidth/halfwidth conversion.
 *
 * Unlike toctrans(), this function may combine two halfwidth code points into
 * one fullwidth code point or decompose one fullwidth code point into two
 * halfwidth code points.
 *
 * @tparam CharT String character type.
 * @param source Source string.
 * @param id Width transformation identifier.
 * @return Converted string on success.
 */
template<supported_string_character CharT>
[[nodiscard]] inline auto strtoctrans_width(
    const std::basic_string_view<CharT> source,
    const ctrans_id id) -> result<std::basic_string<CharT>>
{
    const auto decoded = decode_string_code_points(source);
    if (!decoded.has_value()) {
        return std::unexpected(decoded.error());
    }

    std::basic_string<CharT> output;
    output.reserve(source.size());

    for (std::size_t index = 0; index < decoded->size(); ++index) {
        const char32_t value = (*decoded)[index];

        if (id == ctrans_id::fullwidth || id == ctrans_id::fullwidth_kana ||
            id == ctrans_id::fullwidth_graph || id == ctrans_id::fullwidth_print) {
            if (index + 1 < decoded->size() &&
                (is_halfwidth_voiced_sound_mark((*decoded)[index + 1]) ||
                 is_halfwidth_semivoiced_sound_mark((*decoded)[index + 1]))) {
                const char32_t composed = compose_halfwidth_kana(
                    value,
                    (*decoded)[index + 1]);
                if (composed != U'\0') {
                    const auto appended = append_transformed_code_point(
                        output,
                        composed);
                    if (!appended.has_value()) {
                        return std::unexpected(appended.error());
                    }

                    ++index;
                    continue;
                }
            }

            const auto converted = xer::toctrans(value, id);
            if (!converted.has_value()) {
                return std::unexpected(converted.error());
            }

            const auto appended = append_transformed_code_point(
                output,
                *converted);
            if (!appended.has_value()) {
                return std::unexpected(appended.error());
            }

            continue;
        }

        if (id == ctrans_id::halfwidth || id == ctrans_id::halfwidth_kana ||
            id == ctrans_id::halfwidth_graph || id == ctrans_id::halfwidth_print) {
            const halfwidth_kana_decomposition decomposed =
                fullwidth_kana_to_halfwidth(value);

            if (decomposed.mark != U'\0' || decomposed.base != value ||
                id == ctrans_id::halfwidth_kana) {
                const auto appended_base = append_transformed_code_point(
                    output,
                    decomposed.base);
                if (!appended_base.has_value()) {
                    return std::unexpected(appended_base.error());
                }

                if (decomposed.mark != U'\0') {
                    const auto appended_mark = append_transformed_code_point(
                        output,
                        decomposed.mark);
                    if (!appended_mark.has_value()) {
                        return std::unexpected(appended_mark.error());
                    }
                }

                continue;
            }

            const auto converted = xer::toctrans(value, id);
            if (!converted.has_value()) {
                return std::unexpected(converted.error());
            }

            const auto appended = append_transformed_code_point(output, *converted);
            if (!appended.has_value()) {
                return std::unexpected(appended.error());
            }

            continue;
        }

        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return output;
}
} // namespace xer::detail

namespace xer {

/**
 * @brief Transforms each character in a string according to a transformation ID.
 *
 * For char and unsigned char strings, each code unit is treated as one byte-sized
 * code point. For UTF-8, UTF-16, and UTF-32 strings, the input is decoded by
 * Unicode code point and encoded back to the same string character type.
 *
 * @tparam CharT String character type.
 * @param source Source string.
 * @param id Transformation identifier.
 * @return Transformed string on success.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] inline auto strtoctrans(
    const std::basic_string_view<CharT> source,
    const ctrans_id id) -> result<std::basic_string<CharT>>
{
    std::basic_string<CharT> output;
    output.reserve(source.size());

    if (id == ctrans_id::romaji || id == ctrans_id::romaji_alt) {
        return detail::strtoctrans_romaji(source, id);
    }

    if (id == ctrans_id::fullwidth || id == ctrans_id::halfwidth ||
        id == ctrans_id::fullwidth_kana || id == ctrans_id::halfwidth_kana ||
        id == ctrans_id::fullwidth_graph || id == ctrans_id::halfwidth_graph ||
        id == ctrans_id::fullwidth_print || id == ctrans_id::halfwidth_print) {
        return detail::strtoctrans_width(source, id);
    }

    if constexpr (std::same_as<CharT, char> ||
                  std::same_as<CharT, unsigned char>) {
        using unsigned_char_type = std::make_unsigned_t<CharT>;

        for (const CharT unit : source) {
            const char32_t value = static_cast<char32_t>(
                static_cast<unsigned_char_type>(unit));
            const auto converted = xer::toctrans(value, id);
            if (!converted.has_value()) {
                return std::unexpected(converted.error());
            }

            const auto appended = detail::append_transformed_code_point(
                output,
                *converted);
            if (!appended.has_value()) {
                return std::unexpected(appended.error());
            }
        }
    } else if constexpr (std::same_as<CharT, char8_t>) {
        for (std::size_t index = 0; index < source.size();) {
            const auto decoded = detail::decode_utf8_at(source, index);
            if (!decoded.has_value()) {
                return std::unexpected(decoded.error());
            }

            const auto converted = xer::toctrans(decoded->value, id);
            if (!converted.has_value()) {
                return std::unexpected(converted.error());
            }

            const auto appended = detail::append_transformed_code_point(
                output,
                *converted);
            if (!appended.has_value()) {
                return std::unexpected(appended.error());
            }

            index += decoded->size;
        }
    } else if constexpr (std::same_as<CharT, char16_t>) {
        for (std::size_t index = 0; index < source.size();) {
            const auto decoded = detail::decode_utf16_at(source, index);
            if (!decoded.has_value()) {
                return std::unexpected(decoded.error());
            }

            const auto converted = xer::toctrans(decoded->value, id);
            if (!converted.has_value()) {
                return std::unexpected(converted.error());
            }

            const auto appended = detail::append_transformed_code_point(
                output,
                *converted);
            if (!appended.has_value()) {
                return std::unexpected(appended.error());
            }

            index += decoded->size;
        }
    } else {
        for (const char32_t value : source) {
            if (!detail::is_unicode_scalar_value(value)) {
                return std::unexpected(make_error(error_t::encoding_error));
            }

            const auto converted = xer::toctrans(value, id);
            if (!converted.has_value()) {
                return std::unexpected(converted.error());
            }

            const auto appended = detail::append_transformed_code_point(
                output,
                *converted);
            if (!appended.has_value()) {
                return std::unexpected(appended.error());
            }
        }
    }

    return output;
}

/**
 * @brief Transforms each character in a pointer-sized string.
 *
 * The supplied size is used directly. A NUL code unit is treated as ordinary
 * input when it is included in the specified range.
 *
 * @tparam CharT String character type.
 * @param source Source pointer.
 * @param size Source size in code units.
 * @param id Transformation identifier.
 * @return Transformed string on success.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] inline auto strtoctrans(
    const CharT* source,
    const std::size_t size,
    const ctrans_id id) -> result<std::basic_string<std::remove_cv_t<CharT>>>
{
    if (source == nullptr && size != 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    using bare_char_t = std::remove_cv_t<CharT>;
    return xer::strtoctrans(
        std::basic_string_view<bare_char_t>(source, size),
        id);
}

/**
 * @brief Transforms each character in an array string.
 *
 * A trailing NUL code unit is excluded when present. Embedded NUL code units are
 * treated as ordinary input.
 *
 * @tparam CharT String character type.
 * @tparam N Source array size.
 * @param source Source array.
 * @param id Transformation identifier.
 * @return Transformed string on success.
 */
template<typename CharT, std::size_t N>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] inline auto strtoctrans(
    const CharT (&source)[N],
    const ctrans_id id) -> result<std::basic_string<std::remove_cv_t<CharT>>>
{
    return xer::strtoctrans(detail::transformation_array_string_view(source), id);
}

/**
 * @brief Converts a string to lowercase using ASCII lowercase conversion.
 *
 * @tparam CharT String character type.
 * @param source Source string.
 * @return Lowercase string on success.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] inline auto strtolower(
    const std::basic_string_view<CharT> source) -> result<std::basic_string<CharT>>
{
    return xer::strtoctrans(source, ctrans_id::lower);
}

/**
 * @brief Converts a pointer-sized string to lowercase.
 *
 * @tparam CharT String character type.
 * @param source Source pointer.
 * @param size Source size in code units.
 * @return Lowercase string on success.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] inline auto strtolower(
    const CharT* source,
    const std::size_t size) -> result<std::basic_string<std::remove_cv_t<CharT>>>
{
    return xer::strtoctrans(source, size, ctrans_id::lower);
}

/**
 * @brief Converts an array string to lowercase.
 *
 * @tparam CharT String character type.
 * @tparam N Source array size.
 * @param source Source array.
 * @return Lowercase string on success.
 */
template<typename CharT, std::size_t N>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] inline auto strtolower(
    const CharT (&source)[N]) -> result<std::basic_string<std::remove_cv_t<CharT>>>
{
    return xer::strtoctrans(source, ctrans_id::lower);
}

/**
 * @brief Converts a string to uppercase using ASCII uppercase conversion.
 *
 * @tparam CharT String character type.
 * @param source Source string.
 * @return Uppercase string on success.
 */
template<detail::supported_string_character CharT>
[[nodiscard]] inline auto strtoupper(
    const std::basic_string_view<CharT> source) -> result<std::basic_string<CharT>>
{
    return xer::strtoctrans(source, ctrans_id::upper);
}

/**
 * @brief Converts a pointer-sized string to uppercase.
 *
 * @tparam CharT String character type.
 * @param source Source pointer.
 * @param size Source size in code units.
 * @return Uppercase string on success.
 */
template<typename CharT>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] inline auto strtoupper(
    const CharT* source,
    const std::size_t size) -> result<std::basic_string<std::remove_cv_t<CharT>>>
{
    return xer::strtoctrans(source, size, ctrans_id::upper);
}

/**
 * @brief Converts an array string to uppercase.
 *
 * @tparam CharT String character type.
 * @tparam N Source array size.
 * @param source Source array.
 * @return Uppercase string on success.
 */
template<typename CharT, std::size_t N>
    requires detail::supported_string_character<std::remove_cv_t<CharT>>
[[nodiscard]] inline auto strtoupper(
    const CharT (&source)[N]) -> result<std::basic_string<std::remove_cv_t<CharT>>>
{
    return xer::strtoctrans(source, ctrans_id::upper);
}

} // namespace xer

#endif /* XER_BITS_STRING_CASE_H_INCLUDED_ */

/**
 * @file xer/bits/advanced_encoding.h
 * @brief Low-level packed encoding conversion functions.
 */

#pragma once

#ifndef XER_BITS_ADVANCED_ENCODING_H_INCLUDED_
#define XER_BITS_ADVANCED_ENCODING_H_INCLUDED_

#include <cstdint>

#include <xer/bits/common.h>
#include <xer/bits/packed_cp932_tables.h>

namespace xer::advanced {

namespace detail {

/**
 * @brief Invalid value for packed UTF-8.
 */
inline constexpr std::uint32_t invalid_packed_utf8 = static_cast<std::uint32_t>(-1);

/**
 * @brief Invalid value for packed UTF-16.
 */
inline constexpr std::uint32_t invalid_packed_utf16 = static_cast<std::uint32_t>(-1);

/**
 * @brief Invalid value for UTF-32 code point.
 */
inline constexpr char32_t invalid_utf32 = static_cast<char32_t>(-1);

/**
 * @brief Invalid value for packed CP932.
 */
inline constexpr std::int32_t invalid_packed_cp932 = -1;

/**
 * @brief Tests whether the value is a Unicode surrogate code point.
 * @param code_point Code point to test.
 * @return `true` if the value is in U+D800..U+DFFF.
 */
[[nodiscard]] inline constexpr bool is_surrogate(char32_t code_point)
{
    return code_point >= static_cast<char32_t>(0xD800u) &&
           code_point <= static_cast<char32_t>(0xDFFFu);
}

/**
 * @brief Tests whether the byte is a UTF-8 continuation byte.
 * @param byte Byte to test.
 * @return `true` if the byte is in 0x80..0xBF.
 */
[[nodiscard]] inline constexpr bool is_utf8_continuation(std::uint8_t byte)
{
    return byte >= 0x80u && byte <= 0xBFu;
}

/**
 * @brief Tests whether the packed CP932 value is a valid single-byte code unit.
 * @param packed Packed CP932 value.
 * @return `true` if the value is a valid single-byte CP932 code unit.
 */
[[nodiscard]] inline constexpr bool is_cp932_single_byte(std::uint16_t packed)
{
    return packed <= 0x007Fu || (packed >= 0x00A1u && packed <= 0x00DFu);
}

/**
 * @brief Tests whether the byte is a valid CP932 lead byte.
 * @param byte Byte to test.
 * @return `true` if the byte is a valid CP932 lead byte.
 */
[[nodiscard]] inline constexpr bool is_cp932_lead_byte(std::uint8_t byte)
{
    return (byte >= 0x81u && byte <= 0x9Fu) || (byte >= 0xE0u && byte <= 0xFCu);
}

/**
 * @brief Tests whether the byte is a valid CP932 trail byte.
 * @param byte Byte to test.
 * @return `true` if the byte is a valid CP932 trail byte.
 */
[[nodiscard]] inline constexpr bool is_cp932_trail_byte(std::uint8_t byte)
{
    return (byte >= 0x40u && byte <= 0x7Eu) || (byte >= 0x80u && byte <= 0xFCu);
}

/**
 * @brief Converts a BMP code point to packed UTF-16.
 * @param code_point BMP code point.
 * @return Packed UTF-16, or invalid value on failure.
 */
[[nodiscard]] inline constexpr std::uint32_t bmp_to_packed_utf16(char32_t code_point)
{
    if (code_point > 0xFFFFu || is_surrogate(code_point)) {
        return invalid_packed_utf16;
    }

    return static_cast<std::uint32_t>(code_point);
}

/**
 * @brief Converts a UTF-16 code unit to UTF-32.
 * @param code_unit UTF-16 code unit.
 * @return UTF-32 code point, or invalid value on failure.
 */
[[nodiscard]] inline constexpr char32_t utf16_unit_to_utf32(char16_t code_unit)
{
    if (code_unit >= 0xD800u && code_unit <= 0xDFFFu) {
        return invalid_utf32;
    }

    return static_cast<char32_t>(code_unit);
}

} // namespace detail

/**
 * @brief Converts a UTF-32 code point to packed UTF-8.
 * @param code_point UTF-32 code point.
 * @return Packed UTF-8, or `static_cast<std::uint32_t>(-1)` on failure.
 */
[[nodiscard]] inline constexpr std::uint32_t utf32_to_packed_utf8(char32_t code_point)
{
    if (code_point > 0x10FFFFu || detail::is_surrogate(code_point)) {
        return detail::invalid_packed_utf8;
    }

    if (code_point <= 0x7Fu) {
        return static_cast<std::uint32_t>(code_point);
    }

    if (code_point <= 0x7FFu) {
        const std::uint32_t b1 =
            0xC0u | (static_cast<std::uint32_t>(code_point) >> 6);
        const std::uint32_t b2 =
            0x80u | (static_cast<std::uint32_t>(code_point) & 0x3Fu);
        return b1 | (b2 << 8);
    }

    if (code_point <= 0xFFFFu) {
        const std::uint32_t b1 =
            0xE0u | (static_cast<std::uint32_t>(code_point) >> 12);
        const std::uint32_t b2 =
            0x80u | ((static_cast<std::uint32_t>(code_point) >> 6) & 0x3Fu);
        const std::uint32_t b3 =
            0x80u | (static_cast<std::uint32_t>(code_point) & 0x3Fu);
        return b1 | (b2 << 8) | (b3 << 16);
    }

    const std::uint32_t b1 =
        0xF0u | (static_cast<std::uint32_t>(code_point) >> 18);
    const std::uint32_t b2 =
        0x80u | ((static_cast<std::uint32_t>(code_point) >> 12) & 0x3Fu);
    const std::uint32_t b3 =
        0x80u | ((static_cast<std::uint32_t>(code_point) >> 6) & 0x3Fu);
    const std::uint32_t b4 =
        0x80u | (static_cast<std::uint32_t>(code_point) & 0x3Fu);
    return b1 | (b2 << 8) | (b3 << 16) | (b4 << 24);
}

/**
 * @brief Converts packed UTF-8 to a UTF-32 code point.
 * @param packed Packed UTF-8.
 * @return UTF-32 code point, or `static_cast<char32_t>(-1)` on failure.
 */
[[nodiscard]] inline constexpr char32_t packed_utf8_to_utf32(std::uint32_t packed)
{
    const std::uint8_t b1 = static_cast<std::uint8_t>(packed & 0xFFu);
    const std::uint8_t b2 = static_cast<std::uint8_t>((packed >> 8) & 0xFFu);
    const std::uint8_t b3 = static_cast<std::uint8_t>((packed >> 16) & 0xFFu);
    const std::uint8_t b4 = static_cast<std::uint8_t>((packed >> 24) & 0xFFu);

    if (b1 <= 0x7Fu) {
        if (b2 != 0 || b3 != 0 || b4 != 0) {
            return detail::invalid_utf32;
        }

        return static_cast<char32_t>(b1);
    }

    if (b1 >= 0xC2u && b1 <= 0xDFu) {
        if (!detail::is_utf8_continuation(b2) || b3 != 0 || b4 != 0) {
            return detail::invalid_utf32;
        }

        return static_cast<char32_t>(((b1 & 0x1Fu) << 6) | (b2 & 0x3Fu));
    }

    if (b1 >= 0xE0u && b1 <= 0xEFu) {
        if (!detail::is_utf8_continuation(b2) ||
            !detail::is_utf8_continuation(b3) ||
            b4 != 0) {
            return detail::invalid_utf32;
        }

        if (b1 == 0xE0u && b2 < 0xA0u) {
            return detail::invalid_utf32;
        }

        if (b1 == 0xEDu && b2 >= 0xA0u) {
            return detail::invalid_utf32;
        }

        const char32_t code_point =
            static_cast<char32_t>(((b1 & 0x0Fu) << 12) |
                                  ((b2 & 0x3Fu) << 6) |
                                  (b3 & 0x3Fu));

        if (detail::is_surrogate(code_point)) {
            return detail::invalid_utf32;
        }

        return code_point;
    }

    if (b1 >= 0xF0u && b1 <= 0xF4u) {
        if (!detail::is_utf8_continuation(b2) ||
            !detail::is_utf8_continuation(b3) ||
            !detail::is_utf8_continuation(b4)) {
            return detail::invalid_utf32;
        }

        if (b1 == 0xF0u && b2 < 0x90u) {
            return detail::invalid_utf32;
        }

        if (b1 == 0xF4u && b2 > 0x8Fu) {
            return detail::invalid_utf32;
        }

        const char32_t code_point =
            static_cast<char32_t>(((b1 & 0x07u) << 18) |
                                  ((b2 & 0x3Fu) << 12) |
                                  ((b3 & 0x3Fu) << 6) |
                                  (b4 & 0x3Fu));

        if (code_point > 0x10FFFFu) {
            return detail::invalid_utf32;
        }

        return code_point;
    }

    return detail::invalid_utf32;
}

/**
 * @brief Converts a UTF-32 code point to packed UTF-16.
 * @param code_point UTF-32 code point.
 * @return Packed UTF-16, or `static_cast<std::uint32_t>(-1)` on failure.
 */
[[nodiscard]] inline constexpr std::uint32_t utf32_to_packed_utf16(char32_t code_point)
{
    if (code_point > 0x10FFFFu || detail::is_surrogate(code_point)) {
        return detail::invalid_packed_utf16;
    }

    if (code_point <= 0xFFFFu) {
        return static_cast<std::uint32_t>(code_point);
    }

    const std::uint32_t value = static_cast<std::uint32_t>(code_point) - 0x10000u;
    const std::uint16_t high =
        static_cast<std::uint16_t>(0xD800u | (value >> 10));
    const std::uint16_t low =
        static_cast<std::uint16_t>(0xDC00u | (value & 0x3FFu));

    return static_cast<std::uint32_t>(high) |
           (static_cast<std::uint32_t>(low) << 16);
}

/**
 * @brief Converts packed UTF-16 to a UTF-32 code point.
 * @param packed Packed UTF-16.
 * @return UTF-32 code point, or `static_cast<char32_t>(-1)` on failure.
 */
[[nodiscard]] inline constexpr char32_t packed_utf16_to_utf32(std::uint32_t packed)
{
    const std::uint16_t w1 = static_cast<std::uint16_t>(packed & 0xFFFFu);
    const std::uint16_t w2 = static_cast<std::uint16_t>((packed >> 16) & 0xFFFFu);

    if (w1 == 0) {
        return U'\0';
    }

    if (w1 < 0xD800u || w1 > 0xDFFFu) {
        if (w2 != 0) {
            return detail::invalid_utf32;
        }

        return static_cast<char32_t>(w1);
    }

    if (w1 < 0xD800u || w1 > 0xDBFFu) {
        return detail::invalid_utf32;
    }

    if (w2 < 0xDC00u || w2 > 0xDFFFu) {
        return detail::invalid_utf32;
    }

    const std::uint32_t high = static_cast<std::uint32_t>(w1) - 0xD800u;
    const std::uint32_t low = static_cast<std::uint32_t>(w2) - 0xDC00u;
    return static_cast<char32_t>(0x10000u + ((high << 10) | low));
}

/**
 * @brief Converts a UTF-32 code point to packed CP932.
 * @param code_point UTF-32 code point.
 * @return Packed CP932, or `-1` on failure.
 */
[[nodiscard]] inline constexpr std::int32_t utf32_to_packed_cp932(char32_t code_point)
{
    if (code_point > 0xFFFFu || detail::is_surrogate(code_point)) {
        return detail::invalid_packed_cp932;
    }

    return static_cast<std::int32_t>(
        xer::detail::utf16_to_packed_cp932_table[static_cast<std::uint16_t>(code_point)]);
}

/**
 * @brief Converts packed CP932 to a UTF-32 code point.
 * @param packed Packed CP932.
 * @return UTF-32 code point, or `static_cast<char32_t>(-1)` on failure.
 */
[[nodiscard]] inline constexpr char32_t packed_cp932_to_utf32(std::uint16_t packed)
{
    if (detail::is_cp932_single_byte(packed)) {
        const char16_t u16 = xer::detail::packed_cp932_to_utf16_table[packed];
        if (u16 == static_cast<char16_t>(0xFFFFu)) {
            return detail::invalid_utf32;
        }

        return static_cast<char32_t>(u16);
    }

    const std::uint8_t lead = static_cast<std::uint8_t>(packed & 0xFFu);
    const std::uint8_t trail = static_cast<std::uint8_t>((packed >> 8) & 0xFFu);

    if (!detail::is_cp932_lead_byte(lead) || !detail::is_cp932_trail_byte(trail)) {
        return detail::invalid_utf32;
    }

    const char16_t u16 = xer::detail::packed_cp932_to_utf16_table[packed];
    if (u16 == static_cast<char16_t>(0xFFFFu)) {
        return detail::invalid_utf32;
    }

    return static_cast<char32_t>(u16);
}

static_assert(utf32_to_packed_utf8(U'A') == 0x00000041u);
static_assert(utf32_to_packed_utf8(U'\u00A2') == 0x0000A2C2u);
static_assert(utf32_to_packed_utf8(U'\u3042') == 0x008281E3u);
static_assert(utf32_to_packed_utf8(U'\U0001F600') == 0x80989FF0u);

static_assert(packed_utf8_to_utf32(0x00000041u) == U'A');
static_assert(packed_utf8_to_utf32(0x0000A2C2u) == U'\u00A2');
static_assert(packed_utf8_to_utf32(0x008281E3u) == U'\u3042');
static_assert(packed_utf8_to_utf32(0x80989FF0u) == U'\U0001F600');

static_assert(utf32_to_packed_utf16(U'\0') == 0x00000000u);
static_assert(utf32_to_packed_utf16(U'A') == 0x00000041u);
static_assert(utf32_to_packed_utf16(U'\u3042') == 0x00003042u);
static_assert(utf32_to_packed_utf16(U'\U0001F600') == 0xDE00D83Du);

static_assert(packed_utf16_to_utf32(0x00000000u) == U'\0');
static_assert(packed_utf16_to_utf32(0x00000041u) == U'A');
static_assert(packed_utf16_to_utf32(0x00003042u) == U'\u3042');
static_assert(packed_utf16_to_utf32(0xDE00D83Du) == U'\U0001F600');

static_assert(
    utf32_to_packed_utf8(static_cast<char32_t>(0x110000)) ==
    static_cast<std::uint32_t>(-1));

static_assert(
    utf32_to_packed_utf16(static_cast<char32_t>(0x110000)) ==
    static_cast<std::uint32_t>(-1));

static_assert(
    packed_utf8_to_utf32(0x000080C0u) ==
    static_cast<char32_t>(-1));

static_assert(
    packed_utf16_to_utf32(0x0000D800u) ==
    static_cast<char32_t>(-1));

} // namespace xer::advanced

#endif /* XER_BITS_ADVANCED_ENCODING_H_INCLUDED_ */

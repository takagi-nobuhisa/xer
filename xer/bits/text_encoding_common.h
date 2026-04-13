/**
 * @file xer/bits/text_encoding_common.h
 * @brief Internal common text encoding helpers.
 */

#pragma once

#ifndef XER_BITS_TEXT_ENCODING_COMMON_H_INCLUDED_
#define XER_BITS_TEXT_ENCODING_COMMON_H_INCLUDED_

#include <expected>
#include <string>
#include <string_view>

#include <xer/bits/advanced_encoding.h>
#include <xer/bits/common.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Converts a UTF-8 string view to a byte string without reinterpretation.
 *
 * @param value Source UTF-8 string.
 * @return Converted byte string.
 */
[[nodiscard]] inline auto to_byte_string(std::u8string_view value) -> std::string
{
    return std::string(
        reinterpret_cast<const char*>(value.data()),
        reinterpret_cast<const char*>(value.data() + value.size()));
}

/**
 * @brief Converts a byte string view to a UTF-8 string without reinterpretation.
 *
 * @param value Source byte string.
 * @return Converted UTF-8 string.
 */
[[nodiscard]] inline auto to_u8string(std::string_view value) -> std::u8string
{
    std::u8string result;
    result.reserve(value.size());

    for (unsigned char ch : value) {
        result.push_back(static_cast<char8_t>(ch));
    }

    return result;
}

/**
 * @brief Returns whether the specified byte string is valid UTF-8.
 *
 * @param value Source byte string.
 * @return true if the input is valid UTF-8.
 * @return false otherwise.
 */
[[nodiscard]] inline auto is_valid_utf8(std::string_view value) noexcept -> bool
{
    std::size_t i = 0;

    while (i < value.size()) {
        const unsigned char c0 = static_cast<unsigned char>(value[i]);

        if (c0 <= 0x7f) {
            ++i;
            continue;
        }

        if (c0 >= 0xc2 && c0 <= 0xdf) {
            if (i + 1 >= value.size()) {
                return false;
            }

            const unsigned char c1 = static_cast<unsigned char>(value[i + 1]);
            if ((c1 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 2;
            continue;
        }

        if (c0 == 0xe0) {
            if (i + 2 >= value.size()) {
                return false;
            }

            const unsigned char c1 = static_cast<unsigned char>(value[i + 1]);
            const unsigned char c2 = static_cast<unsigned char>(value[i + 2]);

            if (c1 < 0xa0 || c1 > 0xbf || (c2 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 3;
            continue;
        }

        if (c0 >= 0xe1 && c0 <= 0xec) {
            if (i + 2 >= value.size()) {
                return false;
            }

            const unsigned char c1 = static_cast<unsigned char>(value[i + 1]);
            const unsigned char c2 = static_cast<unsigned char>(value[i + 2]);

            if ((c1 & 0xc0u) != 0x80u || (c2 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 3;
            continue;
        }

        if (c0 == 0xed) {
            if (i + 2 >= value.size()) {
                return false;
            }

            const unsigned char c1 = static_cast<unsigned char>(value[i + 1]);
            const unsigned char c2 = static_cast<unsigned char>(value[i + 2]);

            if (c1 < 0x80 || c1 > 0x9f || (c2 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 3;
            continue;
        }

        if (c0 >= 0xee && c0 <= 0xef) {
            if (i + 2 >= value.size()) {
                return false;
            }

            const unsigned char c1 = static_cast<unsigned char>(value[i + 1]);
            const unsigned char c2 = static_cast<unsigned char>(value[i + 2]);

            if ((c1 & 0xc0u) != 0x80u || (c2 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 3;
            continue;
        }

        if (c0 == 0xf0) {
            if (i + 3 >= value.size()) {
                return false;
            }

            const unsigned char c1 = static_cast<unsigned char>(value[i + 1]);
            const unsigned char c2 = static_cast<unsigned char>(value[i + 2]);
            const unsigned char c3 = static_cast<unsigned char>(value[i + 3]);

            if (c1 < 0x90 || c1 > 0xbf || (c2 & 0xc0u) != 0x80u ||
                (c3 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 4;
            continue;
        }

        if (c0 >= 0xf1 && c0 <= 0xf3) {
            if (i + 3 >= value.size()) {
                return false;
            }

            const unsigned char c1 = static_cast<unsigned char>(value[i + 1]);
            const unsigned char c2 = static_cast<unsigned char>(value[i + 2]);
            const unsigned char c3 = static_cast<unsigned char>(value[i + 3]);

            if ((c1 & 0xc0u) != 0x80u || (c2 & 0xc0u) != 0x80u ||
                (c3 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 4;
            continue;
        }

        if (c0 == 0xf4) {
            if (i + 3 >= value.size()) {
                return false;
            }

            const unsigned char c1 = static_cast<unsigned char>(value[i + 1]);
            const unsigned char c2 = static_cast<unsigned char>(value[i + 2]);
            const unsigned char c3 = static_cast<unsigned char>(value[i + 3]);

            if (c1 < 0x80 || c1 > 0x8f || (c2 & 0xc0u) != 0x80u ||
                (c3 & 0xc0u) != 0x80u) {
                return false;
            }

            i += 4;
            continue;
        }

        return false;
    }

    return true;
}

#ifdef _WIN32

/**
 * @brief Decodes one UTF-8 code point from a byte string.
 *
 * @param value Source byte string.
 * @param index Current read position.
 * @return Decoded code point and advances @p index on success.
 */
[[nodiscard]] inline auto decode_utf8_code_point(
    std::string_view value,
    std::size_t& index) -> result<char32_t>
{
    if (index >= value.size()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const unsigned char c0 = static_cast<unsigned char>(value[index]);
    std::uint32_t packed = 0;
    std::size_t length = 0;

    if (c0 <= 0x7fu) {
        packed = static_cast<std::uint32_t>(c0);
        length = 1;
    } else if (c0 >= 0xc2u && c0 <= 0xdfu) {
        if (index + 1 >= value.size()) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        const unsigned char c1 = static_cast<unsigned char>(value[index + 1]);
        packed = static_cast<std::uint32_t>(c0) |
                 (static_cast<std::uint32_t>(c1) << 8);
        length = 2;
    } else if (c0 >= 0xe0u && c0 <= 0xefu) {
        if (index + 2 >= value.size()) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        const unsigned char c1 = static_cast<unsigned char>(value[index + 1]);
        const unsigned char c2 = static_cast<unsigned char>(value[index + 2]);
        packed = static_cast<std::uint32_t>(c0) |
                 (static_cast<std::uint32_t>(c1) << 8) |
                 (static_cast<std::uint32_t>(c2) << 16);
        length = 3;
    } else if (c0 >= 0xf0u && c0 <= 0xf4u) {
        if (index + 3 >= value.size()) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        const unsigned char c1 = static_cast<unsigned char>(value[index + 1]);
        const unsigned char c2 = static_cast<unsigned char>(value[index + 2]);
        const unsigned char c3 = static_cast<unsigned char>(value[index + 3]);
        packed = static_cast<std::uint32_t>(c0) |
                 (static_cast<std::uint32_t>(c1) << 8) |
                 (static_cast<std::uint32_t>(c2) << 16) |
                 (static_cast<std::uint32_t>(c3) << 24);
        length = 4;
    } else {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    const char32_t code_point = advanced::packed_utf8_to_utf32(packed);
    if (code_point == advanced::detail::invalid_utf32) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    index += length;
    return code_point;
}

/**
 * @brief Appends packed UTF-8 bytes to a UTF-8 string.
 *
 * @param out Destination string.
 * @param packed Packed UTF-8 value.
 */
inline void append_packed_utf8(std::u8string& out, std::uint32_t packed)
{
    const unsigned char b1 = static_cast<unsigned char>(packed & 0xffu);
    const unsigned char b2 = static_cast<unsigned char>((packed >> 8) & 0xffu);
    const unsigned char b3 = static_cast<unsigned char>((packed >> 16) & 0xffu);
    const unsigned char b4 = static_cast<unsigned char>((packed >> 24) & 0xffu);

    out.push_back(static_cast<char8_t>(b1));

    if (b2 != 0) {
        out.push_back(static_cast<char8_t>(b2));
    }

    if (b3 != 0) {
        out.push_back(static_cast<char8_t>(b3));
    }

    if (b4 != 0) {
        out.push_back(static_cast<char8_t>(b4));
    }
}

/**
 * @brief Converts a UTF-8 byte string to a wide string.
 *
 * @param value Source UTF-8 byte string.
 * @return Converted wide string on success.
 */
[[nodiscard]] inline auto utf8_to_wstring(
    std::string_view value) -> result<std::wstring>
{
    static_assert(sizeof(wchar_t) == 2, "Windows wchar_t must be UTF-16");

    if (value.empty()) {
        return std::wstring();
    }

    std::wstring result;
    result.reserve(value.size());

    std::size_t index = 0;
    while (index < value.size()) {
        const auto code_point_result = decode_utf8_code_point(value, index);
        if (!code_point_result.has_value()) {
            return std::unexpected(code_point_result.error());
        }

        const std::uint32_t packed =
            advanced::utf32_to_packed_utf16(*code_point_result);
        if (packed == advanced::detail::invalid_packed_utf16) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        const char16_t u0 = static_cast<char16_t>(packed & 0xffffu);
        const char16_t u1 = static_cast<char16_t>((packed >> 16) & 0xffffu);

        result.push_back(static_cast<wchar_t>(u0));
        if (u1 != 0) {
            result.push_back(static_cast<wchar_t>(u1));
        }
    }

    return result;
}

/**
 * @brief Converts a wide string to a UTF-8 string.
 *
 * @param value Source wide string.
 * @return Converted UTF-8 string on success.
 */
[[nodiscard]] inline auto wstring_to_utf8(
    std::wstring_view value) -> result<std::u8string>
{
    static_assert(sizeof(wchar_t) == 2, "Windows wchar_t must be UTF-16");

    if (value.empty()) {
        return std::u8string();
    }

    std::u8string result;
    result.reserve(value.size());

    std::size_t index = 0;
    while (index < value.size()) {
        const char16_t u0 = static_cast<char16_t>(value[index]);
        std::uint32_t packed = static_cast<std::uint32_t>(u0);
        ++index;

        if (u0 >= 0xd800u && u0 <= 0xdbffu) {
            if (index >= value.size()) {
                return std::unexpected(make_error(error_t::encoding_error));
            }

            const char16_t u1 = static_cast<char16_t>(value[index]);
            ++index;
            packed |= static_cast<std::uint32_t>(u1) << 16;
        }

        const char32_t code_point = advanced::packed_utf16_to_utf32(packed);
        if (code_point == advanced::detail::invalid_utf32) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        const std::uint32_t utf8_packed =
            advanced::utf32_to_packed_utf8(code_point);
        if (utf8_packed == advanced::detail::invalid_packed_utf8) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        append_packed_utf8(result, utf8_packed);
    }

    return result;
}

#endif

} // namespace xer::detail

#endif /* XER_BITS_TEXT_ENCODING_COMMON_H_INCLUDED_ */

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

#include <xer/bits/common.h>
#include <xer/error.h>

#ifdef _WIN32
#    include <windows.h>
#endif

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
 * @brief Converts a UTF-8 byte string to a wide string.
 *
 * @param value Source UTF-8 byte string.
 * @return Converted wide string on success.
 */
[[nodiscard]] inline auto utf8_to_wstring(
    std::string_view value) -> result<std::wstring>
{
    if (value.empty()) {
        return std::wstring();
    }

    const int required = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        value.data(),
        static_cast<int>(value.size()),
        nullptr,
        0);

    if (required <= 0) {
        return std::unexpected(make_error(error_t::ilseq));
    }

    std::wstring result(static_cast<std::size_t>(required), L'\0');

    const int written = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        value.data(),
        static_cast<int>(value.size()),
        result.data(),
        required);

    if (written != required) {
        return std::unexpected(make_error(error_t::ilseq));
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
    if (value.empty()) {
        return std::u8string();
    }

    const int required = WideCharToMultiByte(
        CP_UTF8,
        WC_ERR_INVALID_CHARS,
        value.data(),
        static_cast<int>(value.size()),
        nullptr,
        0,
        nullptr,
        nullptr);

    if (required <= 0) {
        return std::unexpected(make_error(error_t::ilseq));
    }

    std::string buffer(static_cast<std::size_t>(required), '\0');

    const int written = WideCharToMultiByte(
        CP_UTF8,
        WC_ERR_INVALID_CHARS,
        value.data(),
        static_cast<int>(value.size()),
        buffer.data(),
        required,
        nullptr,
        nullptr);

    if (written != required) {
        return std::unexpected(make_error(error_t::ilseq));
    }

    return to_u8string(buffer);
}

#endif

} // namespace xer::detail

#endif /* XER_BITS_TEXT_ENCODING_COMMON_H_INCLUDED_ */

/**
 * @file xer/bits/printf.h
 * @brief Internal printf-style function implementations.
 */

#pragma once

#ifndef XER_BITS_PRINTF_H_INCLUDED_
#define XER_BITS_PRINTF_H_INCLUDED_

#include <cstddef>
#include <expected>
#include <string_view>
#include <utility>

#include <xer/bits/common.h>
#include <xer/bits/printf_format.h>
#include <xer/bits/text_stream.h>
#include <xer/error.h>

#if __has_include(<xer/bits/standard_streams.h>)
#    include <xer/bits/standard_streams.h>
#endif

namespace xer::detail {

/**
 * @brief Decodes one UTF-8 code point from the specified offset.
 *
 * @param text Source UTF-8 text.
 * @param index Current byte offset. Advanced on success.
 * @return Decoded code point on success.
 */
[[nodiscard]] inline result<char32_t> decode_one_utf8(
    std::u8string_view text,
    std::size_t& index)
{
    if (index >= text.size()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const unsigned char c0 = static_cast<unsigned char>(text[index]);

    if (c0 <= 0x7f) {
        ++index;
        return static_cast<char32_t>(c0);
    }

    if (c0 >= 0xc2 && c0 <= 0xdf) {
        if (index + 1 >= text.size()) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        const unsigned char c1 = static_cast<unsigned char>(text[index + 1]);
        if ((c1 & 0xc0u) != 0x80u) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        index += 2;
        return static_cast<char32_t>(((c0 & 0x1fu) << 6) | (c1 & 0x3fu));
    }

    if (c0 >= 0xe0 && c0 <= 0xef) {
        if (index + 2 >= text.size()) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        const unsigned char c1 = static_cast<unsigned char>(text[index + 1]);
        const unsigned char c2 = static_cast<unsigned char>(text[index + 2]);

        if ((c1 & 0xc0u) != 0x80u || (c2 & 0xc0u) != 0x80u) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        const char32_t value =
            (static_cast<char32_t>(c0 & 0x0fu) << 12) |
            (static_cast<char32_t>(c1 & 0x3fu) << 6) |
            static_cast<char32_t>(c2 & 0x3fu);

        if (value < 0x800u || (value >= 0xd800u && value <= 0xdfffu)) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        index += 3;
        return value;
    }

    if (c0 >= 0xf0 && c0 <= 0xf4) {
        if (index + 3 >= text.size()) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        const unsigned char c1 = static_cast<unsigned char>(text[index + 1]);
        const unsigned char c2 = static_cast<unsigned char>(text[index + 2]);
        const unsigned char c3 = static_cast<unsigned char>(text[index + 3]);

        if ((c1 & 0xc0u) != 0x80u || (c2 & 0xc0u) != 0x80u ||
            (c3 & 0xc0u) != 0x80u) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        const char32_t value =
            (static_cast<char32_t>(c0 & 0x07u) << 18) |
            (static_cast<char32_t>(c1 & 0x3fu) << 12) |
            (static_cast<char32_t>(c2 & 0x3fu) << 6) |
            static_cast<char32_t>(c3 & 0x3fu);

        if (value < 0x10000u || value > 0x10ffffu) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        index += 4;
        return value;
    }

    return std::unexpected(make_error(error_t::encoding_error));
}

/**
 * @brief Writes a UTF-8 string to a text stream.
 *
 * @param stream Destination stream.
 * @param text Source UTF-8 text.
 * @return Written byte count on success.
 */
[[nodiscard]] inline result<std::size_t> write_u8_to_text_stream(
    text_stream& stream,
    std::u8string_view text)
{
    if (!stream.has_value()) {
        return std::unexpected(make_error(error_t::io_error));
    }

    std::size_t byte_count = 0;
    std::size_t index = 0;

    while (index < text.size()) {
        const std::size_t before = index;

        auto decoded = decode_one_utf8(text, index);
        if (!decoded.has_value()) {
            stream.set_error(true);
            return std::unexpected(decoded.error());
        }

        const int written = stream.write_fn()(stream.handle(), &*decoded, 1);
        if (written <= 0) {
            stream.set_error(true);
            return std::unexpected(make_error(error_t::io_error));
        }

        byte_count += (index - before);
    }

    return byte_count;
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Writes formatted text to a text stream.
 *
 * @tparam Args Argument types.
 * @param stream Destination stream.
 * @param format UTF-8 format string.
 * @param args Format arguments.
 * @return Written byte count on success.
 */
template<typename... Args>
[[nodiscard]] inline result<std::size_t> fprintf(
    text_stream& stream,
    std::u8string_view format,
    Args&&... args)
{
    auto formatted = detail::format_printf(format, std::forward<Args>(args)...);
    if (!formatted.has_value()) {
        return std::unexpected(formatted.error());
    }

    return detail::write_u8_to_text_stream(stream, *formatted);
}

/**
 * @brief Writes formatted text to a UTF-8 string.
 *
 * @tparam Args Argument types.
 * @param out Destination string.
 * @param format UTF-8 format string.
 * @param args Format arguments.
 * @return Written byte count on success.
 */
template<typename... Args>
[[nodiscard]] inline result<std::size_t> sprintf(
    std::u8string& out,
    std::u8string_view format,
    Args&&... args)
{
    auto formatted = detail::format_printf(format, std::forward<Args>(args)...);
    if (!formatted.has_value()) {
        return std::unexpected(formatted.error());
    }

    out = std::move(*formatted);
    return out.size();
}

/**
 * @brief Writes formatted text to the standard output stream.
 *
 * @tparam Args Argument types.
 * @param format UTF-8 format string.
 * @param args Format arguments.
 * @return Written byte count on success.
 */
template<typename... Args>
[[nodiscard]] inline result<std::size_t> printf(
    std::u8string_view format,
    Args&&... args)
{
#if __has_include(<xer/bits/standard_streams.h>)
    return fprintf(standard_output, format, std::forward<Args>(args)...);
#else
    (void)format;
    (void)sizeof...(args);
    return std::unexpected(make_error(error_t::nosys));
#endif
}

} // namespace xer

#endif /* XER_BITS_PRINTF_H_INCLUDED_ */

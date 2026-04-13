/**
 * @file xer/bits/text_stream_io.h
 * @brief Text stream character and string I/O functions.
 */

#pragma once

#ifndef XER_BITS_TEXT_STREAM_IO_H_INCLUDED_
#define XER_BITS_TEXT_STREAM_IO_H_INCLUDED_

#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <string>
#include <string_view>

#include <xer/bits/advanced_encoding.h>
#include <xer/bits/standard_streams.h>
#include <xer/bits/text_stream.h>
#include <xer/bits/utf8_char_encode.h>
#include <xer/error.h>

namespace xer {

namespace detail {

/**
 * @brief Decodes one UTF-8 code point from the specified offset.
 *
 * @param text Source UTF-8 text.
 * @param offset Current byte offset. Advanced on success.
 * @return Decoded code point on success.
 */
[[nodiscard]] inline auto decode_utf8_char(
    std::u8string_view text,
    std::size_t& offset) -> result<char32_t> {
    if (offset >= text.size()) {
        return std::unexpected(make_error(error_t::not_found));
    }

    const std::uint8_t b1 = static_cast<std::uint8_t>(text[offset]);

    if (b1 <= 0x7fu) {
        ++offset;
        return static_cast<char32_t>(b1);
    }

    if (b1 >= 0xc2u && b1 <= 0xdfu) {
        if (offset + 1 >= text.size()) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        const std::uint8_t b2 = static_cast<std::uint8_t>(text[offset + 1]);
        const std::uint32_t packed =
            static_cast<std::uint32_t>(b1) |
            (static_cast<std::uint32_t>(b2) << 8);

        const char32_t ch = advanced::packed_utf8_to_utf32(packed);
        if (ch == advanced::detail::invalid_utf32) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        offset += 2;
        return ch;
    }

    if (b1 >= 0xe0u && b1 <= 0xefu) {
        if (offset + 2 >= text.size()) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        const std::uint8_t b2 = static_cast<std::uint8_t>(text[offset + 1]);
        const std::uint8_t b3 = static_cast<std::uint8_t>(text[offset + 2]);
        const std::uint32_t packed =
            static_cast<std::uint32_t>(b1) |
            (static_cast<std::uint32_t>(b2) << 8) |
            (static_cast<std::uint32_t>(b3) << 16);

        const char32_t ch = advanced::packed_utf8_to_utf32(packed);
        if (ch == advanced::detail::invalid_utf32) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        offset += 3;
        return ch;
    }

    if (b1 >= 0xf0u && b1 <= 0xf4u) {
        if (offset + 3 >= text.size()) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        const std::uint8_t b2 = static_cast<std::uint8_t>(text[offset + 1]);
        const std::uint8_t b3 = static_cast<std::uint8_t>(text[offset + 2]);
        const std::uint8_t b4 = static_cast<std::uint8_t>(text[offset + 3]);
        const std::uint32_t packed =
            static_cast<std::uint32_t>(b1) |
            (static_cast<std::uint32_t>(b2) << 8) |
            (static_cast<std::uint32_t>(b3) << 16) |
            (static_cast<std::uint32_t>(b4) << 24);

        const char32_t ch = advanced::packed_utf8_to_utf32(packed);
        if (ch == advanced::detail::invalid_utf32) {
            return std::unexpected(make_error(error_t::encoding_error));
        }

        offset += 4;
        return ch;
    }

    return std::unexpected(make_error(error_t::encoding_error));
}

/**
 * @brief Decodes a UTF-8 string into UTF-32 code points.
 *
 * @param text Source UTF-8 text.
 * @return Decoded UTF-32 sequence on success.
 */
[[nodiscard]] inline auto decode_utf8_string(
    std::u8string_view text) -> result<std::u32string> {
    std::u32string result;
    result.reserve(text.size());

    std::size_t offset = 0;
    while (offset < text.size()) {
        const auto ch = decode_utf8_char(text, offset);
        if (!ch.has_value()) {
            return std::unexpected(ch.error());
        }

        result.push_back(*ch);
    }

    return result;
}

/**
 * @brief Returns whether the specified code point can be encoded as UTF-8.
 *
 * @param ch Source code point.
 * @return true if encodable.
 * @return false otherwise.
 */
[[nodiscard]] inline auto is_valid_text_char(char32_t ch) noexcept -> bool {
    return advanced::utf32_to_packed_utf8(ch) != advanced::detail::invalid_packed_utf8;
}

/**
 * @brief Reads one character from a text stream.
 *
 * @param stream Target stream.
 * @return Read code point on success.
 */
[[nodiscard]] inline auto text_stream_read_char(
    text_stream& stream) -> result<char32_t> {
    if (!stream.has_value()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (stream.has_unget_char()) {
        const char32_t ch = stream.unget_char();
        stream.clear_unget_char();
        stream.set_eof(false);
        return ch;
    }

    char32_t ch = U'\0';
    const int result = stream.read_fn()(stream.handle(), &ch, 1);

    if (result < 0) {
        stream.set_error(true);
        return std::unexpected(make_error(error_t::io_error));
    }

    if (result == 0) {
        stream.set_eof(true);
        return std::unexpected(make_error(error_t::not_found));
    }

    stream.set_eof(false);
    return ch;
}

/**
 * @brief Writes one character to a text stream.
 *
 * @param stream Target stream.
 * @param ch Source code point.
 * @return Success or error.
 */
[[nodiscard]] inline auto text_stream_write_char(
    text_stream& stream,
    char32_t ch) -> result<void> {
    if (!stream.has_value()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (!is_valid_text_char(ch)) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    const int result = stream.write_fn()(stream.handle(), &ch, 1);
    if (result < 0 || result != 1) {
        stream.set_error(true);
        return std::unexpected(make_error(error_t::io_error));
    }

    return {};
}

/**
 * @brief Reads one line from a text stream.
 *
 * @param stream Target stream.
 * @param keep_newline Whether to keep the trailing newline.
 * @return UTF-8 line text on success.
 */
[[nodiscard]] inline auto text_stream_read_line(
    text_stream& stream,
    bool keep_newline) -> result<std::u8string> {
    std::u8string result;

    while (true) {
        const auto ch = text_stream_read_char(stream);
        if (!ch.has_value()) {
            if (ch.error().code == error_t::not_found && !result.empty()) {
                return result;
            }

            return std::unexpected(ch.error());
        }

        if (*ch == U'\n') {
            if (keep_newline) {
                const auto appended = append_utf8_char(result, *ch);
                if (!appended.has_value()) {
                    return std::unexpected(appended.error());
                }
            }

            return result;
        }

        const auto appended = append_utf8_char(result, *ch);
        if (!appended.has_value()) {
            return std::unexpected(appended.error());
        }
    }
}

/**
 * @brief Writes UTF-8 text to a text stream.
 *
 * @param stream Target stream.
 * @param text Source UTF-8 text.
 * @param append_newline Whether to append a trailing newline.
 * @return Number of UTF-8 code units requested for output on success.
 */
[[nodiscard]] inline auto text_stream_write_text(
    text_stream& stream,
    std::u8string_view text,
    bool append_newline) -> result<std::size_t> {
    if (!stream.has_value()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    auto decoded = decode_utf8_string(text);
    if (!decoded.has_value()) {
        return std::unexpected(decoded.error());
    }

    if (append_newline) {
        decoded->push_back(U'\n');
    }

    std::size_t offset = 0;
    while (offset < decoded->size()) {
        const std::size_t remaining = decoded->size() - offset;
        const std::size_t chunk_size =
            remaining > static_cast<std::size_t>(std::numeric_limits<int>::max())
                ? static_cast<std::size_t>(std::numeric_limits<int>::max())
                : remaining;

        const int written = stream.write_fn()(
            stream.handle(),
            decoded->data() + offset,
            static_cast<int>(chunk_size));

        if (written < 0 || written == 0) {
            stream.set_error(true);
            return std::unexpected(make_error(error_t::io_error));
        }

        offset += static_cast<std::size_t>(written);
    }

    return text.size() + (append_newline ? 1u : 0u);
}

/**
 * @brief Pushes one character back to a text stream.
 *
 * Only one character can be pushed back at a time.
 *
 * @param ch Character to push back.
 * @param stream Target stream.
 * @return The pushed-back character on success.
 */
[[nodiscard]] inline auto text_stream_unget_char(
    char32_t ch,
    text_stream& stream) -> result<char32_t> {
    if (!stream.has_value()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (!is_valid_text_char(ch)) {
        return std::unexpected(make_error(error_t::encoding_error));
    }

    if (stream.has_unget_char()) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    stream.set_unget_char(ch);
    stream.set_eof(false);
    return ch;
}

} // namespace detail

/**
 * @brief Reads one character from a text stream.
 *
 * @param stream Target stream.
 * @return Read code point on success.
 */
[[nodiscard]] inline auto fgetc(text_stream& stream) -> result<char32_t> {
    return detail::text_stream_read_char(stream);
}

/**
 * @brief Reads one character from the standard input text stream.
 *
 * @return Read code point on success.
 */
[[nodiscard]] inline auto getchar() -> result<char32_t> {
    return fgetc(standard_input);
}

/**
 * @brief Pushes one character back to a text stream.
 *
 * Only one character can be pushed back at a time.
 *
 * @param ch Character to push back.
 * @param stream Target stream.
 * @return The pushed-back character on success.
 */
[[nodiscard]] inline auto ungetc(
    char32_t ch,
    text_stream& stream) -> result<char32_t> {
    return detail::text_stream_unget_char(ch, stream);
}

/**
 * @brief Writes one character to a text stream.
 *
 * @param ch Source code point.
 * @param stream Target stream.
 * @return The written code point on success.
 */
[[nodiscard]] inline auto fputc(
    char32_t ch,
    text_stream& stream) -> result<char32_t> {
    const auto result = detail::text_stream_write_char(stream, ch);
    if (!result.has_value()) {
        return std::unexpected(result.error());
    }

    return ch;
}

/**
 * @brief Writes one character to the standard output text stream.
 *
 * @param ch Source code point.
 * @return The written code point on success.
 */
[[nodiscard]] inline auto putchar(char32_t ch) -> result<char32_t> {
    return fputc(ch, standard_output);
}

/**
 * @brief Reads one line from a text stream.
 *
 * @param stream Target stream.
 * @param keep_newline Whether to keep the trailing newline.
 * @return UTF-8 line text on success.
 */
[[nodiscard]] inline auto fgets(
    text_stream& stream,
    bool keep_newline = true) -> result<std::u8string> {
    return detail::text_stream_read_line(stream, keep_newline);
}

/**
 * @brief Reads one line from the standard input text stream.
 *
 * @param keep_newline Whether to keep the trailing newline.
 * @return UTF-8 line text on success.
 */
[[nodiscard]] inline auto gets(
    bool keep_newline = false) -> result<std::u8string> {
    return fgets(standard_input, keep_newline);
}

/**
 * @brief Writes UTF-8 text to a text stream.
 *
 * @param text Source UTF-8 text.
 * @param stream Target stream.
 * @param append_newline Whether to append a trailing newline.
 * @return Number of UTF-8 code units requested for output on success.
 */
[[nodiscard]] inline auto fputs(
    std::u8string_view text,
    text_stream& stream,
    bool append_newline = false) -> result<std::size_t> {
    return detail::text_stream_write_text(stream, text, append_newline);
}

/**
 * @brief Writes UTF-8 text and a trailing newline to the standard output text stream.
 *
 * @param text Source UTF-8 text.
 * @return Number of UTF-8 code units requested for output on success.
 */
[[nodiscard]] inline auto puts(
    std::u8string_view text) -> result<std::size_t> {
    return fputs(text, standard_output, true);
}

} // namespace xer

#endif /* XER_BITS_TEXT_STREAM_IO_H_INCLUDED_ */

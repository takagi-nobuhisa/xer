/**
 * @file xer/bits/stream_contents.h
 * @brief Whole-stream convenience input and output functions.
 */

#pragma once

#ifndef XER_BITS_STREAM_CONTENTS_H_INCLUDED_
#define XER_BITS_STREAM_CONTENTS_H_INCLUDED_

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <span>
#include <string>
#include <vector>

#include <xer/bits/binary_stream.h>
#include <xer/bits/binary_stream_io.h>
#include <xer/bits/text_stream.h>
#include <xer/bits/text_stream_io.h>
#include <xer/bits/utf8_char_encode.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Appends one read chunk to a byte vector.
 *
 * @param out Destination vector.
 * @param buffer Source buffer.
 * @param size Number of bytes to append.
 */
inline auto append_stream_contents_chunk(
    std::vector<std::byte>& out,
    const std::byte* buffer,
    std::size_t size) -> void
{
    out.insert(out.end(), buffer, buffer + size);
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Reads binary contents from the current stream position.
 *
 * This function reads from the current position of @p stream up to @p length
 * bytes, or until EOF is reached.
 *
 * This function intentionally does not provide an offset parameter. A stream
 * already has a current position, and callers should use `fseek` explicitly
 * before calling this function when they need to choose the starting position.
 *
 * This also avoids the confusing argument-order difference found in PHP,
 * where `file_get_contents` and `stream_get_contents` place offset and length
 * differently.
 *
 * @param stream Source binary stream.
 * @param length Maximum number of bytes to read.
 * @return Read bytes on success.
 */
[[nodiscard]] inline auto stream_get_contents(
    binary_stream& stream,
    std::uint64_t length = std::numeric_limits<std::uint64_t>::max())
    -> result<std::vector<std::byte>>
{
    std::vector<std::byte> result;

    if (length == 0) {
        return result;
    }

    std::byte buffer[8192];
    std::uint64_t remaining = length;

    while (remaining > 0) {
        const std::size_t chunk_size = static_cast<std::size_t>(
            std::min<std::uint64_t>(
                remaining,
                static_cast<std::uint64_t>(sizeof(buffer))));

        const auto read_size = fread(std::span<std::byte>(buffer, chunk_size), stream);
        if (!read_size.has_value()) {
            return std::unexpected(read_size.error());
        }

        if (*read_size == 0) {
            break;
        }

        detail::append_stream_contents_chunk(result, buffer, *read_size);
        remaining -= static_cast<std::uint64_t>(*read_size);
    }

    return result;
}

/**
 * @brief Reads text contents from the current stream position.
 *
 * This function reads characters from the current position of @p stream until
 * EOF is reached and returns the text as UTF-8.
 *
 * Like the binary overload, this function intentionally does not provide an
 * offset parameter. Callers should use the appropriate stream positioning
 * functions explicitly when they need to choose the starting position.
 *
 * @param stream Source text stream.
 * @return Read UTF-8 text on success.
 */
[[nodiscard]] inline auto stream_get_contents(text_stream& stream)
    -> result<std::u8string>
{
    std::u8string result;

    for (;;) {
        const auto ch = fgetc(stream);
        if (!ch.has_value()) {
            if (ch.error().code == error_t::not_found) {
                return result;
            }

            return std::unexpected(ch.error());
        }

        const auto appended = detail::append_utf8_char(result, *ch);
        if (!appended.has_value()) {
            return std::unexpected(appended.error());
        }
    }
}

/**
 * @brief Writes binary contents to the current stream position.
 *
 * This function writes all bytes in @p contents to @p stream starting from the
 * current stream position. Append behavior, overwriting behavior, and other
 * placement details are determined by the stream's current state and open mode.
 *
 * @param stream Destination binary stream.
 * @param contents Bytes to write.
 * @return Empty success value on success.
 */
[[nodiscard]] inline auto stream_put_contents(
    binary_stream& stream,
    std::span<const std::byte> contents) -> result<void>
{
    std::size_t offset = 0;

    while (offset < contents.size()) {
        const auto written = fwrite(contents.subspan(offset), stream);
        if (!written.has_value()) {
            return std::unexpected(written.error());
        }

        if (*written == 0) {
            return std::unexpected(make_error(error_t::io_error));
        }

        offset += *written;
    }

    return {};
}

/**
 * @brief Writes UTF-8 text to the current stream position.
 *
 * This function writes all UTF-8 text in @p contents to @p stream starting from
 * the current stream position. The stream's encoding determines how the text is
 * encoded externally.
 *
 * @param stream Destination text stream.
 * @param contents UTF-8 text to write.
 * @return Empty success value on success.
 */
[[nodiscard]] inline auto stream_put_contents(
    text_stream& stream,
    std::u8string_view contents) -> result<void>
{
    const auto written = fputs(contents, stream);
    if (!written.has_value()) {
        return std::unexpected(written.error());
    }

    return {};
}

} // namespace xer

#endif /* XER_BITS_STREAM_CONTENTS_H_INCLUDED_ */

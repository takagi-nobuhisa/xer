/**
 * @file xer/bits/file_contents.h
 * @brief Whole-file convenience input and output functions.
 */

#pragma once

#ifndef XER_BITS_FILE_CONTENTS_H_INCLUDED_
#define XER_BITS_FILE_CONTENTS_H_INCLUDED_

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <span>
#include <string>
#include <vector>

#include <xer/bits/binary_stream_io.h>
#include <xer/bits/fopen.h>
#include <xer/bits/stream_position_io.h>
#include <xer/bits/text_stream_io.h>
#include <xer/bits/utf8_char_encode.h>
#include <xer/error.h>
#include <xer/path.h>

namespace xer::detail {

/**
 * @brief Converts a uint64 byte offset to a signed stream offset.
 *
 * @param value Source offset.
 * @return Signed offset on success.
 */
[[nodiscard]] inline auto file_contents_offset_to_int64(
    std::uint64_t value) noexcept -> result<std::int64_t>
{
    if (value > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
        return std::unexpected(make_error(error_t::out_of_range));
    }

    return static_cast<std::int64_t>(value);
}

/**
 * @brief Appends one read chunk to a byte vector.
 *
 * @param out Destination vector.
 * @param buffer Source buffer.
 * @param size Number of bytes to append.
 */
inline auto append_file_contents_chunk(
    std::vector<std::byte>& out,
    const std::byte* buffer,
    std::size_t size) -> void
{
    out.insert(out.end(), buffer, buffer + size);
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Reads binary file contents.
 *
 * This overload opens the file as a binary stream. The optional @p offset and
 * @p length arguments are byte-based and are available only for the binary
 * overload. If @p offset is greater than the file size, this function returns
 * `error_t::invalid_argument`. If @p offset is exactly equal to the file size,
 * it succeeds and returns an empty byte vector.
 *
 * @param filename Source file path.
 * @param offset Byte offset from the beginning of the file.
 * @param length Maximum number of bytes to read.
 * @return File contents on success.
 */
[[nodiscard]] inline auto file_get_contents(
    const path& filename,
    std::uint64_t offset = 0,
    std::uint64_t length = std::numeric_limits<std::uint64_t>::max())
    -> result<std::vector<std::byte>>
{
    auto stream = fopen(filename, "r");
    if (!stream.has_value()) {
        return std::unexpected(stream.error());
    }

    const auto seek_end_result = fseek(*stream, 0, seek_end);
    if (!seek_end_result.has_value()) {
        return std::unexpected(seek_end_result.error());
    }

    const auto file_size = ftell(*stream);
    if (!file_size.has_value()) {
        return std::unexpected(file_size.error());
    }

    if (offset > *file_size) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const auto signed_offset = detail::file_contents_offset_to_int64(offset);
    if (!signed_offset.has_value()) {
        return std::unexpected(signed_offset.error());
    }

    const auto seek_result = fseek(*stream, *signed_offset, seek_set);
    if (!seek_result.has_value()) {
        return std::unexpected(seek_result.error());
    }

    const std::uint64_t readable_size = *file_size - offset;
    const std::uint64_t requested_size = std::min(readable_size, length);

    if (requested_size == 0) {
        return std::vector<std::byte>{};
    }

    if (requested_size > static_cast<std::uint64_t>(
                             std::vector<std::byte>{}.max_size())) {
        return std::unexpected(make_error(error_t::length_error));
    }

    std::vector<std::byte> result;
    result.reserve(static_cast<std::size_t>(requested_size));

    std::byte buffer[8192];
    std::uint64_t remaining = requested_size;

    while (remaining > 0) {
        const std::size_t chunk_size = static_cast<std::size_t>(std::min<std::uint64_t>(
            remaining,
            static_cast<std::uint64_t>(sizeof(buffer))));

        const auto read_size = fread(std::span<std::byte>(buffer, chunk_size), *stream);
        if (!read_size.has_value()) {
            return std::unexpected(read_size.error());
        }

        if (*read_size == 0) {
            break;
        }

        detail::append_file_contents_chunk(result, buffer, *read_size);
        remaining -= static_cast<std::uint64_t>(*read_size);
    }

    return result;
}

/**
 * @brief Reads text file contents.
 *
 * This overload opens the file as a text stream using the specified encoding.
 * `encoding_t::auto_detect` is valid for this input-side operation.
 *
 * @param filename Source file path.
 * @param encoding Source text encoding.
 * @return UTF-8 text on success.
 */
[[nodiscard]] inline auto file_get_contents(
    const path& filename,
    encoding_t encoding) -> result<std::u8string>
{
    auto stream = fopen(filename, "r", encoding);
    if (!stream.has_value()) {
        return std::unexpected(stream.error());
    }

    std::u8string result;

    for (;;) {
        const auto ch = fgetc(*stream);
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
 * @brief Writes binary file contents.
 *
 * This overload opens the file as a binary stream and writes @p contents from
 * the beginning of the file, replacing any existing file contents.
 *
 * XER intentionally does not provide PHP-style `flags` here. In particular,
 * append and locking behavior are not hidden inside this convenience function.
 * If locking is required, the caller should use an explicit outer operation
 * such as `flock`. If append-style stream output is required, it should be
 * expressed through stream APIs such as `fopen` with append mode and `fwrite`,
 * or through a future `stream_put_contents` helper paired with
 * `stream_get_contents`.
 *
 * @param filename Destination file path.
 * @param contents Bytes to write.
 * @return Empty success value on success.
 */
[[nodiscard]] inline auto file_put_contents(
    const path& filename,
    std::span<const std::byte> contents) -> result<void>
{
    auto stream = fopen(filename, "w");
    if (!stream.has_value()) {
        return std::unexpected(stream.error());
    }

    std::size_t offset = 0;
    while (offset < contents.size()) {
        const auto written = fwrite(contents.subspan(offset), *stream);
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
 * @brief Writes text file contents.
 *
 * This overload opens the file as a text stream using the specified encoding
 * and writes @p contents from the beginning of the file, replacing any existing
 * file contents. `encoding_t::auto_detect` is invalid for writing and returns
 * `error_t::invalid_argument`.
 *
 * XER intentionally does not provide PHP-style `flags` here. In particular,
 * append and locking behavior are not hidden inside this convenience function.
 * If locking is required, the caller should use an explicit outer operation
 * such as `flock`. If append-style stream output is required, it should be
 * expressed through stream APIs such as `fopen` with append mode and `fputs`,
 * or through a future `stream_put_contents` helper paired with
 * `stream_get_contents`.
 *
 * @param filename Destination file path.
 * @param contents UTF-8 text to write.
 * @param encoding Destination text encoding.
 * @return Empty success value on success.
 */
[[nodiscard]] inline auto file_put_contents(
    const path& filename,
    std::u8string_view contents,
    encoding_t encoding) -> result<void>
{
    if (encoding == encoding_t::auto_detect) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    auto stream = fopen(filename, "w", encoding);
    if (!stream.has_value()) {
        return std::unexpected(stream.error());
    }

    const auto written = fputs(contents, *stream);
    if (!written.has_value()) {
        return std::unexpected(written.error());
    }

    return {};
}

} // namespace xer

#endif /* XER_BITS_FILE_CONTENTS_H_INCLUDED_ */

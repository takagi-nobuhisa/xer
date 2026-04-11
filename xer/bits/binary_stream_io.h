/**
 * @file xer/bits/binary_stream_io.h
 * @brief Binary I/O operations for binary_stream.
 */

#pragma once

#ifndef XER_BITS_BINARY_STREAM_IO_H_INCLUDED_
#define XER_BITS_BINARY_STREAM_IO_H_INCLUDED_

#include <cstddef>
#include <expected>
#include <limits>
#include <span>

#include <xer/bits/binary_stream.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Converts a byte count to the internal int count used by binary_stream.
 *
 * @param size Source byte count.
 * @return Converted count on success.
 */
[[nodiscard]] inline auto binary_stream_io_size_to_int(
    std::size_t size) noexcept -> std::expected<int, error<void>> {
    if (size > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        return std::unexpected(make_error(error_t::out_of_range));
    }

    return static_cast<int>(size);
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Reads bytes from a binary stream.
 *
 * This function attempts to read up to `buffer.size()` bytes into `buffer`.
 * A partial read is reported as success with the actual number of bytes read.
 *
 * @param buffer Destination buffer.
 * @param stream Source stream.
 * @return Number of bytes read on success.
 */
[[nodiscard]] inline auto fread(
    std::span<std::byte> buffer,
    binary_stream& stream) noexcept -> std::expected<std::size_t, error<void>> {
    const auto count = detail::binary_stream_io_size_to_int(buffer.size());
    if (!count.has_value()) {
        return std::unexpected(count.error());
    }

    if (*count == 0) {
        return static_cast<std::size_t>(0);
    }

    const int result = stream.read_fn()(stream.handle(), buffer.data(), *count);
    if (result < 0) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    return static_cast<std::size_t>(result);
}

/**
 * @brief Writes bytes to a binary stream.
 *
 * This function attempts to write up to `buffer.size()` bytes from `buffer`.
 * A partial write is reported as success with the actual number of bytes written.
 *
 * @param buffer Source buffer.
 * @param stream Destination stream.
 * @return Number of bytes written on success.
 */
[[nodiscard]] inline auto fwrite(
    std::span<const std::byte> buffer,
    binary_stream& stream) noexcept -> std::expected<std::size_t, error<void>> {
    const auto count = detail::binary_stream_io_size_to_int(buffer.size());
    if (!count.has_value()) {
        return std::unexpected(count.error());
    }

    if (*count == 0) {
        return static_cast<std::size_t>(0);
    }

    const int result = stream.write_fn()(stream.handle(), buffer.data(), *count);
    if (result < 0) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    return static_cast<std::size_t>(result);
}

/**
 * @brief Reads one byte from a binary stream.
 *
 * Reaching EOF is treated as failure for this one-byte input operation.
 *
 * @param stream Source stream.
 * @return Read byte on success.
 */
[[nodiscard]] inline auto fgetb(
    binary_stream& stream) noexcept -> std::expected<std::byte, error<void>> {
    std::byte value{};
    const int result = stream.read_fn()(stream.handle(), &value, 1);

    if (result < 0) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    if (result == 0) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    return value;
}

/**
 * @brief Writes one byte to a binary stream.
 *
 * @param value Source byte.
 * @param stream Destination stream.
 * @return Success or error.
 */
[[nodiscard]] inline auto fputb(
    std::byte value,
    binary_stream& stream) noexcept -> std::expected<std::byte, error<void>> {
    const int result = stream.write_fn()(stream.handle(), &value, 1);

    if (result < 0) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    if (result == 0) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    return {};
}

} // namespace xer

#endif /* XER_BITS_BINARY_STREAM_IO_H_INCLUDED_ */

/**
 * @file xer/bits/stream_position_io.h
 * @brief Stream position operations for binary_stream and text_stream.
 */

#pragma once

#ifndef XER_BITS_STREAM_POSITION_IO_H_INCLUDED_
#define XER_BITS_STREAM_POSITION_IO_H_INCLUDED_

#include <cstdint>
#include <expected>
#include <limits>

#include <xer/bits/binary_stream.h>
#include <xer/bits/stream_position.h>
#include <xer/bits/text_stream.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Converts a signed stream position to fpos_t.
 *
 * @param value Source position.
 * @return Converted position on success.
 */
[[nodiscard]] inline auto signed_pos_to_fpos(
    std::int64_t value) noexcept -> std::expected<fpos_t, error<void>> {
    if (value < 0) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    return static_cast<fpos_t>(value);
}

/**
 * @brief Converts fpos_t to a signed stream position.
 *
 * @param value Source position.
 * @return Converted signed position on success.
 */
[[nodiscard]] inline auto fpos_to_signed_pos(
    fpos_t value) noexcept -> std::expected<std::int64_t, error<void>> {
    if (value > static_cast<fpos_t>(std::numeric_limits<std::int64_t>::max())) {
        return std::unexpected(make_error(error_t::out_of_range));
    }

    return static_cast<std::int64_t>(value);
}

/**
 * @brief Validates a seek origin value.
 *
 * @param origin Source origin.
 * @return true if valid.
 * @return false otherwise.
 */
[[nodiscard]] constexpr auto is_valid_seek_origin(seek_origin_t origin) noexcept -> bool {
    return origin == seek_set || origin == seek_cur || origin == seek_end;
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Repositions a binary stream.
 *
 * @param stream Target stream.
 * @param offset Signed byte offset.
 * @param origin Seek origin.
 * @return Success or error.
 */
[[nodiscard]] inline auto fseek(
    binary_stream& stream,
    std::int64_t offset,
    seek_origin_t origin) noexcept -> std::expected<void, error<void>> {
    if (!detail::is_valid_seek_origin(origin)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const int result = stream.seek_fn()(stream.handle(), offset, static_cast<int>(origin));
    if (result < 0) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    return {};
}

/**
 * @brief Returns the current binary stream position.
 *
 * @param stream Target stream.
 * @return Current byte offset on success.
 */
[[nodiscard]] inline auto ftell(
    binary_stream& stream) noexcept -> std::expected<std::uint64_t, error<void>> {
    const std::int64_t result = stream.tell_fn()(stream.handle());
    if (result < 0) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    return static_cast<std::uint64_t>(result);
}

/**
 * @brief Obtains a binary stream position for later restoration.
 *
 * @param stream Target stream.
 * @return Current position on success.
 */
[[nodiscard]] inline auto fgetpos(
    binary_stream& stream) noexcept -> std::expected<std::uint64_t, error<void>> {
    const auto position = ftell(stream);
    if (!position.has_value()) {
        return std::unexpected(position.error());
    }

    return static_cast<fpos_t>(*position);
}

/**
 * @brief Restores a previously obtained binary stream position.
 *
 * @param stream Target stream.
 * @param position Position value obtained from fgetpos.
 * @return Success or error.
 */
[[nodiscard]] inline auto fsetpos(
    binary_stream& stream,
    fpos_t position) noexcept -> std::expected<void, error<void>> {
    const auto signed_position = detail::fpos_to_signed_pos(position);
    if (!signed_position.has_value()) {
        return std::unexpected(signed_position.error());
    }

    return fseek(stream, *signed_position, seek_set);
}

/**
 * @brief Repositions a text stream.
 *
 * For text streams, only seek to end with offset 0 is supported here.
 * Other repositioning should use fgetpos/fsetpos.
 *
 * @param stream Target stream.
 * @param offset Signed offset.
 * @param origin Seek origin.
 * @return Success or error.
 */
[[nodiscard]] inline auto fseek(
    text_stream& stream,
    std::int64_t offset,
    seek_origin_t origin) noexcept -> std::expected<void, error<void>> {
    if (!detail::is_valid_seek_origin(origin)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (origin != seek_end || offset != 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const int result = stream.seek_end_fn()(stream.handle());
    if (result < 0) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    stream.clear_unget_char();
    return {};
}

/**
 * @brief Returns the current text stream position value.
 *
 * The returned value is suitable for later use with fsetpos.
 *
 * @param stream Target stream.
 * @return Current position value on success.
 */
[[nodiscard]] inline auto ftell(
    text_stream& stream) noexcept -> std::expected<std::uint64_t, error<void>> {
    const text_stream_pos_t result = stream.getpos_fn()(stream.handle());
    if (result < 0) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    return static_cast<std::uint64_t>(result);
}

/**
 * @brief Obtains a text stream position for later restoration.
 *
 * @param stream Target stream.
 * @return Current position on success.
 */
[[nodiscard]] inline auto fgetpos(
    text_stream& stream) noexcept -> std::expected<std::uint64_t, error<void>> {
    const text_stream_pos_t result = stream.getpos_fn()(stream.handle());
    if (result < 0) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    return static_cast<fpos_t>(result);
}

/**
 * @brief Restores a previously obtained text stream position.
 *
 * @param stream Target stream.
 * @param position Position value obtained from fgetpos.
 * @return Success or error.
 */
[[nodiscard]] inline auto fsetpos(
    text_stream& stream,
    fpos_t position) noexcept -> std::expected<void, error<void>> {
    if (position > static_cast<fpos_t>(std::numeric_limits<text_stream_pos_t>::max())) {
        return std::unexpected(make_error(error_t::out_of_range));
    }

    const int result = stream.setpos_fn()(
        stream.handle(),
        static_cast<text_stream_pos_t>(position));

    if (result < 0) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    stream.clear_unget_char();
    return {};
}

} // namespace xer

#endif /* XER_BITS_STREAM_POSITION_IO_H_INCLUDED_ */

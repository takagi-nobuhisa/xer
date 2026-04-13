/**
 * @file xer/bits/fflush.h
 * @brief Internal fflush function implementations.
 */

#pragma once

#ifndef XER_BITS_FFLUSH_H_INCLUDED_
#define XER_BITS_FFLUSH_H_INCLUDED_

#include <cstdio>
#include <expected>
#include <variant>

#include <xer/bits/fopen.h>
#include <xer/error.h>

namespace xer {

/**
 * @brief Flushes a binary stream.
 *
 * For file-backed streams, this forwards to `std::fflush`.
 * For memory-backed streams, this is a successful no-op.
 *
 * @param stream Target stream.
 * @return Empty expected on success.
 * @return Unexpected error on failure.
 */
[[nodiscard]] inline auto fflush(binary_stream& stream) noexcept -> result<void> {
    if (!stream.has_value()) {
        return std::unexpected(make_error(error_t::io_error));
    }

    detail::binary_stream_state* const state =
        detail::binary_handle_to_state(stream.handle());

    if (state == nullptr) {
        stream.set_error(true);
        return std::unexpected(make_error(error_t::io_error));
    }

    if (std::holds_alternative<detail::binary_stream_file_source>(state->source)) {
        std::FILE* const file =
            std::get<detail::binary_stream_file_source>(state->source).file;

        if (file == nullptr || std::fflush(file) != 0) {
            stream.set_error(true);
            return std::unexpected(make_error(error_t::io_error));
        }
    }

    return {};
}

/**
 * @brief Flushes a text stream.
 *
 * For file-backed streams, this forwards to `std::fflush`.
 * On successful flush, transient text decoding state is reset.
 * For string-backed streams, this is a successful no-op.
 *
 * @param stream Target stream.
 * @return Empty expected on success.
 * @return Unexpected error on failure.
 */
[[nodiscard]] inline auto fflush(text_stream& stream) noexcept -> result<void> {
    if (!stream.has_value()) {
        return std::unexpected(make_error(error_t::io_error));
    }

    detail::text_stream_state* const state =
        detail::text_handle_to_state(stream.handle());

    if (state == nullptr) {
        stream.set_error(true);
        return std::unexpected(make_error(error_t::io_error));
    }

    if (std::holds_alternative<detail::text_stream_file_source>(state->source)) {
        std::FILE* const file =
            std::get<detail::text_stream_file_source>(state->source).file;

        if (file == nullptr || std::fflush(file) != 0) {
            stream.set_error(true);
            return std::unexpected(make_error(error_t::io_error));
        }

        detail::reset_text_stream_runtime_state(*state);
    }

    return {};
}

} // namespace xer

#endif /* XER_BITS_FFLUSH_H_INCLUDED_ */

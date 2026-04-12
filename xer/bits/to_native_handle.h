/**
 * @file xer/bits/to_native_handle.h
 * @brief Native handle access functions.
 */

#pragma once

#ifndef XER_BITS_TO_NATIVE_HANDLE_H_INCLUDED_
#define XER_BITS_TO_NATIVE_HANDLE_H_INCLUDED_

#include <cstdio>
#include <expected>
#include <variant>

#include <xer/bits/fopen.h>
#include <xer/error.h>

namespace xer {

/**
 * @brief Returns the native FILE handle of a binary stream.
 *
 * This function succeeds only when the stream is file-backed.
 * Memory-backed streams do not have a native FILE handle.
 *
 * @param stream Target stream.
 * @return Native FILE handle on success.
 * @return Unexpected error on failure.
 */
[[nodiscard]] inline result<std::FILE*> to_native_handle(
    binary_stream& stream) noexcept {
    if (!stream.has_value()) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    detail::binary_stream_state* const state =
        detail::binary_handle_to_state(stream.handle());

    if (state == nullptr) {
        stream.set_error(true);
        return std::unexpected(make_error(error_t::runtime_error));
    }

    if (!std::holds_alternative<detail::binary_stream_file_source>(state->source)) {
        stream.set_error(true);
        return std::unexpected(make_error(error_t::runtime_error));
    }

    std::FILE* const file =
        std::get<detail::binary_stream_file_source>(state->source).file;

    if (file == nullptr) {
        stream.set_error(true);
        return std::unexpected(make_error(error_t::runtime_error));
    }

    return file;
}

/**
 * @brief Returns the native FILE handle of a text stream.
 *
 * This function succeeds only when the stream is file-backed.
 * String-backed streams do not have a native FILE handle.
 *
 * @param stream Target stream.
 * @return Native FILE handle on success.
 * @return Unexpected error on failure.
 *
 * @note If the returned FILE object is manipulated directly, especially by
 *       reading, writing, or repositioning, the subsequent behavior of the
 *       text_stream may become unspecified because its internal buffering and
 *       encoding state can diverge from the underlying FILE state.
 */
[[nodiscard]] inline result<std::FILE*> to_native_handle(
    text_stream& stream) noexcept {
    if (!stream.has_value()) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    detail::text_stream_state* const state =
        detail::text_handle_to_state(stream.handle());

    if (state == nullptr) {
        stream.set_error(true);
        return std::unexpected(make_error(error_t::runtime_error));
    }

    if (!std::holds_alternative<detail::text_stream_file_source>(state->source)) {
        stream.set_error(true);
        return std::unexpected(make_error(error_t::runtime_error));
    }

    std::FILE* const file =
        std::get<detail::text_stream_file_source>(state->source).file;

    if (file == nullptr) {
        stream.set_error(true);
        return std::unexpected(make_error(error_t::runtime_error));
    }

    return file;
}

} // namespace xer

#endif /* XER_BITS_TO_NATIVE_HANDLE_H_INCLUDED_ */

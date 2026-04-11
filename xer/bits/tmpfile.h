/**
 * @file xer/bits/tmpfile.h
 * @brief Temporary file stream open functions.
 */

#pragma once

#ifndef XER_BITS_TMPFILE_H_INCLUDED_
#define XER_BITS_TMPFILE_H_INCLUDED_

#include <cstdio>
#include <expected>
#include <new>

#include <xer/bits/fopen.h>
#include <xer/bits/standard_streams.h>
#include <xer/error.h>

namespace xer {

/**
 * @brief Opens a temporary binary file stream.
 *
 * The returned stream is readable and writable.
 *
 * @return Opened binary stream on success.
 * @return Unexpected error on failure.
 */
[[nodiscard]] inline std::expected<binary_stream, error<void>> tmpfile() noexcept {
    std::FILE* const file = std::tmpfile();
    if (file == nullptr) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    auto* const state = new (std::nothrow) detail::binary_stream_state();
    if (state == nullptr) {
        (void)std::fclose(file);
        return std::unexpected(make_error(error_t::runtime_error));
    }

    state->source = detail::binary_stream_file_source{file};
    state->readable = true;
    state->writable = true;
    state->append = false;

    return binary_stream(
        reinterpret_cast<binary_stream_handle_t>(state),
        detail::binary_state_close,
        detail::binary_state_read,
        detail::binary_state_write,
        detail::binary_state_seek,
        detail::binary_state_tell);
}

/**
 * @brief Opens a temporary text file stream.
 *
 * The returned stream is readable and writable.
 * `encoding_t::auto_detect` is invalid because a temporary file is created
 * for output-capable use from the beginning.
 *
 * @param encoding Text encoding selector.
 * @return Opened text stream on success.
 * @return Unexpected error on failure.
 */
[[nodiscard]] inline std::expected<text_stream, error<void>> tmpfile(
    encoding_t encoding) noexcept {
    if (encoding == encoding_t::auto_detect) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::FILE* const file = std::tmpfile();
    if (file == nullptr) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    auto* const state = new (std::nothrow) detail::text_stream_state();
    if (state == nullptr) {
        (void)std::fclose(file);
        return std::unexpected(make_error(error_t::runtime_error));
    }

    state->source = detail::text_stream_file_source{file};

    switch (encoding) {
    case encoding_t::utf8:
        state->encoding = detail::text_stream_encoding_t::utf8;
        break;

    case encoding_t::cp932:
        state->encoding = detail::text_stream_encoding_t::cp932;
        break;

    case encoding_t::auto_detect:
    default:
        delete state;
        (void)std::fclose(file);
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    detail::reset_text_stream_runtime_state(*state);

    return text_stream(
        reinterpret_cast<text_stream_handle_t>(state),
        detail::text_state_close,
        detail::standard_text_state_read,
        detail::standard_text_state_write,
        detail::text_state_getpos,
        detail::text_state_setpos,
        detail::text_state_seek_end);
}

} // namespace xer

#endif /* XER_BITS_TMPFILE_H_INCLUDED_ */

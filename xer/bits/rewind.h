/**
 * @file xer/bits/rewind.h
 * @brief Internal rewind function implementations.
 */

#pragma once

#ifndef XER_BITS_REWIND_H_INCLUDED_
#define XER_BITS_REWIND_H_INCLUDED_

#include <expected>

#include <xer/bits/binary_stream.h>
#include <xer/bits/clearerr.h>
#include <xer/bits/fopen.h>
#include <xer/bits/stream_position.h>
#include <xer/bits/text_stream.h>
#include <xer/error.h>

namespace xer {

/**
 * @brief Rewinds a binary stream to the beginning and clears stream indicators.
 *
 * Unlike the C standard library function, this function returns xer::result so
 * invalid stream objects and seek failures can be reported explicitly.
 *
 * @param stream Target binary stream.
 * @return Success or error.
 */
[[nodiscard]] inline auto rewind(binary_stream& stream) noexcept -> result<void> {
    if (!stream.has_value()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const int result = stream.seek_fn()(stream.handle(), 0, static_cast<int>(seek_set));
    if (result < 0) {
        stream.set_error(true);
        return std::unexpected(make_error(error_t::io_error));
    }

    clearerr(stream);
    return {};
}

/**
 * @brief Rewinds a text stream to the beginning and clears stream indicators.
 *
 * In addition to repositioning, this discards pushed-back characters,
 * lookahead bytes, and partial decoding state. For streams opened with
 * encoding_t::auto_detect, the concrete encoding is returned to the undecided
 * state so that detection can be performed again from the beginning.
 *
 * @param stream Target text stream.
 * @return Success or error.
 */
[[nodiscard]] inline auto rewind(text_stream& stream) noexcept -> result<void> {
    if (!stream.has_value()) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    detail::text_stream_state* const state = detail::text_handle_to_state(stream.handle());
    if (state == nullptr) {
        stream.set_error(true);
        return std::unexpected(make_error(error_t::io_error));
    }

    const int result = stream.setpos_fn()(stream.handle(), 0);
    if (result < 0) {
        stream.set_error(true);
        return std::unexpected(make_error(error_t::io_error));
    }

    detail::reset_text_stream_rewind_state(*state);
    stream.clear_unget_char();
    clearerr(stream);
    return {};
}

} // namespace xer

#endif /* XER_BITS_REWIND_H_INCLUDED_ */

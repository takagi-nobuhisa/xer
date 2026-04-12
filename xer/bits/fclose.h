/**
 * @file xer/bits/fclose.h
 * @brief Internal fclose function implementations.
 */

#pragma once

#ifndef XER_BITS_FCLOSE_H_INCLUDED_
#define XER_BITS_FCLOSE_H_INCLUDED_

#include <expected>

#include <xer/bits/binary_stream.h>
#include <xer/bits/text_stream.h>
#include <xer/error.h>

namespace xer {

/**
 * @brief Closes a binary stream.
 *
 * This function closes the specified stream and resets it to the empty state
 * regardless of whether the close operation succeeds.
 *
 * @param stream Target stream.
 * @return Empty expected on success.
 * @return Unexpected error on failure.
 */
[[nodiscard]] inline auto fclose(binary_stream& stream) noexcept -> result<void> {
    if (stream.close() < 0) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    return {};
}

/**
 * @brief Closes a text stream.
 *
 * This function closes the specified stream and resets it to the empty state
 * regardless of whether the close operation succeeds.
 *
 * @param stream Target stream.
 * @return Empty expected on success.
 * @return Unexpected error on failure.
 */
[[nodiscard]] inline auto fclose(text_stream& stream) noexcept -> result<void> {
    if (stream.close() < 0) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    return {};
}

} // namespace xer

#endif /* XER_BITS_FCLOSE_H_INCLUDED_ */

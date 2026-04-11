/**
 * @file xer/bits/clearerr.h
 * @brief Internal clearerr function implementations.
 */

#pragma once

#ifndef XER_BITS_CLEARERR_H_INCLUDED_
#define XER_BITS_CLEARERR_H_INCLUDED_

#include <xer/bits/binary_stream.h>
#include <xer/bits/text_stream.h>

namespace xer {

/**
 * @brief Clears the EOF and error indicators of the binary stream.
 *
 * This function is a no-op for an empty stream.
 *
 * @param stream Target binary stream.
 */
constexpr void clearerr(binary_stream& stream) noexcept {
    if (!stream.has_value()) {
        return;
    }

    stream.clear_indicators();
}

/**
 * @brief Clears the EOF and error indicators of the text stream.
 *
 * This function is a no-op for an empty stream.
 *
 * @param stream Target text stream.
 */
constexpr void clearerr(text_stream& stream) noexcept {
    if (!stream.has_value()) {
        return;
    }

    stream.clear_indicators();
}

} // namespace xer

#endif /* XER_BITS_CLEARERR_H_INCLUDED_ */

/**
 * @file xer/bits/feof.h
 * @brief Internal feof function implementations.
 */

#pragma once

#ifndef XER_BITS_FEOF_H_INCLUDED_
#define XER_BITS_FEOF_H_INCLUDED_

#include <xer/bits/binary_stream.h>
#include <xer/bits/text_stream.h>

namespace xer {

/**
 * @brief Returns whether the binary stream is at end-of-file state.
 *
 * @param stream Target binary stream.
 * @return true if the EOF indicator is set.
 * @return false otherwise.
 */
[[nodiscard]] constexpr bool feof(const binary_stream& stream) noexcept {
    return stream.eof();
}

/**
 * @brief Returns whether the text stream is at end-of-file state.
 *
 * @param stream Target text stream.
 * @return true if the EOF indicator is set.
 * @return false otherwise.
 */
[[nodiscard]] constexpr bool feof(const text_stream& stream) noexcept {
    return stream.eof();
}

} // namespace xer

#endif /* XER_BITS_FEOF_H_INCLUDED_ */

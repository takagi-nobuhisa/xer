/**
 * @file xer/bits/ferror.h
 * @brief Internal ferror function implementations.
 */

#pragma once

#ifndef XER_BITS_FERROR_H_INCLUDED_
#define XER_BITS_FERROR_H_INCLUDED_

#include <xer/bits/binary_stream.h>
#include <xer/bits/text_stream.h>

namespace xer {

/**
 * @brief Returns whether the binary stream is at error state.
 *
 * @param stream Target binary stream.
 * @return true if the error indicator is set.
 * @return false otherwise.
 */
[[nodiscard]] constexpr bool ferror(const binary_stream& stream) noexcept {
    return stream.error();
}

/**
 * @brief Returns whether the text stream is at error state.
 *
 * @param stream Target text stream.
 * @return true if the error indicator is set.
 * @return false otherwise.
 */
[[nodiscard]] constexpr bool ferror(const text_stream& stream) noexcept {
    return stream.error();
}

} // namespace xer

#endif /* XER_BITS_FERROR_H_INCLUDED_ */

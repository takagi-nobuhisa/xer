/**
 * @file xer/bits/mbstate.h
 * @brief State object for multibyte character conversion.
 */

#pragma once

#ifndef XER_BITS_MBSTATE_H_INCLUDED_
#define XER_BITS_MBSTATE_H_INCLUDED_

#include <cstddef>
#include <cstdint>

#include <xer/bits/common.h>

namespace xer::detail {

/**
 * @brief Encoding kinds used by multibyte conversion state.
 *
 * This enumeration is used internally to record which encoding should be used
 * to interpret buffered incomplete byte sequences.
 */
enum class multibyte_encoding : std::uint8_t {
    unspecified = 0,
    cp932,
    utf8,
};

/**
 * @brief Maximum number of buffered bytes for incomplete multibyte sequences.
 *
 * UTF-8 requires at most 4 bytes, and CP932 requires at most 2 bytes.
 * Therefore 4 bytes are sufficient for the currently supported encodings.
 */
inline constexpr std::size_t multibyte_state_buffer_size = 4;

} // namespace xer::detail

namespace xer {

/**
 * @brief State object for restartable multibyte conversion.
 *
 * This type stores the encoding kind, buffered incomplete bytes, and the number
 * of buffered bytes. It is default-constructible and starts in an unused state.
 *
 * This type does not provide a reset member function. To reinitialize the
 * state, create a new object instead.
 */
struct mbstate_t {
    /**
     * @brief Encoding kind of the currently buffered sequence.
     */
    detail::multibyte_encoding encoding;

    /**
     * @brief Buffered incomplete multibyte sequence.
     */
    std::uint8_t bytes[detail::multibyte_state_buffer_size];

    /**
     * @brief Number of valid bytes currently stored in @ref bytes.
     */
    std::uint8_t size;

    /**
     * @brief Constructs an unused state object.
     */
    constexpr mbstate_t() noexcept
        : encoding(detail::multibyte_encoding::unspecified),
          bytes{0, 0, 0, 0},
          size(0) {
    }

    /**
     * @brief Returns whether this state object currently holds no buffered bytes.
     *
     * @return `true` if no incomplete sequence is buffered, otherwise `false`.
     */
    [[nodiscard]] constexpr bool empty() const noexcept {
        return size == 0;
    }
};

} // namespace xer

#endif // XER_BITS_MBSTATE_H_INCLUDED_

/**
 * @file xer/bits/stream_position.h
 * @brief Common stream position types.
 */

#pragma once

#ifndef XER_BITS_STREAM_POSITION_H_INCLUDED_
#define XER_BITS_STREAM_POSITION_H_INCLUDED_

#include <cstdint>

namespace xer {

/**
 * @brief Stream seek origin constants.
 *
 * These constants are shared by binary_stream and text_stream.
 */
enum seek_origin_t {
    /**
     * @brief Seek relative to the beginning of the stream.
     */
    seek_set = 0,

    /**
     * @brief Seek relative to the current stream position.
     */
    seek_cur = 1,

    /**
     * @brief Seek relative to the end of the stream.
     */
    seek_end = 2,
};

/**
 * @brief Stream position type for fgetpos/fsetpos.
 *
 * For binary streams, this represents a byte offset.
 * For text streams, this is an opaque position value from the caller's perspective.
 */
using fpos_t = std::uint64_t;

} // namespace xer

#endif // XER_BITS_STREAM_POSITION_H_INCLUDED_

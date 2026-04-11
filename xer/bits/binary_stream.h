/**
 * @file xer/bits/binary_stream.h
 * @brief Internal binary stream type definition.
 */

#pragma once

#ifndef XER_BITS_BINARY_STREAM_H_INCLUDED_
#define XER_BITS_BINARY_STREAM_H_INCLUDED_

#include <cstdint>
#include <utility>

#include <xer/bits/common.h>

namespace xer {

/**
 * @brief Opaque binary stream handle type.
 */
using binary_stream_handle_t = std::uintptr_t;

/**
 * @brief Internal close function pointer type for binary streams.
 *
 * A negative return value indicates failure.
 *
 * @param handle Opaque stream handle.
 * @return Non-negative on success, negative on failure.
 */
using binary_stream_close_fn_t = int (*)(binary_stream_handle_t handle) noexcept;

/**
 * @brief Internal read function pointer type for binary streams.
 *
 * A negative return value indicates failure.
 *
 * @param handle Opaque stream handle.
 * @param s Destination buffer.
 * @param n Maximum number of bytes to read.
 * @return Number of bytes read on success, negative on failure.
 */
using binary_stream_read_fn_t =
    int (*)(binary_stream_handle_t handle, void* s, int n) noexcept;

/**
 * @brief Internal write function pointer type for binary streams.
 *
 * A negative return value indicates failure.
 *
 * @param handle Opaque stream handle.
 * @param s Source buffer.
 * @param n Number of bytes to write.
 * @return Number of bytes written on success, negative on failure.
 */
using binary_stream_write_fn_t =
    int (*)(binary_stream_handle_t handle, const void* s, int n) noexcept;

/**
 * @brief Internal seek function pointer type for binary streams.
 *
 * A negative return value indicates failure.
 *
 * @param handle Opaque stream handle.
 * @param pos Seek offset.
 * @param whence Seek origin.
 * @return Non-negative on success, negative on failure.
 */
using binary_stream_seek_fn_t =
    int (*)(binary_stream_handle_t handle, std::int64_t pos, int whence) noexcept;

/**
 * @brief Internal tell function pointer type for binary streams.
 *
 * A negative return value indicates failure.
 *
 * @param handle Opaque stream handle.
 * @return Current stream position on success, negative on failure.
 */
using binary_stream_tell_fn_t =
    std::int64_t (*)(binary_stream_handle_t handle) noexcept;

namespace detail {

/**
 * @brief Returns success for closing an empty stream.
 *
 * @return Always returns 0.
 */
inline int binary_stream_close_empty(binary_stream_handle_t) noexcept {
    return 0;
}

/**
 * @brief Returns an error for close operations.
 *
 * @return Always returns -1.
 */
inline int binary_stream_close_error(binary_stream_handle_t) noexcept {
    return -1;
}

/**
 * @brief Returns an error for read operations.
 *
 * @return Always returns -1.
 */
inline int binary_stream_read_error(binary_stream_handle_t, void*, int) noexcept {
    return -1;
}

/**
 * @brief Returns an error for write operations.
 *
 * @return Always returns -1.
 */
inline int binary_stream_write_error(binary_stream_handle_t, const void*, int) noexcept {
    return -1;
}

/**
 * @brief Returns an error for seek operations.
 *
 * @return Always returns -1.
 */
inline int binary_stream_seek_error(binary_stream_handle_t, std::int64_t, int) noexcept {
    return -1;
}

/**
 * @brief Returns an error for tell operations.
 *
 * @return Always returns -1.
 */
inline std::int64_t binary_stream_tell_error(binary_stream_handle_t) noexcept {
    return -1;
}

} // namespace detail

/**
 * @brief Move-only binary stream object.
 *
 * This type stores an opaque handle and internal function pointers.
 * Destruction closes the stream automatically if it is open.
 */
class binary_stream {
public:
    /**
     * @brief Constructs an empty stream.
     */
    constexpr binary_stream() noexcept = default;

    /**
     * @brief Constructs a stream from an opaque handle and function table.
     *
     * @param handle Stream handle.
     * @param close_fn Close function.
     * @param read_fn Read function.
     * @param write_fn Write function.
     * @param seek_fn Seek function.
     * @param tell_fn Tell function.
     */
    constexpr binary_stream(
        binary_stream_handle_t handle,
        binary_stream_close_fn_t close_fn,
        binary_stream_read_fn_t read_fn,
        binary_stream_write_fn_t write_fn,
        binary_stream_seek_fn_t seek_fn,
        binary_stream_tell_fn_t tell_fn) noexcept
        : handle_(handle),
          close_fn_(close_fn),
          read_fn_(read_fn),
          write_fn_(write_fn),
          seek_fn_(seek_fn),
          tell_fn_(tell_fn),
          has_value_(true),
          eof_(false),
          error_(false) {}

    binary_stream(const binary_stream&) = delete;
    binary_stream& operator=(const binary_stream&) = delete;

    /**
     * @brief Move-constructs a stream.
     *
     * @param other Source stream.
     */
    constexpr binary_stream(binary_stream&& other) noexcept
        : handle_(other.handle_),
          close_fn_(other.close_fn_),
          read_fn_(other.read_fn_),
          write_fn_(other.write_fn_),
          seek_fn_(other.seek_fn_),
          tell_fn_(other.tell_fn_),
          has_value_(other.has_value_),
          eof_(other.eof_),
          error_(other.error_) {
        other.reset();
    }

    /**
     * @brief Move-assigns a stream.
     *
     * The current stream is closed before taking ownership from the source.
     *
     * @param other Source stream.
     * @return Reference to this object.
     */
    constexpr binary_stream& operator=(binary_stream&& other) noexcept {
        if (this != &other) {
            (void)close();

            handle_ = other.handle_;
            close_fn_ = other.close_fn_;
            read_fn_ = other.read_fn_;
            write_fn_ = other.write_fn_;
            seek_fn_ = other.seek_fn_;
            tell_fn_ = other.tell_fn_;
            has_value_ = other.has_value_;
            eof_ = other.eof_;
            error_ = other.error_;

            other.reset();
        }

        return *this;
    }

    /**
     * @brief Destroys the stream.
     *
     * Any close failure is ignored.
     */
    ~binary_stream() {
        (void)close();
    }

    /**
     * @brief Closes the stream if it is open.
     *
     * This function is a no-op for an empty stream.
     * After this function returns, the stream becomes empty regardless of
     * whether the close operation succeeded.
     *
     * @return Non-negative on success, negative on failure.
     */
    int close() noexcept {
        if (!has_value_) {
            return 0;
        }

        const int result = close_fn_(handle_);
        reset();
        return result;
    }

    /**
     * @brief Returns whether the stream is non-empty.
     *
     * @return true if the stream is open.
     * @return false otherwise.
     */
    [[nodiscard]] constexpr bool has_value() const noexcept {
        return has_value_;
    }

    /**
     * @brief Returns whether the end-of-file indicator is set.
     *
     * @return true if the stream is at end-of-file state.
     * @return false otherwise.
     */
    [[nodiscard]] constexpr bool eof() const noexcept {
        return eof_;
    }

    /**
     * @brief Returns whether the error indicator is set.
     *
     * @return true if the stream is at error state.
     * @return false otherwise.
     */
    [[nodiscard]] constexpr bool error() const noexcept {
        return error_;
    }

    /**
     * @brief Sets the end-of-file indicator.
     *
     * @param value New EOF state.
     */
    constexpr void set_eof(bool value) noexcept {
        eof_ = value;
    }

    /**
     * @brief Sets the error indicator.
     *
     * @param value New error state.
     */
    constexpr void set_error(bool value) noexcept {
        error_ = value;
    }

    /**
     * @brief Clears both EOF and error indicators.
     */
    constexpr void clear_indicators() noexcept {
        eof_ = false;
        error_ = false;
    }

    /**
     * @brief Returns the opaque stream handle.
     *
     * @return Stored stream handle.
     */
    [[nodiscard]] constexpr binary_stream_handle_t handle() const noexcept {
        return handle_;
    }

    /**
     * @brief Returns the stored close function.
     *
     * @return Close function pointer.
     */
    [[nodiscard]] constexpr binary_stream_close_fn_t close_fn() const noexcept {
        return close_fn_;
    }

    /**
     * @brief Returns the stored read function.
     *
     * @return Read function pointer.
     */
    [[nodiscard]] constexpr binary_stream_read_fn_t read_fn() const noexcept {
        return read_fn_;
    }

    /**
     * @brief Returns the stored write function.
     *
     * @return Write function pointer.
     */
    [[nodiscard]] constexpr binary_stream_write_fn_t write_fn() const noexcept {
        return write_fn_;
    }

    /**
     * @brief Returns the stored seek function.
     *
     * @return Seek function pointer.
     */
    [[nodiscard]] constexpr binary_stream_seek_fn_t seek_fn() const noexcept {
        return seek_fn_;
    }

    /**
     * @brief Returns the stored tell function.
     *
     * @return Tell function pointer.
     */
    [[nodiscard]] constexpr binary_stream_tell_fn_t tell_fn() const noexcept {
        return tell_fn_;
    }

    /**
     * @brief Swaps two streams.
     *
     * @param other Target stream.
     */
    constexpr void swap(binary_stream& other) noexcept {
        std::swap(handle_, other.handle_);
        std::swap(close_fn_, other.close_fn_);
        std::swap(read_fn_, other.read_fn_);
        std::swap(write_fn_, other.write_fn_);
        std::swap(seek_fn_, other.seek_fn_);
        std::swap(tell_fn_, other.tell_fn_);
        std::swap(has_value_, other.has_value_);
        std::swap(eof_, other.eof_);
        std::swap(error_, other.error_);
    }

private:
    /**
     * @brief Resets the stream to the empty state.
     */
    constexpr void reset() noexcept {
        handle_ = 0;
        close_fn_ = detail::binary_stream_close_empty;
        read_fn_ = detail::binary_stream_read_error;
        write_fn_ = detail::binary_stream_write_error;
        seek_fn_ = detail::binary_stream_seek_error;
        tell_fn_ = detail::binary_stream_tell_error;
        has_value_ = false;
        eof_ = false;
        error_ = false;
    }

    binary_stream_handle_t handle_ = 0;
    binary_stream_close_fn_t close_fn_ = detail::binary_stream_close_empty;
    binary_stream_read_fn_t read_fn_ = detail::binary_stream_read_error;
    binary_stream_write_fn_t write_fn_ = detail::binary_stream_write_error;
    binary_stream_seek_fn_t seek_fn_ = detail::binary_stream_seek_error;
    binary_stream_tell_fn_t tell_fn_ = detail::binary_stream_tell_error;
    bool has_value_ = false;
    bool eof_ = false;
    bool error_ = false;
};

/**
 * @brief Swaps two binary streams.
 *
 * @param lhs Left stream.
 * @param rhs Right stream.
 */
constexpr void swap(binary_stream& lhs, binary_stream& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace xer

#endif /* XER_BITS_BINARY_STREAM_H_INCLUDED_ */

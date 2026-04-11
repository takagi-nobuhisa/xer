/**
 * @file xer/bits/text_stream.h
 * @brief Internal text stream type definition.
 */

#pragma once

#ifndef XER_BITS_TEXT_STREAM_H_INCLUDED_
#define XER_BITS_TEXT_STREAM_H_INCLUDED_

#include <cstdint>
#include <utility>

#include <xer/bits/common.h>

namespace xer {

/**
 * @brief Opaque text stream handle type.
 */
using text_stream_handle_t = std::uintptr_t;

/**
 * @brief Opaque text stream position type.
 *
 * A negative value is reserved for errors.
 */
using text_stream_pos_t = std::int64_t;

/**
 * @brief Internal close function pointer type for text streams.
 *
 * A negative return value indicates failure.
 *
 * @param handle Opaque stream handle.
 * @return Non-negative on success, negative on failure.
 */
using text_stream_close_fn_t = int (*)(text_stream_handle_t handle) noexcept;

/**
 * @brief Internal read function pointer type for text streams.
 *
 * A negative return value indicates failure.
 *
 * @param handle Opaque stream handle.
 * @param s Destination buffer.
 * @param n Maximum number of characters to read.
 * @return Number of characters read on success, negative on failure.
 */
using text_stream_read_fn_t =
    int (*)(text_stream_handle_t handle, char32_t* s, int n) noexcept;

/**
 * @brief Internal write function pointer type for text streams.
 *
 * A negative return value indicates failure.
 *
 * @param handle Opaque stream handle.
 * @param s Source buffer.
 * @param n Number of characters to write.
 * @return Number of characters written on success, negative on failure.
 */
using text_stream_write_fn_t =
    int (*)(text_stream_handle_t handle, const char32_t* s, int n) noexcept;

/**
 * @brief Internal get-position function pointer type for text streams.
 *
 * A negative return value indicates failure.
 *
 * @param handle Opaque stream handle.
 * @return Current stream position on success, negative on failure.
 */
using text_stream_getpos_fn_t = text_stream_pos_t (*)(text_stream_handle_t handle) noexcept;

/**
 * @brief Internal set-position function pointer type for text streams.
 *
 * A negative return value indicates failure.
 *
 * @param handle Opaque stream handle.
 * @param pos Position previously obtained by getpos.
 * @return Non-negative on success, negative on failure.
 */
using text_stream_setpos_fn_t =
    int (*)(text_stream_handle_t handle, text_stream_pos_t pos) noexcept;

/**
 * @brief Internal seek-end function pointer type for text streams.
 *
 * A negative return value indicates failure.
 *
 * @param handle Opaque stream handle.
 * @return Non-negative on success, negative on failure.
 */
using text_stream_seek_end_fn_t = int (*)(text_stream_handle_t handle) noexcept;

namespace detail {

/**
 * @brief Returns success for closing an empty stream.
 *
 * @return Always returns 0.
 */
inline int text_stream_close_empty(text_stream_handle_t) noexcept {
    return 0;
}

/**
 * @brief Returns an error for close operations.
 *
 * @return Always returns -1.
 */
inline int text_stream_close_error(text_stream_handle_t) noexcept {
    return -1;
}

/**
 * @brief Returns an error for read operations.
 *
 * @return Always returns -1.
 */
inline int text_stream_read_error(text_stream_handle_t, char32_t*, int) noexcept {
    return -1;
}

/**
 * @brief Returns an error for write operations.
 *
 * @return Always returns -1.
 */
inline int text_stream_write_error(text_stream_handle_t, const char32_t*, int) noexcept {
    return -1;
}

/**
 * @brief Returns an error for get-position operations.
 *
 * @return Always returns -1.
 */
inline text_stream_pos_t text_stream_getpos_error(text_stream_handle_t) noexcept {
    return -1;
}

/**
 * @brief Returns an error for set-position operations.
 *
 * @return Always returns -1.
 */
inline int text_stream_setpos_error(text_stream_handle_t, text_stream_pos_t) noexcept {
    return -1;
}

/**
 * @brief Returns an error for seek-end operations.
 *
 * @return Always returns -1.
 */
inline int text_stream_seek_end_error(text_stream_handle_t) noexcept {
    return -1;
}

} // namespace detail

/**
 * @brief Move-only text stream object.
 *
 * This type stores an opaque handle and internal function pointers.
 * Destruction closes the stream automatically if it is open.
 */
class text_stream {
public:
    /**
     * @brief Constructs an empty stream.
     */
    constexpr text_stream() noexcept = default;

    /**
     * @brief Constructs a stream from an opaque handle and function table.
     *
     * @param handle Stream handle.
     * @param close_fn Close function.
     * @param read_fn Read function.
     * @param write_fn Write function.
     * @param getpos_fn Get-position function.
     * @param setpos_fn Set-position function.
     * @param seek_end_fn Seek-end function.
     */
    constexpr text_stream(
        text_stream_handle_t handle,
        text_stream_close_fn_t close_fn,
        text_stream_read_fn_t read_fn,
        text_stream_write_fn_t write_fn,
        text_stream_getpos_fn_t getpos_fn,
        text_stream_setpos_fn_t setpos_fn,
        text_stream_seek_end_fn_t seek_end_fn) noexcept
        : handle_(handle),
          close_fn_(close_fn),
          read_fn_(read_fn),
          write_fn_(write_fn),
          getpos_fn_(getpos_fn),
          setpos_fn_(setpos_fn),
          seek_end_fn_(seek_end_fn),
          has_value_(true),
          eof_(false),
          error_(false),
          has_unget_char_(false),
          unget_char_(U'\0') {}

    text_stream(const text_stream&) = delete;
    text_stream& operator=(const text_stream&) = delete;

    /**
     * @brief Move-constructs a stream.
     *
     * @param other Source stream.
     */
    constexpr text_stream(text_stream&& other) noexcept
        : handle_(other.handle_),
          close_fn_(other.close_fn_),
          read_fn_(other.read_fn_),
          write_fn_(other.write_fn_),
          getpos_fn_(other.getpos_fn_),
          setpos_fn_(other.setpos_fn_),
          seek_end_fn_(other.seek_end_fn_),
          has_value_(other.has_value_),
          eof_(other.eof_),
          error_(other.error_),
          has_unget_char_(other.has_unget_char_),
          unget_char_(other.unget_char_) {
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
    constexpr text_stream& operator=(text_stream&& other) noexcept {
        if (this != &other) {
            (void)close();

            handle_ = other.handle_;
            close_fn_ = other.close_fn_;
            read_fn_ = other.read_fn_;
            write_fn_ = other.write_fn_;
            getpos_fn_ = other.getpos_fn_;
            setpos_fn_ = other.setpos_fn_;
            seek_end_fn_ = other.seek_end_fn_;
            has_value_ = other.has_value_;
            eof_ = other.eof_;
            error_ = other.error_;
            has_unget_char_ = other.has_unget_char_;
            unget_char_ = other.unget_char_;

            other.reset();
        }

        return *this;
    }

    /**
     * @brief Destroys the stream.
     *
     * Any close failure is ignored.
     */
    ~text_stream() {
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
     * @brief Returns whether an ungotten character is pending.
     *
     * @return true if one character has been pushed back.
     * @return false otherwise.
     */
    [[nodiscard]] constexpr bool has_unget_char() const noexcept {
        return has_unget_char_;
    }

    /**
     * @brief Returns the currently stored ungotten character.
     *
     * This function is valid only when has_unget_char() is true.
     *
     * @return Stored pushed-back character.
     */
    [[nodiscard]] constexpr char32_t unget_char() const noexcept {
        return unget_char_;
    }

    /**
     * @brief Stores one pushed-back character.
     *
     * @param ch Character to store.
     */
    constexpr void set_unget_char(char32_t ch) noexcept {
        has_unget_char_ = true;
        unget_char_ = ch;
    }

    /**
     * @brief Clears the pushed-back character slot.
     */
    constexpr void clear_unget_char() noexcept {
        has_unget_char_ = false;
        unget_char_ = U'\0';
    }

    /**
     * @brief Returns the opaque stream handle.
     *
     * @return Stored stream handle.
     */
    [[nodiscard]] constexpr text_stream_handle_t handle() const noexcept {
        return handle_;
    }

    /**
     * @brief Returns the stored close function.
     *
     * @return Close function pointer.
     */
    [[nodiscard]] constexpr text_stream_close_fn_t close_fn() const noexcept {
        return close_fn_;
    }

    /**
     * @brief Returns the stored read function.
     *
     * @return Read function pointer.
     */
    [[nodiscard]] constexpr text_stream_read_fn_t read_fn() const noexcept {
        return read_fn_;
    }

    /**
     * @brief Returns the stored write function.
     *
     * @return Write function pointer.
     */
    [[nodiscard]] constexpr text_stream_write_fn_t write_fn() const noexcept {
        return write_fn_;
    }

    /**
     * @brief Returns the stored get-position function.
     *
     * @return Get-position function pointer.
     */
    [[nodiscard]] constexpr text_stream_getpos_fn_t getpos_fn() const noexcept {
        return getpos_fn_;
    }

    /**
     * @brief Returns the stored set-position function.
     *
     * @return Set-position function pointer.
     */
    [[nodiscard]] constexpr text_stream_setpos_fn_t setpos_fn() const noexcept {
        return setpos_fn_;
    }

    /**
     * @brief Returns the stored seek-end function.
     *
     * @return Seek-end function pointer.
     */
    [[nodiscard]] constexpr text_stream_seek_end_fn_t seek_end_fn() const noexcept {
        return seek_end_fn_;
    }

    /**
     * @brief Swaps two streams.
     *
     * @param other Target stream.
     */
    constexpr void swap(text_stream& other) noexcept {
        std::swap(handle_, other.handle_);
        std::swap(close_fn_, other.close_fn_);
        std::swap(read_fn_, other.read_fn_);
        std::swap(write_fn_, other.write_fn_);
        std::swap(getpos_fn_, other.getpos_fn_);
        std::swap(setpos_fn_, other.setpos_fn_);
        std::swap(seek_end_fn_, other.seek_end_fn_);
        std::swap(has_value_, other.has_value_);
        std::swap(eof_, other.eof_);
        std::swap(error_, other.error_);
        std::swap(has_unget_char_, other.has_unget_char_);
        std::swap(unget_char_, other.unget_char_);
    }

private:
    /**
     * @brief Resets the stream to the empty state.
     */
    constexpr void reset() noexcept {
        handle_ = 0;
        close_fn_ = detail::text_stream_close_empty;
        read_fn_ = detail::text_stream_read_error;
        write_fn_ = detail::text_stream_write_error;
        getpos_fn_ = detail::text_stream_getpos_error;
        setpos_fn_ = detail::text_stream_setpos_error;
        seek_end_fn_ = detail::text_stream_seek_end_error;
        has_value_ = false;
        eof_ = false;
        error_ = false;
        has_unget_char_ = false;
        unget_char_ = U'\0';
    }

    text_stream_handle_t handle_ = 0;
    text_stream_close_fn_t close_fn_ = detail::text_stream_close_empty;
    text_stream_read_fn_t read_fn_ = detail::text_stream_read_error;
    text_stream_write_fn_t write_fn_ = detail::text_stream_write_error;
    text_stream_getpos_fn_t getpos_fn_ = detail::text_stream_getpos_error;
    text_stream_setpos_fn_t setpos_fn_ = detail::text_stream_setpos_error;
    text_stream_seek_end_fn_t seek_end_fn_ = detail::text_stream_seek_end_error;
    bool has_value_ = false;
    bool eof_ = false;
    bool error_ = false;
    bool has_unget_char_ = false;
    char32_t unget_char_ = U'\0';
};

/**
 * @brief Swaps two text streams.
 *
 * @param lhs Left stream.
 * @param rhs Right stream.
 */
constexpr void swap(text_stream& lhs, text_stream& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace xer

#endif /* XER_BITS_TEXT_STREAM_H_INCLUDED_ */

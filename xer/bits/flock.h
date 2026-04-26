/**
 * @file xer/bits/flock.h
 * @brief Advisory file locking for file-backed streams.
 */

#pragma once

#ifndef XER_BITS_FLOCK_H_INCLUDED_
#define XER_BITS_FLOCK_H_INCLUDED_

#include <cstdio>
#include <cstdint>
#include <expected>
#include <type_traits>

#include <xer/bits/binary_stream.h>
#include <xer/bits/text_stream.h>
#include <xer/bits/to_native_handle.h>
#include <xer/error.h>

#ifdef _WIN32
#    include <io.h>
#    include <windows.h>
#else
#    include <cerrno>
#    include <sys/file.h>
#    include <unistd.h>
#endif

namespace xer {

/**
 * @brief File lock operation flags for flock.
 *
 * These values are used with xer::flock. Use lock_sh, lock_ex, or lock_un as
 * the base operation, and optionally combine lock_nb with lock_sh or lock_ex.
 */
enum class lock_t : unsigned {
    sh = 1,
    ex = 2,
    un = 8,
    nb = 4,
};

/**
 * @brief Shared lock operation.
 */
inline constexpr auto lock_sh = lock_t::sh;

/**
 * @brief Exclusive lock operation.
 */
inline constexpr auto lock_ex = lock_t::ex;

/**
 * @brief Unlock operation.
 */
inline constexpr auto lock_un = lock_t::un;

/**
 * @brief Nonblocking modifier for shared or exclusive lock operations.
 */
inline constexpr auto lock_nb = lock_t::nb;

/**
 * @brief Combines file lock operation flags.
 *
 * @param lhs Left-hand flag value.
 * @param rhs Right-hand flag value.
 * @return Combined flag value.
 */
[[nodiscard]] constexpr auto operator|(lock_t lhs, lock_t rhs) noexcept -> lock_t {
    using value_type = std::underlying_type_t<lock_t>;

    return static_cast<lock_t>(
        static_cast<value_type>(lhs) | static_cast<value_type>(rhs));
}

/**
 * @brief Intersects file lock operation flags.
 *
 * @param lhs Left-hand flag value.
 * @param rhs Right-hand flag value.
 * @return Intersected flag value.
 */
[[nodiscard]] constexpr auto operator&(lock_t lhs, lock_t rhs) noexcept -> lock_t {
    using value_type = std::underlying_type_t<lock_t>;

    return static_cast<lock_t>(
        static_cast<value_type>(lhs) & static_cast<value_type>(rhs));
}

namespace detail {

/**
 * @brief Returns true when the operation has the nonblocking modifier.
 *
 * @param operation Lock operation flags.
 * @return true if lock_nb is present.
 */
[[nodiscard]] constexpr auto lock_has_nonblocking(lock_t operation) noexcept -> bool {
    using value_type = std::underlying_type_t<lock_t>;

    return (static_cast<value_type>(operation) &
            static_cast<value_type>(lock_t::nb)) != 0;
}

/**
 * @brief Removes the nonblocking modifier from a lock operation.
 *
 * @param operation Lock operation flags.
 * @return Base operation without lock_nb.
 */
[[nodiscard]] constexpr auto lock_base_operation(lock_t operation) noexcept -> lock_t {
    using value_type = std::underlying_type_t<lock_t>;

    return static_cast<lock_t>(
        static_cast<value_type>(operation) &
        ~static_cast<value_type>(lock_t::nb));
}

#ifdef _WIN32

/**
 * @brief Converts a Windows lock failure to an XER error code.
 *
 * @param value Windows error code.
 * @return Converted error code.
 */
[[nodiscard]] inline auto lock_win32_error_to_error_t(unsigned long value) noexcept
    -> error_t {
    switch (value) {
    case ERROR_LOCK_VIOLATION:
    case ERROR_SHARING_VIOLATION:
    case ERROR_BUSY:
        return error_t::busy;

    case ERROR_ACCESS_DENIED:
        return error_t::acces;

    case ERROR_INVALID_HANDLE:
        return error_t::badf;

    case ERROR_INVALID_PARAMETER:
        return error_t::inval;

    case ERROR_NOT_ENOUGH_MEMORY:
    case ERROR_OUTOFMEMORY:
        return error_t::nomem;

    default:
        return error_t::io_error;
    }
}

#else

/**
 * @brief Converts errno to xer::error_t.
 *
 * @param value errno value.
 * @return Converted error code.
 */
[[nodiscard]] inline auto lock_errno_to_error_t(int value) noexcept -> error_t {
    if (value == EWOULDBLOCK) {
        return error_t::busy;
    }

    return static_cast<error_t>(value);
}

#endif

/**
 * @brief Performs platform-specific file locking on a native FILE handle.
 *
 * @param file Native FILE handle.
 * @param operation Lock operation flags.
 * @param would_block Optional output flag set when nonblocking lock acquisition
 *                    fails because the lock would block.
 * @return Empty success value on success.
 */
[[nodiscard]] inline auto flock_native_file(
    std::FILE* file,
    lock_t operation,
    bool* would_block) noexcept -> result<void> {
    if (would_block != nullptr) {
        *would_block = false;
    }

    if (file == nullptr) {
        return std::unexpected(make_error(error_t::badf));
    }

    const bool nonblocking = lock_has_nonblocking(operation);
    const lock_t base_operation = lock_base_operation(operation);

    if (base_operation != lock_t::sh && base_operation != lock_t::ex &&
        base_operation != lock_t::un) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (base_operation == lock_t::un && nonblocking) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

#ifdef _WIN32
    const int fd = ::_fileno(file);
    if (fd < 0) {
        return std::unexpected(make_error(error_t::badf));
    }

    const intptr_t os_handle = ::_get_osfhandle(fd);
    if (os_handle == -1) {
        return std::unexpected(make_error(error_t::badf));
    }

    HANDLE const handle = reinterpret_cast<HANDLE>(os_handle);
    OVERLAPPED overlapped {};

    constexpr DWORD lock_low = 0xffffffffUL;
    constexpr DWORD lock_high = 0xffffffffUL;

    if (base_operation == lock_t::un) {
        if (::UnlockFileEx(handle, 0, lock_low, lock_high, &overlapped) == 0) {
            return std::unexpected(
                make_error(lock_win32_error_to_error_t(::GetLastError())));
        }

        return {};
    }

    DWORD flags = 0;
    if (base_operation == lock_t::ex) {
        flags |= LOCKFILE_EXCLUSIVE_LOCK;
    }
    if (nonblocking) {
        flags |= LOCKFILE_FAIL_IMMEDIATELY;
    }

    if (::LockFileEx(handle, flags, 0, lock_low, lock_high, &overlapped) == 0) {
        const DWORD error = ::GetLastError();
        if (nonblocking &&
            (error == ERROR_LOCK_VIOLATION || error == ERROR_SHARING_VIOLATION ||
             error == ERROR_BUSY)) {
            if (would_block != nullptr) {
                *would_block = true;
            }
            return std::unexpected(make_error(error_t::busy));
        }

        return std::unexpected(make_error(lock_win32_error_to_error_t(error)));
    }

    return {};
#else
    const int fd = ::fileno(file);
    if (fd < 0) {
        return std::unexpected(make_error(error_t::badf));
    }

    int command = 0;
    switch (base_operation) {
    case lock_t::sh:
        command = LOCK_SH;
        break;

    case lock_t::ex:
        command = LOCK_EX;
        break;

    case lock_t::un:
        command = LOCK_UN;
        break;

    default:
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (nonblocking) {
        command |= LOCK_NB;
    }

    if (::flock(fd, command) != 0) {
        const int saved_errno = errno;
        if (nonblocking &&
            (saved_errno == EWOULDBLOCK || saved_errno == EAGAIN ||
             saved_errno == EACCES)) {
            if (would_block != nullptr) {
                *would_block = true;
            }
            return std::unexpected(make_error(error_t::busy));
        }

        return std::unexpected(make_error(lock_errno_to_error_t(saved_errno)));
    }

    return {};
#endif
}

} // namespace detail

/**
 * @brief Locks or unlocks a file-backed binary stream.
 *
 * This function provides PHP-style advisory file locking for XER streams.
 * Use lock_sh for a shared lock, lock_ex for an exclusive lock, and lock_un to
 * release a lock. lock_nb may be combined with lock_sh or lock_ex to request a
 * nonblocking lock attempt.
 *
 * This function requires a stream that can expose a native FILE handle through
 * to_native_handle. Memory-backed streams, socket-backed streams, and other
 * non-file streams may fail.
 *
 * The lock is advisory. It does not forcibly prevent access by code that does
 * not follow the same locking convention. The filesystem state and lock state
 * may also change immediately after this function returns. Network filesystems
 * and platform-specific filesystems may have environment-dependent behavior.
 *
 * When lock_nb is used and the lock cannot be acquired immediately, this
 * function returns failure with error_t::busy and sets *would_block to true if
 * would_block is not null. For other failures, *would_block is set to false.
 *
 * Locks are associated with the underlying native file handle and are normally
 * released by the platform when the stream is closed, but explicit lock_un is
 * recommended when the protected section ends.
 *
 * @param stream Target stream.
 * @param operation Lock operation flags.
 * @param would_block Optional output flag for nonblocking lock conflicts.
 * @return Empty success value on success.
 */
[[nodiscard]] inline auto flock(
    binary_stream& stream,
    lock_t operation,
    bool* would_block = nullptr) noexcept -> result<void> {
    const auto file = to_native_handle(stream);
    if (!file.has_value()) {
        if (would_block != nullptr) {
            *would_block = false;
        }
        return std::unexpected(file.error());
    }

    const auto result = detail::flock_native_file(*file, operation, would_block);
    if (!result.has_value()) {
        stream.set_error(true);
    }

    return result;
}

/**
 * @brief Locks or unlocks a file-backed text stream.
 *
 * This function provides PHP-style advisory file locking for XER streams.
 * Use lock_sh for a shared lock, lock_ex for an exclusive lock, and lock_un to
 * release a lock. lock_nb may be combined with lock_sh or lock_ex to request a
 * nonblocking lock attempt.
 *
 * This function requires a stream that can expose a native FILE handle through
 * to_native_handle. String-backed streams, socket-backed streams, and other
 * non-file streams may fail.
 *
 * The lock is advisory. It does not forcibly prevent access by code that does
 * not follow the same locking convention. The filesystem state and lock state
 * may also change immediately after this function returns. Network filesystems
 * and platform-specific filesystems may have environment-dependent behavior.
 *
 * When lock_nb is used and the lock cannot be acquired immediately, this
 * function returns failure with error_t::busy and sets *would_block to true if
 * would_block is not null. For other failures, *would_block is set to false.
 *
 * Locks are associated with the underlying native file handle and are normally
 * released by the platform when the stream is closed, but explicit lock_un is
 * recommended when the protected section ends.
 *
 * For text streams, callers should avoid direct native-handle I/O while the
 * text_stream may still use its own buffering and encoding state. flock itself
 * does not read, write, or reposition the native FILE handle.
 *
 * @param stream Target stream.
 * @param operation Lock operation flags.
 * @param would_block Optional output flag for nonblocking lock conflicts.
 * @return Empty success value on success.
 */
[[nodiscard]] inline auto flock(
    text_stream& stream,
    lock_t operation,
    bool* would_block = nullptr) noexcept -> result<void> {
    const auto file = to_native_handle(stream);
    if (!file.has_value()) {
        if (would_block != nullptr) {
            *would_block = false;
        }
        return std::unexpected(file.error());
    }

    const auto result = detail::flock_native_file(*file, operation, would_block);
    if (!result.has_value()) {
        stream.set_error(true);
    }

    return result;
}

} // namespace xer

#endif /* XER_BITS_FLOCK_H_INCLUDED_ */

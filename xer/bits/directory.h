/**
 * @file xer/bits/directory.h
 * @brief Directory stream operations.
 */

#pragma once

#ifndef XER_BITS_DIRECTORY_H_INCLUDED_
#define XER_BITS_DIRECTORY_H_INCLUDED_

#include <cerrno>
#include <dirent.h>
#include <expected>
#include <string>
#include <utility>

#include <xer/error.h>
#include <xer/path.h>

namespace xer {

namespace detail {

#ifdef _WIN32
using native_dir_handle_t = _WDIR*;
using native_dirent_t = _wdirent;
#else
using native_dir_handle_t = DIR*;
using native_dirent_t = dirent;
#endif

/**
 * @brief Converts errno to xer::error_t.
 *
 * @param value errno value.
 * @return Converted error code.
 */
[[nodiscard]] inline auto dirent_errno_to_error_t(int value) noexcept -> error_t {
    return static_cast<error_t>(value);
}

/**
 * @brief Closes a native directory handle.
 *
 * @param handle Native directory handle.
 * @return Non-negative on success, negative on failure.
 */
inline auto close_native_dir(native_dir_handle_t handle) noexcept -> int {
    if (handle == nullptr) {
        return 0;
    }

#ifdef _WIN32
    return ::_wclosedir(handle);
#else
    return ::closedir(handle);
#endif
}

/**
 * @brief Opens a native directory handle.
 *
 * @param dirname Native directory path.
 * @return Native directory handle on success, otherwise nullptr.
 */
[[nodiscard]] inline auto open_native_dir(
    const native_path_string& dirname) noexcept -> native_dir_handle_t {
#ifdef _WIN32
    return ::_wopendir(dirname.c_str());
#else
    return ::opendir(dirname.c_str());
#endif
}

/**
 * @brief Reads a native directory entry.
 *
 * @param handle Native directory handle.
 * @return Native directory entry on success, otherwise nullptr.
 */
[[nodiscard]] inline auto read_native_dir(
    native_dir_handle_t handle) noexcept -> native_dirent_t* {
#ifdef _WIN32
    return ::_wreaddir(handle);
#else
    return ::readdir(handle);
#endif
}

/**
 * @brief Rewinds a native directory handle.
 *
 * @param handle Native directory handle.
 */
inline auto rewind_native_dir(native_dir_handle_t handle) noexcept -> void {
#ifdef _WIN32
    ::_wrewinddir(handle);
#else
    ::rewinddir(handle);
#endif
}

/**
 * @brief Converts a native directory entry name to UTF-8.
 *
 * @param entry Native directory entry.
 * @return Entry name in UTF-8 on success.
 */
[[nodiscard]] inline auto native_dirent_name_to_u8string(
    const native_dirent_t& entry) noexcept -> result<std::u8string> {
    const auto converted = from_native_path(native_path_view(entry.d_name));
    if (!converted.has_value()) {
        return std::unexpected(converted.error());
    }

    return std::u8string(converted->str());
}

} // namespace detail

/**
 * @brief Move-only directory stream handle.
 *
 * This type owns a native directory stream. It is closed automatically by the
 * destructor, but callers should use closedir when they need to observe close
 * errors explicitly.
 */
class dir {
public:
    /**
     * @brief Constructs an empty directory handle.
     */
    constexpr dir() noexcept = default;

    dir(const dir&) = delete;
    auto operator=(const dir&) -> dir& = delete;

    /**
     * @brief Move-constructs a directory handle.
     *
     * @param other Source directory handle.
     */
    constexpr dir(dir&& other) noexcept
        : handle_(other.handle_) {
        other.handle_ = nullptr;
    }

    /**
     * @brief Move-assigns a directory handle.
     *
     * The current handle is closed before taking ownership from the source.
     *
     * @param other Source directory handle.
     * @return Reference to this object.
     */
    auto operator=(dir&& other) noexcept -> dir& {
        if (this != &other) {
            (void)close();
            handle_ = other.handle_;
            other.handle_ = nullptr;
        }

        return *this;
    }

    /**
     * @brief Destroys the directory handle.
     *
     * Any close failure is ignored.
     */
    ~dir() {
        (void)close();
    }

    /**
     * @brief Constructs a directory handle from a native handle.
     *
     * @param handle Native directory handle.
     */
    explicit constexpr dir(detail::native_dir_handle_t handle) noexcept
        : handle_(handle) {}

    /**
     * @brief Closes the directory handle if it is open.
     *
     * This function is a no-op for an empty handle. After this function
     * returns, the directory object is empty regardless of whether the close
     * operation succeeded.
     *
     * @return Non-negative on success, negative on failure.
     */
    auto close() noexcept -> int {
        if (handle_ == nullptr) {
            return 0;
        }

        detail::native_dir_handle_t handle = handle_;
        handle_ = nullptr;
        return detail::close_native_dir(handle);
    }

    /**
     * @brief Returns whether the directory handle is open.
     *
     * @return true if open, otherwise false.
     */
    [[nodiscard]] constexpr auto is_open() const noexcept -> bool {
        return handle_ != nullptr;
    }

    /**
     * @brief Returns the native directory handle.
     *
     * @return Native directory handle.
     */
    [[nodiscard]] constexpr auto native_handle() const noexcept
        -> detail::native_dir_handle_t {
        return handle_;
    }

private:
    detail::native_dir_handle_t handle_ = nullptr;
};

/**
 * @brief Opens a directory stream.
 *
 * This function opens a directory and returns a move-only RAII handle. The path
 * is converted through XER's native path conversion before being passed to the
 * platform directory API.
 *
 * The result is a snapshot-like stream over directory entries. If the directory
 * contents are changed while it is being read, the observed behavior is
 * platform- and filesystem-dependent.
 *
 * @param dirname Target directory path.
 * @return Open directory handle on success.
 */
[[nodiscard]] inline auto opendir(const path& dirname) noexcept -> result<dir> {
    const auto native_dirname = to_native_path(dirname);
    if (!native_dirname.has_value()) {
        return std::unexpected(native_dirname.error());
    }

    errno = 0;
    detail::native_dir_handle_t handle = detail::open_native_dir(*native_dirname);
    if (handle == nullptr) {
        const int saved_errno = errno;
        if (saved_errno == 0) {
            return std::unexpected(make_error(error_t::io_error));
        }

        return std::unexpected(
            make_error(detail::dirent_errno_to_error_t(saved_errno)));
    }

    return dir(handle);
}

/**
 * @brief Closes a directory stream.
 *
 * This function closes the specified directory stream and resets it to the
 * empty state regardless of whether the close operation succeeds. Destroying a
 * dir object also closes it, but explicit closedir is useful when the caller
 * needs to observe close errors.
 *
 * @param directory Target directory stream.
 * @return Empty success value on success.
 */
[[nodiscard]] inline auto closedir(dir& directory) noexcept -> result<void> {
    if (directory.close() < 0) {
        const int saved_errno = errno;
        if (saved_errno == 0) {
            return std::unexpected(make_error(error_t::io_error));
        }

        return std::unexpected(
            make_error(detail::dirent_errno_to_error_t(saved_errno)));
    }

    return {};
}

/**
 * @brief Reads the next entry name from a directory stream.
 *
 * This function returns only the directory entry name. It does not return a
 * full path. The special entries "." and ".." are not filtered out.
 *
 * Entry order is platform- and filesystem-dependent. If the directory contents
 * are changed while it is being read, the observed behavior is also
 * platform- and filesystem-dependent.
 *
 * End of directory is reported as error_t::not_found. Other failures are
 * reported through xer::result in the usual way.
 *
 * @param directory Target directory stream.
 * @return Next entry name in UTF-8 on success.
 */
[[nodiscard]] inline auto readdir(dir& directory) noexcept -> result<std::u8string> {
    if (!directory.is_open()) {
        return std::unexpected(make_error(error_t::badf));
    }

    errno = 0;
    detail::native_dirent_t* entry = detail::read_native_dir(directory.native_handle());
    if (entry == nullptr) {
        const int saved_errno = errno;
        if (saved_errno == 0) {
            return std::unexpected(make_error(error_t::not_found));
        }

        return std::unexpected(
            make_error(detail::dirent_errno_to_error_t(saved_errno)));
    }

    return detail::native_dirent_name_to_u8string(*entry);
}

/**
 * @brief Rewinds a directory stream to the beginning.
 *
 * After this function succeeds, subsequent readdir calls start again from the
 * beginning of the directory stream. As with POSIX/PHP-style directory reads,
 * the entry order is not specified by XER.
 *
 * @param directory Target directory stream.
 * @return Empty success value on success.
 */
[[nodiscard]] inline auto rewinddir(dir& directory) noexcept -> result<void> {
    if (!directory.is_open()) {
        return std::unexpected(make_error(error_t::badf));
    }

    detail::rewind_native_dir(directory.native_handle());
    return {};
}

} // namespace xer

#endif /* XER_BITS_DIRECTORY_H_INCLUDED_ */

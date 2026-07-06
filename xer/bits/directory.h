/**
 * @file xer/bits/directory.h
 * @brief Directory stream operations.
 */

#pragma once

#ifndef XER_BITS_DIRECTORY_H_INCLUDED_
#define XER_BITS_DIRECTORY_H_INCLUDED_

#include <cerrno>
#include <expected>
#include <new>
#include <string>
#include <utility>

#ifdef _WIN32
#    include <xer/bits/windows.h>
#else
#    include <dirent.h>
#endif

#include <xer/error.h>
#include <xer/path.h>

namespace xer {

namespace detail {

#ifdef _WIN32

struct native_dirent_t {
    native_path_string d_name;
};

struct native_dir_handle {
    native_path_string pattern;
    HANDLE find_handle = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW find_data{};
    native_dirent_t current_entry{};
    int synthetic_index = 0;
    bool find_started = false;
    bool finished = false;
};

using native_dir_handle_t = native_dir_handle*;

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

#ifdef _WIN32

/**
 * @brief Converts a Win32 error code to errno.
 *
 * @param value Win32 error code.
 * @return Converted errno value.
 */
[[nodiscard]] inline auto win32_error_to_errno(DWORD value) noexcept -> int {
    switch (value) {
    case ERROR_SUCCESS:
        return 0;
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
    case ERROR_INVALID_DRIVE:
        return ENOENT;
    case ERROR_ACCESS_DENIED:
    case ERROR_SHARING_VIOLATION:
    case ERROR_LOCK_VIOLATION:
        return EACCES;
    case ERROR_NOT_ENOUGH_MEMORY:
    case ERROR_OUTOFMEMORY:
        return ENOMEM;
    case ERROR_INVALID_NAME:
    case ERROR_BAD_PATHNAME:
    case ERROR_DIRECTORY:
        return ENOTDIR;
    default:
        return EIO;
    }
}

/**
 * @brief Stores the current Win32 last error in errno.
 */
inline auto set_errno_from_last_win32_error() noexcept -> void {
    errno = win32_error_to_errno(::GetLastError());
}

/**
 * @brief Returns whether a native path ends with a directory separator.
 *
 * @param path Target native path.
 * @return true if path ends with a directory separator.
 */
[[nodiscard]] inline auto native_path_ends_with_separator(
    const native_path_string& path) noexcept -> bool {
    if (path.empty()) {
        return false;
    }

    const wchar_t ch = path.back();
    return ch == L'\\' || ch == L'/';
}

/**
 * @brief Makes a native FindFirstFile pattern for a directory.
 *
 * @param dirname Native directory path.
 * @return Search pattern.
 */
[[nodiscard]] inline auto make_native_dir_pattern(
    const native_path_string& dirname) -> native_path_string {
    native_path_string pattern = dirname;
    if (!native_path_ends_with_separator(pattern)) {
        pattern.push_back(L'\\');
    }

    pattern.push_back(L'*');
    return pattern;
}

#endif

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
    int result = 0;
    if (handle->find_handle != INVALID_HANDLE_VALUE) {
        if (::FindClose(handle->find_handle) == 0) {
            set_errno_from_last_win32_error();
            result = -1;
        }
    }

    delete handle;
    return result;
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
    const DWORD attributes = ::GetFileAttributesW(dirname.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        set_errno_from_last_win32_error();
        return nullptr;
    }
    if ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        errno = ENOTDIR;
        return nullptr;
    }

    native_dir_handle_t handle = new (std::nothrow) native_dir_handle;
    if (handle == nullptr) {
        errno = ENOMEM;
        return nullptr;
    }

    try {
        handle->pattern = make_native_dir_pattern(dirname);
    } catch (...) {
        delete handle;
        errno = ENOMEM;
        return nullptr;
    }

    return handle;
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
    if (handle == nullptr || handle->finished) {
        errno = 0;
        return nullptr;
    }

    if (handle->synthetic_index == 0) {
        handle->current_entry.d_name = L".";
        handle->synthetic_index = 1;
        errno = 0;
        return &handle->current_entry;
    }

    if (handle->synthetic_index == 1) {
        handle->current_entry.d_name = L"..";
        handle->synthetic_index = 2;
        errno = 0;
        return &handle->current_entry;
    }

    if (!handle->find_started) {
        handle->find_started = true;
        handle->find_handle = ::FindFirstFileExW(
            handle->pattern.c_str(),
            FindExInfoBasic,
            &handle->find_data,
            FindExSearchNameMatch,
            nullptr,
            0);
        if (handle->find_handle == INVALID_HANDLE_VALUE) {
            const DWORD error = ::GetLastError();
            handle->finished = true;
            if (error == ERROR_FILE_NOT_FOUND || error == ERROR_NO_MORE_FILES) {
                errno = 0;
            } else {
                errno = win32_error_to_errno(error);
            }
            return nullptr;
        }
    } else {
        if (::FindNextFileW(handle->find_handle, &handle->find_data) == 0) {
            const DWORD error = ::GetLastError();
            handle->finished = true;
            if (error == ERROR_NO_MORE_FILES) {
                errno = 0;
            } else {
                errno = win32_error_to_errno(error);
            }
            return nullptr;
        }
    }

    try {
        handle->current_entry.d_name = handle->find_data.cFileName;
    } catch (...) {
        errno = ENOMEM;
        return nullptr;
    }

    errno = 0;
    return &handle->current_entry;
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
    if (handle == nullptr) {
        return;
    }

    if (handle->find_handle != INVALID_HANDLE_VALUE) {
        (void)::FindClose(handle->find_handle);
        handle->find_handle = INVALID_HANDLE_VALUE;
    }

    handle->find_data = WIN32_FIND_DATAW{};
    handle->current_entry.d_name.clear();
    handle->synthetic_index = 0;
    handle->find_started = false;
    handle->finished = false;
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
 * End of directory is reported as error_t::end_of_file. Other failures are
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
            return std::unexpected(make_error(error_t::end_of_file));
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

#endif // XER_BITS_DIRECTORY_H_INCLUDED_

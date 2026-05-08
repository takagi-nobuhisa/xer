/**
 * @file xer/bits/file_entry.h
 * @brief File entry operations that do not depend on FILE.
 */

#pragma once

#ifndef XER_BITS_FILE_ENTRY_H_INCLUDED_
#define XER_BITS_FILE_ENTRY_H_INCLUDED_

#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <expected>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include <xer/bits/common.h>
#include <xer/bits/time_clock.h>
#include <xer/error.h>
#include <xer/path.h>

#ifdef _WIN32
#    include <io.h>
#    include <windows.h>
#else
#    include <cstdio>
#    include <fcntl.h>
#    include <sys/stat.h>
#    include <unistd.h>
#endif

namespace xer::detail {

#ifdef _WIN32

/**
 * @brief Converts a Windows system error code to xer::error_t.
 *
 * @param value Windows error code.
 * @return Converted error code.
 */
[[nodiscard]] inline auto win32_error_to_error_t(unsigned long value) noexcept -> error_t
{
    switch (value) {
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
    case ERROR_INVALID_DRIVE:
        return error_t::noent;

    case ERROR_ACCESS_DENIED:
    case ERROR_SHARING_VIOLATION:
    case ERROR_LOCK_VIOLATION:
        return error_t::acces;

    case ERROR_ALREADY_EXISTS:
    case ERROR_FILE_EXISTS:
        return error_t::exist;

    case ERROR_NOT_SAME_DEVICE:
        return error_t::xdev;

    case ERROR_DIR_NOT_EMPTY:
        return error_t::notempty;

    case ERROR_DIRECTORY:
        return error_t::isdir;

    case ERROR_WRITE_PROTECT:
        return error_t::rofs;

    case ERROR_BUSY:
        return error_t::busy;

    case ERROR_FILENAME_EXCED_RANGE:
        return error_t::nametoolong;

    case ERROR_INVALID_NAME:
        return error_t::inval;

    case ERROR_NOT_ENOUGH_MEMORY:
    case ERROR_OUTOFMEMORY:
        return error_t::nomem;

    default:
        return error_t::io_error;
    }
}

/**
 * @brief Closes a Windows handle if it is valid.
 *
 * @param handle Target handle.
 */
inline auto close_handle_if_valid(HANDLE handle) noexcept -> void
{
    if (handle != nullptr && handle != INVALID_HANDLE_VALUE) {
        ::CloseHandle(handle);
    }
}

/**
 * @brief Converts XER calendar time to a Windows FILETIME value.
 *
 * @param value Seconds since the POSIX epoch.
 * @return Converted FILETIME on success.
 */
[[nodiscard]] inline auto time_to_filetime(time_t value) noexcept -> result<FILETIME>
{
    constexpr long double windows_to_unix_epoch_100ns = 116444736000000000.0L;
    constexpr long double ticks_per_second = 10000000.0L;
    constexpr long double max_ticks =
        static_cast<long double>(std::numeric_limits<std::uint64_t>::max());

    if (!std::isfinite(value) || value < 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const long double ticks =
        (static_cast<long double>(value) * ticks_per_second) +
        windows_to_unix_epoch_100ns;

    if (ticks < 0.0L || ticks > max_ticks) {
        return std::unexpected(make_error(error_t::range));
    }

    const auto integer_ticks = static_cast<std::uint64_t>(std::llround(ticks));

    FILETIME result{};
    result.dwLowDateTime = static_cast<DWORD>(integer_ticks & 0xffffffffULL);
    result.dwHighDateTime = static_cast<DWORD>(integer_ticks >> 32U);
    return result;
}

/**
 * @brief Removes Windows extended path prefixes from a final path name.
 *
 * GetFinalPathNameByHandleW usually returns paths such as "\\?\C:\dir\file"
 * or "\\?\UNC\server\share\dir". XER path conversion expects ordinary native
 * path spelling, so this helper converts them back to "C:\dir\file" and
 * "\\server\share\dir".
 *
 * @param value Native Windows final path.
 * @return Native Windows path without the extended prefix.
 */
[[nodiscard]] inline auto strip_windows_final_path_prefix(std::wstring value)
    -> std::wstring
{
    constexpr std::wstring_view drive_prefix = L"\\\\?\\";
    constexpr std::wstring_view unc_prefix = L"\\\\?\\UNC\\";

    if (value.starts_with(unc_prefix)) {
        value.erase(0, unc_prefix.size());
        value.insert(0, L"\\\\");
        return value;
    }

    if (value.starts_with(drive_prefix)) {
        value.erase(0, drive_prefix.size());
        return value;
    }

    return value;
}

#else

/**
 * @brief Converts errno to xer::error_t.
 *
 * @param value errno value.
 * @return Converted error code.
 */
[[nodiscard]] inline auto errno_to_error_t(int value) noexcept -> error_t
{
    return static_cast<error_t>(value);
}

/**
 * @brief Closes a POSIX file descriptor.
 *
 * @param fd Target file descriptor.
 */
inline auto close_fd_if_valid(int fd) noexcept -> void
{
    if (fd >= 0) {
        ::close(fd);
    }
}

/**
 * @brief Converts XER calendar time to a POSIX timespec value.
 *
 * @param value Seconds since the POSIX epoch.
 * @return Converted timespec on success.
 */
[[nodiscard]] inline auto time_to_timespec(time_t value) noexcept -> result<timespec>
{
    if (!std::isfinite(value) || value < 0) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    const long double whole = std::floor(static_cast<long double>(value));
    const long double fraction = static_cast<long double>(value) - whole;

    if (whole > static_cast<long double>(std::numeric_limits<::time_t>::max())) {
        return std::unexpected(make_error(error_t::range));
    }

    auto seconds = static_cast<::time_t>(whole);
    auto nanoseconds = static_cast<long>(std::llround(fraction * 1000000000.0L));

    if (nanoseconds >= 1000000000L) {
        ++seconds;
        nanoseconds -= 1000000000L;
    }

    timespec result{};
    result.tv_sec = seconds;
    result.tv_nsec = nanoseconds;
    return result;
}

#endif

} // namespace xer::detail

namespace xer {

/**
 * @brief Checks whether a file system entry exists.
 *
 * This function is intended as a simple PHP-style predicate. It returns
 * false when the path cannot be converted to a native path or when the
 * platform check fails.
 *
 * The result is only a snapshot-like approximation. The file system state may
 * change immediately after this function returns, so a later operation on the
 * same path may still fail.
 *
 * @param filename Target path.
 * @return true if the file system entry appears to exist, otherwise false.
 */
[[nodiscard]] inline auto file_exists(const path& filename) -> bool
{
    const auto native_filename = to_native_path(filename);
    if (!native_filename.has_value()) {
        return false;
    }

#ifdef _WIN32
    return ::GetFileAttributesW(native_filename->c_str()) != INVALID_FILE_ATTRIBUTES;
#else
    struct stat status {};
    return ::stat(native_filename->c_str(), &status) == 0;
#endif
}

/**
 * @brief Checks whether a path appears to refer to a regular file.
 *
 * This function is intended as a simple PHP-style predicate. It returns
 * false when the path cannot be converted to a native path or when the
 * platform check fails.
 *
 * The result is only a snapshot-like approximation. The file system state may
 * change immediately after this function returns, so a later operation on the
 * same path may still fail. Symbolic links and other special entries follow
 * the behavior of the underlying platform check.
 *
 * @param filename Target path.
 * @return true if the path appears to refer to a regular file, otherwise false.
 */
[[nodiscard]] inline auto is_file(const path& filename) -> bool
{
    const auto native_filename = to_native_path(filename);
    if (!native_filename.has_value()) {
        return false;
    }

#ifdef _WIN32
    const DWORD attributes = ::GetFileAttributesW(native_filename->c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return false;
    }

    return (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
#else
    struct stat status {};
    if (::stat(native_filename->c_str(), &status) != 0) {
        return false;
    }

    return S_ISREG(status.st_mode);
#endif
}

/**
 * @brief Checks whether a path appears to refer to a directory.
 *
 * This function is intended as a simple PHP-style predicate. It returns
 * false when the path cannot be converted to a native path or when the
 * platform check fails.
 *
 * The result is only a snapshot-like approximation. The file system state may
 * change immediately after this function returns, so a later operation on the
 * same path may still fail. Symbolic links and other special entries follow
 * the behavior of the underlying platform check.
 *
 * @param filename Target path.
 * @return true if the path appears to refer to a directory, otherwise false.
 */
[[nodiscard]] inline auto is_dir(const path& filename) -> bool
{
    const auto native_filename = to_native_path(filename);
    if (!native_filename.has_value()) {
        return false;
    }

#ifdef _WIN32
    const DWORD attributes = ::GetFileAttributesW(native_filename->c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return false;
    }

    return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
    struct stat status {};
    if (::stat(native_filename->c_str(), &status) != 0) {
        return false;
    }

    return S_ISDIR(status.st_mode);
#endif
}

/**
 * @brief Checks whether the specified path appears to be readable.
 *
 * This function is intended as a simple PHP-style predicate. It returns
 * false when the path cannot be converted to a native path or when the
 * platform check fails.
 *
 * The result is only a snapshot-like approximation. It does not guarantee
 * that a subsequent open or read operation will succeed, because file
 * permissions, ACLs, locks, and file system state may change after this
 * function returns.
 *
 * @param filename Target path.
 * @return true if the path appears to be readable, otherwise false.
 */
[[nodiscard]] inline auto is_readable(const path& filename) -> bool
{
    const auto native_filename = to_native_path(filename);
    if (!native_filename.has_value()) {
        return false;
    }

#ifdef _WIN32
    return ::_waccess(native_filename->c_str(), 4) == 0;
#else
    return ::access(native_filename->c_str(), R_OK) == 0;
#endif
}

/**
 * @brief Checks whether the specified path appears to be writable.
 *
 * This function is intended as a simple PHP-style predicate. It returns
 * false when the path cannot be converted to a native path or when the
 * platform check fails.
 *
 * The result is only a snapshot-like approximation. It does not guarantee
 * that a subsequent open or write operation will succeed, because file
 * permissions, ACLs, locks, and file system state may change after this
 * function returns.
 *
 * @param filename Target path.
 * @return true if the path appears to be writable, otherwise false.
 */
[[nodiscard]] inline auto is_writable(const path& filename) -> bool
{
    const auto native_filename = to_native_path(filename);
    if (!native_filename.has_value()) {
        return false;
    }

#ifdef _WIN32
    return ::_waccess(native_filename->c_str(), 2) == 0;
#else
    return ::access(native_filename->c_str(), W_OK) == 0;
#endif
}

/**
 * @brief Changes the current working directory.
 *
 * This function changes the process-wide current working directory. The effect
 * is global to the process, so callers should be careful when using it in
 * programs that have multiple components or threads that also depend on the
 * current directory.
 *
 * @param target New current working directory.
 * @return Empty success value on success.
 */
[[nodiscard]] inline auto chdir(const path& target) -> result<void>
{
    const auto native_target = to_native_path(target);
    if (!native_target.has_value()) {
        return std::unexpected(native_target.error());
    }

#ifdef _WIN32
    if (::SetCurrentDirectoryW(native_target->c_str()) == 0) {
        return std::unexpected(
            make_error(detail::win32_error_to_error_t(::GetLastError())));
    }
#else
    if (::chdir(native_target->c_str()) != 0) {
        return std::unexpected(make_error(detail::errno_to_error_t(errno)));
    }
#endif

    return {};
}

/**
 * @brief Gets the current working directory.
 *
 * The returned path is converted into XER's UTF-8 path representation and uses
 * '/' as the internal separator. The current working directory is process-wide,
 * so the returned value is only a snapshot of the state at the time of the
 * call.
 *
 * @return Current working directory on success.
 */
[[nodiscard]] inline auto getcwd() -> result<path>
{
#ifdef _WIN32
    const DWORD required_size = ::GetCurrentDirectoryW(0, nullptr);
    if (required_size == 0) {
        return std::unexpected(
            make_error(detail::win32_error_to_error_t(::GetLastError())));
    }

    std::wstring buffer(required_size, L'\0');
    const DWORD written_size = ::GetCurrentDirectoryW(required_size, buffer.data());
    if (written_size == 0) {
        return std::unexpected(
            make_error(detail::win32_error_to_error_t(::GetLastError())));
    }

    if (written_size >= required_size) {
        return std::unexpected(make_error(error_t::range));
    }

    buffer.resize(written_size);

    const auto converted = from_native_path(std::wstring_view(buffer));
    if (!converted.has_value()) {
        return std::unexpected(converted.error());
    }

    return *converted;
#else
    std::vector<char> buffer(256);

    for (;;) {
        errno = 0;
        if (::getcwd(buffer.data(), buffer.size()) != nullptr) {
            const auto converted = from_native_path(std::string_view(buffer.data()));
            if (!converted.has_value()) {
                return std::unexpected(converted.error());
            }

            return *converted;
        }

        if (errno != ERANGE) {
            return std::unexpected(make_error(detail::errno_to_error_t(errno)));
        }

        if (buffer.size() > buffer.max_size() / 2) {
            return std::unexpected(make_error(error_t::length_error));
        }

        buffer.resize(buffer.size() * 2);
    }
#endif
}

/**
 * @brief Gets the canonicalized absolute path of an existing file system entry.
 *
 * This function queries the actual file system through the platform path
 * canonicalization mechanism. The target must exist. Relative path components
 * and symbolic links are resolved according to the underlying platform
 * behavior.
 *
 * The returned path is converted into XER's UTF-8 path representation and uses
 * '/' as the internal separator.
 *
 * @param filename Target path.
 * @return Canonicalized absolute path on success.
 */
[[nodiscard]] inline auto realpath(const path& filename) -> result<path>
{
#ifdef _WIN32
    const auto native_filename = to_native_path(filename);
    if (!native_filename.has_value()) {
        return std::unexpected(native_filename.error());
    }

    const HANDLE handle = ::CreateFileW(
        native_filename->c_str(),
        0,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        nullptr);

    if (handle == INVALID_HANDLE_VALUE) {
        return std::unexpected(
            make_error(detail::win32_error_to_error_t(::GetLastError())));
    }

    const DWORD required_size = ::GetFinalPathNameByHandleW(
        handle,
        nullptr,
        0,
        FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);

    if (required_size == 0) {
        const DWORD error = ::GetLastError();
        detail::close_handle_if_valid(handle);
        return std::unexpected(
            make_error(detail::win32_error_to_error_t(error)));
    }

    std::wstring buffer(required_size + 1, L'\0');
    const DWORD written_size = ::GetFinalPathNameByHandleW(
        handle,
        buffer.data(),
        static_cast<DWORD>(buffer.size()),
        FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);

    if (written_size == 0) {
        const DWORD error = ::GetLastError();
        detail::close_handle_if_valid(handle);
        return std::unexpected(
            make_error(detail::win32_error_to_error_t(error)));
    }

    detail::close_handle_if_valid(handle);

    if (written_size >= buffer.size()) {
        return std::unexpected(make_error(error_t::range));
    }

    buffer.resize(written_size);
    buffer = detail::strip_windows_final_path_prefix(std::move(buffer));

    const auto converted = from_native_path(std::wstring_view(buffer));
    if (!converted.has_value()) {
        return std::unexpected(converted.error());
    }

    return *converted;
#else
    const auto native_filename = to_native_path(filename);
    if (!native_filename.has_value()) {
        return std::unexpected(native_filename.error());
    }

    errno = 0;
    std::unique_ptr<char, decltype(&std::free)> resolved(
        ::realpath(native_filename->c_str(), nullptr),
        &std::free);

    if (!resolved) {
        return std::unexpected(make_error(detail::errno_to_error_t(errno)));
    }

    const auto converted = from_native_path(std::string_view(resolved.get()));
    if (!converted.has_value()) {
        return std::unexpected(converted.error());
    }

    return *converted;
#endif
}


/**
 * @brief Changes file access and modification times.
 *
 * If the target does not exist, this function creates an empty regular file.
 * Negative mtime means that the current time is used. Negative atime means
 * that the resolved mtime is also used as the access time. Non-finite time
 * values are rejected as invalid arguments.
 *
 * @param filename Target path.
 * @param mtime Modification time, or a negative value to use the current time.
 * @param atime Access time, or a negative value to use the resolved mtime.
 * @return Empty success value on success.
 */
[[nodiscard]] inline auto touch(
    const path& filename,
    time_t mtime = -1,
    time_t atime = -1) -> result<void>
{
    if (!std::isfinite(mtime) || !std::isfinite(atime)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (mtime < 0) {
        const auto now = time();
        if (!now.has_value()) {
            return std::unexpected(now.error());
        }

        mtime = *now;
    }

    if (atime < 0) {
        atime = mtime;
    }

    const auto native_filename = to_native_path(filename);
    if (!native_filename.has_value()) {
        return std::unexpected(native_filename.error());
    }

#ifdef _WIN32
    const auto access_time = detail::time_to_filetime(atime);
    if (!access_time.has_value()) {
        return std::unexpected(access_time.error());
    }

    const auto write_time = detail::time_to_filetime(mtime);
    if (!write_time.has_value()) {
        return std::unexpected(write_time.error());
    }

    const HANDLE handle = ::CreateFileW(
        native_filename->c_str(),
        FILE_WRITE_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
        nullptr);

    if (handle == INVALID_HANDLE_VALUE) {
        return std::unexpected(
            make_error(detail::win32_error_to_error_t(::GetLastError())));
    }

    if (::SetFileTime(handle, nullptr, &*access_time, &*write_time) == 0) {
        const DWORD error = ::GetLastError();
        detail::close_handle_if_valid(handle);
        return std::unexpected(make_error(detail::win32_error_to_error_t(error)));
    }

    detail::close_handle_if_valid(handle);
    return {};
#else
    const auto access_time = detail::time_to_timespec(atime);
    if (!access_time.has_value()) {
        return std::unexpected(access_time.error());
    }

    const auto write_time = detail::time_to_timespec(mtime);
    if (!write_time.has_value()) {
        return std::unexpected(write_time.error());
    }

    const struct timespec times[2] = {
        *access_time,
        *write_time,
    };

    if (::utimensat(AT_FDCWD, native_filename->c_str(), times, 0) == 0) {
        return {};
    }

    if (errno != ENOENT) {
        return std::unexpected(make_error(detail::errno_to_error_t(errno)));
    }

    const int fd = ::open(native_filename->c_str(), O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd < 0) {
        if (errno == EEXIST) {
            if (::utimensat(AT_FDCWD, native_filename->c_str(), times, 0) == 0) {
                return {};
            }
        }

        return std::unexpected(make_error(detail::errno_to_error_t(errno)));
    }

    if (::close(fd) != 0) {
        return std::unexpected(make_error(detail::errno_to_error_t(errno)));
    }

    if (::utimensat(AT_FDCWD, native_filename->c_str(), times, 0) != 0) {
        return std::unexpected(make_error(detail::errno_to_error_t(errno)));
    }

    return {};
#endif
}

/**
 * @brief Removes a file system entry that must not be a directory.
 *
 * This function removes a regular file or a symbolic link itself.
 * Directories are not removed by this function.
 *
 * @param target Target path.
 * @return Empty success value on success.
 */
[[nodiscard]] inline auto remove(const path& target) -> result<void>
{
    const auto native_target = to_native_path(target);
    if (!native_target.has_value()) {
        return std::unexpected(native_target.error());
    }

#ifdef _WIN32
    if (::DeleteFileW(native_target->c_str()) == 0) {
        return std::unexpected(
            make_error(detail::win32_error_to_error_t(::GetLastError())));
    }
#else
    if (::unlink(native_target->c_str()) != 0) {
        return std::unexpected(make_error(detail::errno_to_error_t(errno)));
    }
#endif

    return {};
}

/**
 * @brief Renames or moves a file system entry.
 *
 * If the destination already exists, this function fails.
 * On Windows, file moves across volumes are allowed when the underlying
 * platform API allows them. Directory moves across volumes may fail.
 *
 * @param from Source path.
 * @param to Destination path.
 * @return Empty success value on success.
 */
[[nodiscard]] inline auto rename(
    const path& from,
    const path& to) -> result<void>
{
    const auto native_from = to_native_path(from);
    if (!native_from.has_value()) {
        return std::unexpected(native_from.error());
    }

    const auto native_to = to_native_path(to);
    if (!native_to.has_value()) {
        return std::unexpected(native_to.error());
    }

#ifdef _WIN32
    if (::MoveFileW(native_from->c_str(), native_to->c_str()) == 0) {
        return std::unexpected(
            make_error(detail::win32_error_to_error_t(::GetLastError())));
    }
#else
    if (::rename(native_from->c_str(), native_to->c_str()) != 0) {
        return std::unexpected(make_error(detail::errno_to_error_t(errno)));
    }
#endif

    return {};
}

/**
 * @brief Creates a single directory.
 *
 * This function does not create missing parent directories.
 *
 * @param target Target directory path.
 * @return Empty success value on success.
 */
[[nodiscard]] inline auto mkdir(const path& target) -> result<void>
{
    const auto native_target = to_native_path(target);
    if (!native_target.has_value()) {
        return std::unexpected(native_target.error());
    }

#ifdef _WIN32
    if (::CreateDirectoryW(native_target->c_str(), nullptr) == 0) {
        return std::unexpected(
            make_error(detail::win32_error_to_error_t(::GetLastError())));
    }
#else
    if (::mkdir(native_target->c_str(), 0777) != 0) {
        return std::unexpected(make_error(detail::errno_to_error_t(errno)));
    }
#endif

    return {};
}

/**
 * @brief Removes an empty directory.
 *
 * @param target Target directory path.
 * @return Empty success value on success.
 */
[[nodiscard]] inline auto rmdir(const path& target) -> result<void>
{
    const auto native_target = to_native_path(target);
    if (!native_target.has_value()) {
        return std::unexpected(native_target.error());
    }

#ifdef _WIN32
    if (::RemoveDirectoryW(native_target->c_str()) == 0) {
        return std::unexpected(
            make_error(detail::win32_error_to_error_t(::GetLastError())));
    }
#else
    if (::rmdir(native_target->c_str()) != 0) {
        return std::unexpected(make_error(detail::errno_to_error_t(errno)));
    }
#endif

    return {};
}

/**
 * @brief Copies a regular file.
 *
 * This function fails if the source is a directory or if the destination
 * already exists. Stream context resources and URL wrappers are unsupported.
 *
 * @param from Source file path.
 * @param to Destination file path.
 * @return Empty success value on success.
 */
[[nodiscard]] inline auto copy(
    const path& from,
    const path& to) -> result<void>
{
    const auto native_from = to_native_path(from);
    if (!native_from.has_value()) {
        return std::unexpected(native_from.error());
    }

    const auto native_to = to_native_path(to);
    if (!native_to.has_value()) {
        return std::unexpected(native_to.error());
    }

#ifdef _WIN32
    if (::CopyFileW(native_from->c_str(), native_to->c_str(), TRUE) == 0) {
        return std::unexpected(
            make_error(detail::win32_error_to_error_t(::GetLastError())));
    }

    return {};
#else
    struct stat source_stat {};
    if (::stat(native_from->c_str(), &source_stat) != 0) {
        return std::unexpected(make_error(detail::errno_to_error_t(errno)));
    }

    if (S_ISDIR(source_stat.st_mode)) {
        return std::unexpected(make_error(error_t::isdir));
    }

    const int source_fd = ::open(native_from->c_str(), O_RDONLY);
    if (source_fd < 0) {
        return std::unexpected(make_error(detail::errno_to_error_t(errno)));
    }

    const mode_t create_mode = static_cast<mode_t>(source_stat.st_mode & 07777);
    const int destination_fd =
        ::open(native_to->c_str(), O_WRONLY | O_CREAT | O_EXCL, create_mode);

    if (destination_fd < 0) {
        const int saved_errno = errno;
        ::close(source_fd);
        return std::unexpected(make_error(detail::errno_to_error_t(saved_errno)));
    }

    char buffer[8192];
    bool copy_failed = false;
    error<void> copy_error = make_error(error_t::io_error);

    for (;;) {
        const ssize_t read_size = ::read(source_fd, buffer, sizeof(buffer));
        if (read_size == 0) {
            break;
        }

        if (read_size < 0) {
            copy_failed = true;
            copy_error = make_error(detail::errno_to_error_t(errno));
            break;
        }

        ssize_t written_total = 0;
        while (written_total < read_size) {
            const ssize_t written_size = ::write(
                destination_fd,
                buffer + written_total,
                static_cast<std::size_t>(read_size - written_total));

            if (written_size < 0) {
                copy_failed = true;
                copy_error = make_error(detail::errno_to_error_t(errno));
                break;
            }

            written_total += written_size;
        }

        if (copy_failed) {
            break;
        }
    }

    if (!copy_failed) {
        if (::fchmod(destination_fd, create_mode) != 0) {
            copy_failed = true;
            copy_error = make_error(detail::errno_to_error_t(errno));
        }
    }

#    if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200809L)
    if (!copy_failed) {
        const struct timespec times[2] = {
            source_stat.st_atim,
            source_stat.st_mtim,
        };

        if (::futimens(destination_fd, times) != 0) {
            copy_failed = true;
            copy_error = make_error(detail::errno_to_error_t(errno));
        }
    }
#    endif

    {
        int saved_errno = 0;

        if (::close(destination_fd) != 0 && !copy_failed) {
            saved_errno = errno;
            copy_failed = true;
            copy_error = make_error(detail::errno_to_error_t(saved_errno));
        }

        if (::close(source_fd) != 0 && !copy_failed) {
            saved_errno = errno;
            copy_failed = true;
            copy_error = make_error(detail::errno_to_error_t(saved_errno));
        }
    }

    if (copy_failed) {
        ::unlink(native_to->c_str());
        return std::unexpected(copy_error);
    }

    return {};
#endif
}

} // namespace xer

#endif /* XER_BITS_FILE_ENTRY_H_INCLUDED_ */

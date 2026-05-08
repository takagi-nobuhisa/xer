/**
 * @file xer/bits/file_stat.h
 * @brief File status functions for paths and file-backed streams.
 */

#pragma once

#ifndef XER_BITS_FILE_STAT_H_INCLUDED_
#define XER_BITS_FILE_STAT_H_INCLUDED_

#include <cstdio>
#include <cstdint>
#include <expected>
#include <limits>
#include <type_traits>

#include <xer/bits/time_types.h>

#include <xer/bits/binary_stream.h>
#include <xer/bits/text_stream.h>
#include <xer/bits/to_native_handle.h>
#include <xer/error.h>
#include <xer/path.h>

#ifdef _WIN32
#    include <io.h>
#    include <windows.h>
#else
#    include <cerrno>
#    include <sys/stat.h>
#    include <unistd.h>
#endif

namespace xer {

/**
 * @brief File status information.
 *
 * This structure follows the naming style of PHP stat/lstat results and the
 * traditional POSIX stat structure, but it is a normalized XER structure.
 * The exact meaning and availability of each field are platform-dependent.
 *
 * Time values are seconds since the POSIX epoch. Sub-second precision is not
 * exposed. On Windows, ctime is based on the file creation time because
 * Windows does not expose POSIX inode-change time through the APIs used here.
 */
struct stat {
    std::uintmax_t dev{};
    std::uintmax_t ino{};
    std::uintmax_t mode{};
    std::uintmax_t nlink{};
    std::uintmax_t uid{};
    std::uintmax_t gid{};
    std::uintmax_t rdev{};
    std::uintmax_t size{};
    std::int64_t atime{};
    std::int64_t mtime{};
    std::int64_t ctime{};
    std::uintmax_t blksize{};
    std::uintmax_t blocks{};
};

namespace detail {

#ifdef _WIN32

[[nodiscard]] inline auto stat_win32_error_to_error_t(unsigned long value) noexcept
    -> error_t {
    switch (value) {
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
    case ERROR_INVALID_DRIVE:
        return error_t::noent;

    case ERROR_ACCESS_DENIED:
    case ERROR_SHARING_VIOLATION:
    case ERROR_LOCK_VIOLATION:
        return error_t::acces;

    case ERROR_INVALID_HANDLE:
        return error_t::badf;

    case ERROR_FILENAME_EXCED_RANGE:
        return error_t::nametoolong;

    case ERROR_INVALID_NAME:
    case ERROR_INVALID_PARAMETER:
        return error_t::inval;

    case ERROR_NOT_ENOUGH_MEMORY:
    case ERROR_OUTOFMEMORY:
        return error_t::nomem;

    default:
        return error_t::io_error;
    }
}

[[nodiscard]] constexpr auto make_uint64_from_high_low(
    unsigned long high,
    unsigned long low) noexcept -> std::uint64_t {
    return (static_cast<std::uint64_t>(high) << 32U) |
           static_cast<std::uint64_t>(low);
}

[[nodiscard]] constexpr auto filetime_to_unix_seconds(
    const FILETIME& value) noexcept -> std::int64_t {
    constexpr std::uint64_t windows_to_unix_epoch_100ns = 116444736000000000ULL;
    constexpr std::uint64_t ticks_per_second = 10000000ULL;

    const std::uint64_t ticks = make_uint64_from_high_low(
        value.dwHighDateTime,
        value.dwLowDateTime);

    if (ticks < windows_to_unix_epoch_100ns) {
        return 0;
    }

    return static_cast<std::int64_t>(
        (ticks - windows_to_unix_epoch_100ns) / ticks_per_second);
}

[[nodiscard]] constexpr auto windows_attributes_to_mode(
    unsigned long attributes) noexcept -> std::uintmax_t {
    std::uintmax_t mode = 0;

    if ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        mode |= 0040000u;
        mode |= 0111u;
    }
    else {
        mode |= 0100000u;
    }

    mode |= 0444u;
    if ((attributes & FILE_ATTRIBUTE_READONLY) == 0) {
        mode |= 0222u;
    }

    return mode;
}

[[nodiscard]] inline auto convert_win32_file_information(
    const BY_HANDLE_FILE_INFORMATION& value) noexcept -> stat {
    stat result{};

    result.dev = value.dwVolumeSerialNumber;
    result.ino = make_uint64_from_high_low(value.nFileIndexHigh, value.nFileIndexLow);
    result.mode = windows_attributes_to_mode(value.dwFileAttributes);
    result.nlink = value.nNumberOfLinks;
    result.uid = 0;
    result.gid = 0;
    result.rdev = result.dev;
    result.size = make_uint64_from_high_low(value.nFileSizeHigh, value.nFileSizeLow);
    result.atime = filetime_to_unix_seconds(value.ftLastAccessTime);
    result.mtime = filetime_to_unix_seconds(value.ftLastWriteTime);
    result.ctime = filetime_to_unix_seconds(value.ftCreationTime);
    result.blksize = 0;
    result.blocks = 0;

    return result;
}

[[nodiscard]] inline auto stat_native_handle(HANDLE handle) noexcept -> result<stat> {
    if (handle == INVALID_HANDLE_VALUE || handle == nullptr) {
        return std::unexpected(make_error(error_t::badf));
    }

    BY_HANDLE_FILE_INFORMATION info{};
    if (::GetFileInformationByHandle(handle, &info) == 0) {
        return std::unexpected(
            make_error(stat_win32_error_to_error_t(::GetLastError())));
    }

    return convert_win32_file_information(info);
}

[[nodiscard]] inline auto stat_native_file(std::FILE* file) noexcept -> result<stat> {
    if (file == nullptr) {
        return std::unexpected(make_error(error_t::badf));
    }

    const int fd = ::_fileno(file);
    if (fd < 0) {
        return std::unexpected(make_error(error_t::badf));
    }

    const intptr_t os_handle = ::_get_osfhandle(fd);
    if (os_handle == -1) {
        return std::unexpected(make_error(error_t::badf));
    }

    return stat_native_handle(reinterpret_cast<HANDLE>(os_handle));
}

[[nodiscard]] inline auto stat_native_path(
    const native_path_string& filename) noexcept -> result<stat> {
    HANDLE const handle = ::CreateFileW(
        filename.c_str(),
        FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        nullptr);

    if (handle == INVALID_HANDLE_VALUE) {
        return std::unexpected(
            make_error(stat_win32_error_to_error_t(::GetLastError())));
    }

    const auto result = stat_native_handle(handle);
    ::CloseHandle(handle);
    return result;
}

[[nodiscard]] inline auto lstat_native_path(
    const native_path_string& filename) noexcept -> result<stat> {
    HANDLE const handle = ::CreateFileW(
        filename.c_str(),
        FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
        nullptr);

    if (handle == INVALID_HANDLE_VALUE) {
        return std::unexpected(
            make_error(stat_win32_error_to_error_t(::GetLastError())));
    }

    const auto result = stat_native_handle(handle);
    ::CloseHandle(handle);
    return result;
}

[[nodiscard]] inline auto filesize_native_path(
    const native_path_string& filename) noexcept -> result<std::uintmax_t> {
    WIN32_FILE_ATTRIBUTE_DATA data{};
    if (::GetFileAttributesExW(filename.c_str(), GetFileExInfoStandard, &data) == 0) {
        return std::unexpected(
            make_error(stat_win32_error_to_error_t(::GetLastError())));
    }

    return make_uint64_from_high_low(data.nFileSizeHigh, data.nFileSizeLow);
}

#else

[[nodiscard]] inline auto stat_errno_to_error_t(int value) noexcept -> error_t {
    return static_cast<error_t>(value);
}

template<class T>
[[nodiscard]] constexpr auto stat_to_uintmax(T value) noexcept -> std::uintmax_t {
    if constexpr (std::is_signed_v<T>) {
        if (value < 0) {
            return 0;
        }
    }

    return static_cast<std::uintmax_t>(value);
}

[[nodiscard]] inline auto convert_posix_stat(const struct ::stat& value) noexcept -> stat {
    stat result{};

    result.dev = stat_to_uintmax(value.st_dev);
    result.ino = stat_to_uintmax(value.st_ino);
    result.mode = stat_to_uintmax(value.st_mode);
    result.nlink = stat_to_uintmax(value.st_nlink);
    result.uid = stat_to_uintmax(value.st_uid);
    result.gid = stat_to_uintmax(value.st_gid);
    result.rdev = stat_to_uintmax(value.st_rdev);
    result.size = stat_to_uintmax(value.st_size);
    result.atime = static_cast<std::int64_t>(value.st_atime);
    result.mtime = static_cast<std::int64_t>(value.st_mtime);
    result.ctime = static_cast<std::int64_t>(value.st_ctime);

#    ifdef _STATBUF_ST_BLKSIZE
    result.blksize = stat_to_uintmax(value.st_blksize);
#    elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    result.blksize = stat_to_uintmax(value.st_blksize);
#    else
    result.blksize = 0;
#    endif

#    ifdef _STATBUF_ST_BLOCKS
    result.blocks = stat_to_uintmax(value.st_blocks);
#    elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    result.blocks = stat_to_uintmax(value.st_blocks);
#    else
    result.blocks = 0;
#    endif

    return result;
}

[[nodiscard]] inline auto stat_native_file(std::FILE* file) noexcept -> result<stat> {
    if (file == nullptr) {
        return std::unexpected(make_error(error_t::badf));
    }

    const int fd = ::fileno(file);
    if (fd < 0) {
        return std::unexpected(make_error(error_t::badf));
    }

    struct ::stat status {};
    if (::fstat(fd, &status) != 0) {
        return std::unexpected(make_error(stat_errno_to_error_t(errno)));
    }

    return convert_posix_stat(status);
}

[[nodiscard]] inline auto stat_native_path(
    const native_path_string& filename) noexcept -> result<stat> {
    struct ::stat status {};
    if (::stat(filename.c_str(), &status) != 0) {
        return std::unexpected(make_error(stat_errno_to_error_t(errno)));
    }

    return convert_posix_stat(status);
}

[[nodiscard]] inline auto lstat_native_path(
    const native_path_string& filename) noexcept -> result<stat> {
    struct ::stat status {};
    if (::lstat(filename.c_str(), &status) != 0) {
        return std::unexpected(make_error(stat_errno_to_error_t(errno)));
    }

    return convert_posix_stat(status);
}

[[nodiscard]] inline auto filesize_native_path(
    const native_path_string& filename) noexcept -> result<std::uintmax_t> {
    struct ::stat status {};
    if (::stat(filename.c_str(), &status) != 0) {
        return std::unexpected(make_error(stat_errno_to_error_t(errno)));
    }

    return stat_to_uintmax(status.st_size);
}

#endif

} // namespace detail

/**
 * @brief Gets status information for a file-backed binary stream.
 *
 * This function obtains status information for the file entry currently
 * associated with the stream. It requires a stream that can expose a native
 * FILE handle through to_native_handle. Memory-backed streams, socket-backed
 * streams, and other non-file streams may fail.
 *
 * The returned structure is normalized for XER, but the exact meaning of each
 * field is platform-dependent. On Windows, some POSIX-like fields such as uid,
 * gid, blksize, and blocks may be zero because there is no direct equivalent in
 * the native API used here.
 *
 * @param stream Target stream.
 * @return File status information on success.
 */
[[nodiscard]] inline auto fstat(binary_stream& stream) noexcept -> result<stat> {
    const auto file = to_native_handle(stream);
    if (!file.has_value()) {
        return std::unexpected(file.error());
    }

    const auto result = detail::stat_native_file(*file);
    if (!result.has_value()) {
        stream.set_error(true);
    }

    return result;
}

/**
 * @brief Gets status information for a file-backed text stream.
 *
 * This function obtains status information for the file entry currently
 * associated with the stream. It requires a stream that can expose a native
 * FILE handle through to_native_handle. String-backed streams, socket-backed
 * streams, and other non-file streams may fail.
 *
 * The returned structure is normalized for XER, but the exact meaning of each
 * field is platform-dependent. On Windows, some POSIX-like fields such as uid,
 * gid, blksize, and blocks may be zero because there is no direct equivalent in
 * the native API used here.
 *
 * This function does not read, write, or reposition the native FILE handle, so
 * it does not disturb text_stream buffering or encoding state.
 *
 * @param stream Target stream.
 * @return File status information on success.
 */
[[nodiscard]] inline auto fstat(text_stream& stream) noexcept -> result<stat> {
    const auto file = to_native_handle(stream);
    if (!file.has_value()) {
        return std::unexpected(file.error());
    }

    const auto result = detail::stat_native_file(*file);
    if (!result.has_value()) {
        stream.set_error(true);
    }

    return result;
}

/**
 * @brief Gets status information for a path without following symbolic links.
 *
 * On POSIX-like systems, this function uses lstat and obtains information about
 * the symbolic link itself when the path refers to a symbolic link. On Windows,
 * the result is normalized from native file information obtained with reparse
 * point traversal disabled where possible, and may not match POSIX lstat
 * semantics exactly.
 *
 * The returned structure is a snapshot-like result. The filesystem state may
 * change immediately after this function returns.
 *
 * @param filename Target path.
 * @return File status information on success.
 */
[[nodiscard]] inline auto lstat(const path& filename) noexcept -> result<stat> {
    const auto native_filename = to_native_path(filename);
    if (!native_filename.has_value()) {
        return std::unexpected(native_filename.error());
    }

    return detail::lstat_native_path(*native_filename);
}

/**
 * @brief Gets the size of a file system entry.
 *
 * This function returns the size in bytes using the platform's ordinary path
 * status operation. Unlike lstat, it is intended as a PHP-style file-size
 * helper and may follow symbolic links when the platform's normal stat-like
 * operation does so.
 *
 * The returned value is a snapshot-like result. The filesystem state may change
 * immediately after this function returns. For directories and special files,
 * the meaning of the returned size is platform-dependent.
 *
 * @param filename Target path.
 * @return File size in bytes on success.
 */
[[nodiscard]] inline auto filesize(const path& filename) noexcept
    -> result<std::uintmax_t> {
    const auto native_filename = to_native_path(filename);
    if (!native_filename.has_value()) {
        return std::unexpected(native_filename.error());
    }

    return detail::filesize_native_path(*native_filename);
}

/**
 * @brief Gets the last access time of a file system entry.
 *
 * This function returns the atime field from the platform's ordinary path
 * status operation. Unlike lstat, it is intended as a PHP-style file-time
 * helper and may follow symbolic links when the platform's normal stat-like
 * operation does so.
 *
 * The returned value is seconds since the POSIX epoch. Sub-second precision is
 * not exposed. The value is a snapshot-like result. The filesystem state may
 * change immediately after this function returns.
 *
 * @param filename Target path.
 * @return Last access time on success.
 */
[[nodiscard]] inline auto fileatime(const path& filename) noexcept -> result<time_t> {
    const auto native_filename = to_native_path(filename);
    if (!native_filename.has_value()) {
        return std::unexpected(native_filename.error());
    }

    const auto status = detail::stat_native_path(*native_filename);
    if (!status.has_value()) {
        return std::unexpected(status.error());
    }

    return static_cast<time_t>(status->atime);
}

/**
 * @brief Gets the last modification time of a file system entry.
 *
 * This function returns the mtime field from the platform's ordinary path
 * status operation. Unlike lstat, it is intended as a PHP-style file-time
 * helper and may follow symbolic links when the platform's normal stat-like
 * operation does so.
 *
 * The returned value is seconds since the POSIX epoch. Sub-second precision is
 * not exposed. The value is a snapshot-like result. The filesystem state may
 * change immediately after this function returns.
 *
 * @param filename Target path.
 * @return Last modification time on success.
 */
[[nodiscard]] inline auto filemtime(const path& filename) noexcept -> result<time_t> {
    const auto native_filename = to_native_path(filename);
    if (!native_filename.has_value()) {
        return std::unexpected(native_filename.error());
    }

    const auto status = detail::stat_native_path(*native_filename);
    if (!status.has_value()) {
        return std::unexpected(status.error());
    }

    return static_cast<time_t>(status->mtime);
}

/**
 * @brief Gets the ctime field of a file system entry.
 *
 * This function returns the ctime field from the platform's ordinary path
 * status operation. The meaning of ctime is the same as xer::stat::ctime; see
 * the documentation comment for xer::stat for the platform-specific details.
 *
 * The returned value is seconds since the POSIX epoch. Sub-second precision is
 * not exposed. The value is a snapshot-like result. The filesystem state may
 * change immediately after this function returns.
 *
 * @param filename Target path.
 * @return ctime value on success.
 */
[[nodiscard]] inline auto filectime(const path& filename) noexcept -> result<time_t> {
    const auto native_filename = to_native_path(filename);
    if (!native_filename.has_value()) {
        return std::unexpected(native_filename.error());
    }

    const auto status = detail::stat_native_path(*native_filename);
    if (!status.has_value()) {
        return std::unexpected(status.error());
    }

    return static_cast<time_t>(status->ctime);
}

} // namespace xer

#endif /* XER_BITS_FILE_STAT_H_INCLUDED_ */

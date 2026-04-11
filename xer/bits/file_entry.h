/**
 * @file xer/bits/file_entry.h
 * @brief File entry operations that do not depend on FILE.
 */

#pragma once

#ifndef XER_BITS_FILE_ENTRY_H_INCLUDED_
#define XER_BITS_FILE_ENTRY_H_INCLUDED_

#include <cerrno>
#include <expected>

#include <xer/bits/common.h>
#include <xer/error.h>
#include <xer/path.h>

#ifdef _WIN32
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
[[nodiscard]] inline error_t win32_error_to_error_t(unsigned long value) noexcept {
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
        return error_t::runtime_error;
    }
}

#else

/**
 * @brief Converts errno to xer::error_t.
 *
 * @param value errno value.
 * @return Converted error code.
 */
[[nodiscard]] inline error_t errno_to_error_t(int value) noexcept {
    return static_cast<error_t>(value);
}

/**
 * @brief Closes a POSIX file descriptor.
 *
 * @param fd Target file descriptor.
 */
inline void close_fd_if_valid(int fd) noexcept {
    if (fd >= 0) {
        ::close(fd);
    }
}

#endif

} // namespace xer::detail

namespace xer {

/**
 * @brief Removes a file system entry that must not be a directory.
 *
 * This function removes a regular file or a symbolic link itself.
 * Directories are not removed by this function.
 *
 * @param target Target path.
 * @return Empty success value on success.
 */
[[nodiscard]] inline std::expected<void, error<void>> remove(const path& target) {
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
[[nodiscard]] inline std::expected<void, error<void>> rename(
    const path& from,
    const path& to) {
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
[[nodiscard]] inline std::expected<void, error<void>> mkdir(const path& target) {
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
[[nodiscard]] inline std::expected<void, error<void>> rmdir(const path& target) {
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
[[nodiscard]] inline std::expected<void, error<void>> copy(
    const path& from,
    const path& to) {
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
    error<void> copy_error = make_error(error_t::runtime_error);

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

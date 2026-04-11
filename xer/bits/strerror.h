/**
 * @file xer/bits/strerror.h
 * @brief Defines strerror-related functions for xer::error_t.
 */

#pragma once

#ifndef XER_BITS_STRERROR_H_INCLUDED_
#define XER_BITS_STRERROR_H_INCLUDED_

#include <expected>
#include <string_view>
#include <type_traits>

#include <xer/bits/common.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Converts an enum value to its underlying integer type.
 *
 * @tparam Enum Enumeration type.
 * @param value Enumeration value.
 * @return Underlying integer value.
 */
template<typename Enum>
[[nodiscard]] constexpr std::underlying_type_t<Enum> to_underlying(Enum value) noexcept {
    return static_cast<std::underlying_type_t<Enum>>(value);
}

/**
 * @brief Creates an unexpected error result.
 *
 * @param code Error code.
 * @return Unexpected error result.
 */
[[nodiscard]] constexpr std::unexpected<error<void>> make_unexpected_error(error_t code) noexcept {
    return std::unexpected(make_error(code));
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Returns the English error message for an error code.
 *
 * Positive values follow the conventional strerror-style English messages
 * for errno-compatible errors. Negative values use the corresponding
 * error_t enumerator name with underscores replaced by spaces.
 *
 * @param code Error code.
 * @return English error message on success.
 */
[[nodiscard]] constexpr std::expected<std::u8string_view, error<void>> strerror(error_t code) noexcept {
    switch (code) {
    case error_t::perm:
        return u8"Operation not permitted";
    case error_t::noent:
        return u8"No such file or directory";
    case error_t::srch:
        return u8"No such process";
    case error_t::intr:
        return u8"Interrupted system call";
    case error_t::io:
        return u8"Input/output error";
    case error_t::nxio:
        return u8"No such device or address";
    case error_t::toobig:
        return u8"Argument list too long";
    case error_t::noexec:
        return u8"Exec format error";
    case error_t::badf:
        return u8"Bad file descriptor";
    case error_t::child:
        return u8"No child processes";
    case error_t::again:
        return u8"Resource temporarily unavailable";
    case error_t::nomem:
        return u8"Cannot allocate memory";
    case error_t::acces:
        return u8"Permission denied";
    case error_t::fault:
        return u8"Bad address";
    case error_t::busy:
        return u8"Device or resource busy";
    case error_t::exist:
        return u8"File exists";
    case error_t::xdev:
        return u8"Invalid cross-device link";
    case error_t::nodev:
        return u8"No such device";
    case error_t::notdir:
        return u8"Not a directory";
    case error_t::isdir:
        return u8"Is a directory";
    case error_t::inval:
        return u8"Invalid argument";
    case error_t::nfile:
        return u8"Too many open files in system";
    case error_t::mfile:
        return u8"Too many open files";
    case error_t::notty:
        return u8"Inappropriate ioctl for device";
    case error_t::fbig:
        return u8"File too large";
    case error_t::nospc:
        return u8"No space left on device";
    case error_t::spipe:
        return u8"Illegal seek";
    case error_t::rofs:
        return u8"Read-only file system";
    case error_t::mlink:
        return u8"Too many links";
    case error_t::pipe:
        return u8"Broken pipe";
    case error_t::dom:
        return u8"Numerical argument out of domain";
    case error_t::range:
        return u8"Numerical result out of range";

#ifdef EDEADLK
    case error_t::deadlk:
        return u8"Resource deadlock avoided";
#endif
#ifdef ENAMETOOLONG
    case error_t::nametoolong:
        return u8"File name too long";
#endif
#ifdef ENOLCK
    case error_t::nolck:
        return u8"No locks available";
#endif
#ifdef ENOSYS
    case error_t::nosys:
        return u8"Function not implemented";
#endif
#ifdef ENOTEMPTY
    case error_t::notempty:
        return u8"Directory not empty";
#endif
#ifdef EILSEQ
    case error_t::ilseq:
        return u8"Invalid or incomplete multibyte or wide character";
#endif

    case static_cast<error_t>(0):
        return u8"Undefined error: 0";

    case error_t::logic_error:
        return u8"logic error";
    case error_t::domain_error:
        return u8"domain error";
    case error_t::invalid_argument:
        return u8"invalid argument";
    case error_t::length_error:
        return u8"length error";
    case error_t::out_of_range:
        return u8"out of range";
    case error_t::runtime_error:
        return u8"runtime error";
    case error_t::range_error:
        return u8"range error";
    case error_t::overflow_error:
        return u8"overflow error";
    case error_t::underflow_error:
        return u8"underflow error";
    case error_t::not_found:
        return u8"not found";
    case error_t::divide_by_zero:
        return u8"divide by zero";
    case error_t::user_error:
        return u8"user error";
    }

    return detail::make_unexpected_error(error_t::not_found);
}

/**
 * @brief Returns the xer::error_t enumerator name for an error code.
 *
 * @param code Error code.
 * @return Enumerator name on success.
 */
[[nodiscard]] constexpr std::expected<std::u8string_view, error<void>> get_error_name(error_t code) noexcept {
    switch (code) {
    case error_t::perm:
        return u8"perm";
    case error_t::noent:
        return u8"noent";
    case error_t::srch:
        return u8"srch";
    case error_t::intr:
        return u8"intr";
    case error_t::io:
        return u8"io";
    case error_t::nxio:
        return u8"nxio";
    case error_t::toobig:
        return u8"toobig";
    case error_t::noexec:
        return u8"noexec";
    case error_t::badf:
        return u8"badf";
    case error_t::child:
        return u8"child";
    case error_t::again:
        return u8"again";
    case error_t::nomem:
        return u8"nomem";
    case error_t::acces:
        return u8"acces";
    case error_t::fault:
        return u8"fault";
    case error_t::busy:
        return u8"busy";
    case error_t::exist:
        return u8"exist";
    case error_t::xdev:
        return u8"xdev";
    case error_t::nodev:
        return u8"nodev";
    case error_t::notdir:
        return u8"notdir";
    case error_t::isdir:
        return u8"isdir";
    case error_t::inval:
        return u8"inval";
    case error_t::nfile:
        return u8"nfile";
    case error_t::mfile:
        return u8"mfile";
    case error_t::notty:
        return u8"notty";
    case error_t::fbig:
        return u8"fbig";
    case error_t::nospc:
        return u8"nospc";
    case error_t::spipe:
        return u8"spipe";
    case error_t::rofs:
        return u8"rofs";
    case error_t::mlink:
        return u8"mlink";
    case error_t::pipe:
        return u8"pipe";
    case error_t::dom:
        return u8"dom";
    case error_t::range:
        return u8"range";

#ifdef EDEADLK
    case error_t::deadlk:
        return u8"deadlk";
#endif
#ifdef ENAMETOOLONG
    case error_t::nametoolong:
        return u8"nametoolong";
#endif
#ifdef ENOLCK
    case error_t::nolck:
        return u8"nolck";
#endif
#ifdef ENOSYS
    case error_t::nosys:
        return u8"nosys";
#endif
#ifdef ENOTEMPTY
    case error_t::notempty:
        return u8"notempty";
#endif
#ifdef EILSEQ
    case error_t::ilseq:
        return u8"ilseq";
#endif

    case static_cast<error_t>(0):
        return u8"0";

    case error_t::logic_error:
        return u8"logic_error";
    case error_t::domain_error:
        return u8"domain_error";
    case error_t::invalid_argument:
        return u8"invalid_argument";
    case error_t::length_error:
        return u8"length_error";
    case error_t::out_of_range:
        return u8"out_of_range";
    case error_t::runtime_error:
        return u8"runtime_error";
    case error_t::range_error:
        return u8"range_error";
    case error_t::overflow_error:
        return u8"overflow_error";
    case error_t::underflow_error:
        return u8"underflow_error";
    case error_t::not_found:
        return u8"not_found";
    case error_t::divide_by_zero:
        return u8"divide_by_zero";
    case error_t::user_error:
        return u8"user_error";
    }

    return detail::make_unexpected_error(error_t::not_found);
}

/**
 * @brief Returns the errno macro name for an errno-compatible error code.
 *
 * For xer-specific negative error codes, this function fails with
 * error_t::not_found.
 *
 * @param code Error code.
 * @return errno macro name on success.
 */
[[nodiscard]] constexpr std::expected<std::u8string_view, error<void>> get_errno_name(error_t code) noexcept {
    switch (code) {
    case error_t::perm:
        return u8"EPERM";
    case error_t::noent:
        return u8"ENOENT";
    case error_t::srch:
        return u8"ESRCH";
    case error_t::intr:
        return u8"EINTR";
    case error_t::io:
        return u8"EIO";
    case error_t::nxio:
        return u8"ENXIO";
    case error_t::toobig:
        return u8"E2BIG";
    case error_t::noexec:
        return u8"ENOEXEC";
    case error_t::badf:
        return u8"EBADF";
    case error_t::child:
        return u8"ECHILD";
    case error_t::again:
        return u8"EAGAIN";
    case error_t::nomem:
        return u8"ENOMEM";
    case error_t::acces:
        return u8"EACCES";
    case error_t::fault:
        return u8"EFAULT";
    case error_t::busy:
        return u8"EBUSY";
    case error_t::exist:
        return u8"EEXIST";
    case error_t::xdev:
        return u8"EXDEV";
    case error_t::nodev:
        return u8"ENODEV";
    case error_t::notdir:
        return u8"ENOTDIR";
    case error_t::isdir:
        return u8"EISDIR";
    case error_t::inval:
        return u8"EINVAL";
    case error_t::nfile:
        return u8"ENFILE";
    case error_t::mfile:
        return u8"EMFILE";
    case error_t::notty:
        return u8"ENOTTY";
    case error_t::fbig:
        return u8"EFBIG";
    case error_t::nospc:
        return u8"ENOSPC";
    case error_t::spipe:
        return u8"ESPIPE";
    case error_t::rofs:
        return u8"EROFS";
    case error_t::mlink:
        return u8"EMLINK";
    case error_t::pipe:
        return u8"EPIPE";
    case error_t::dom:
        return u8"EDOM";
    case error_t::range:
        return u8"ERANGE";

#ifdef EDEADLK
    case error_t::deadlk:
        return u8"EDEADLK";
#endif
#ifdef ENAMETOOLONG
    case error_t::nametoolong:
        return u8"ENAMETOOLONG";
#endif
#ifdef ENOLCK
    case error_t::nolck:
        return u8"ENOLCK";
#endif
#ifdef ENOSYS
    case error_t::nosys:
        return u8"ENOSYS";
#endif
#ifdef ENOTEMPTY
    case error_t::notempty:
        return u8"ENOTEMPTY";
#endif
#ifdef EILSEQ
    case error_t::ilseq:
        return u8"EILSEQ";
#endif

    case static_cast<error_t>(0):
        return u8"0";

    case error_t::logic_error:
    case error_t::domain_error:
    case error_t::invalid_argument:
    case error_t::length_error:
    case error_t::out_of_range:
    case error_t::runtime_error:
    case error_t::range_error:
    case error_t::overflow_error:
    case error_t::underflow_error:
    case error_t::not_found:
    case error_t::divide_by_zero:
    case error_t::user_error:
        return detail::make_unexpected_error(error_t::not_found);
    }

    return detail::make_unexpected_error(error_t::not_found);
}

} // namespace xer

#endif /* XER_BITS_STRERROR_H_INCLUDED_ */

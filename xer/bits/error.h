/**
 * @file xer/bits/error.h
 * @brief Defines error_t, error, result, and make_error.
 */

#pragma once

#ifndef XER_BITS_ERROR_H_INCLUDED_
#define XER_BITS_ERROR_H_INCLUDED_

#include <cerrno>
#include <concepts>
#include <expected>
#include <cstdint>
#include <ostream>
#include <string>
#include <string_view>
#include <source_location>
#include <type_traits>
#include <utility>

#include <xer/bits/common.h>

namespace xer {

/**
 * @brief Represents XER error codes.
 *
 * Positive values are intended to be compatible with errno values.
 * Negative values are reserved for XER-specific or extended errors.
 */
enum class error_t : std::int32_t {
    perm = EPERM,
    noent = ENOENT,
    srch = ESRCH,
    intr = EINTR,
    io = EIO,
    nxio = ENXIO,
    toobig = E2BIG,
    noexec = ENOEXEC,
    badf = EBADF,
    child = ECHILD,
    again = EAGAIN,
    nomem = ENOMEM,
    acces = EACCES,
    fault = EFAULT,
    busy = EBUSY,
    exist = EEXIST,
    xdev = EXDEV,
    nodev = ENODEV,
    notdir = ENOTDIR,
    isdir = EISDIR,
    inval = EINVAL,
    nfile = ENFILE,
    mfile = EMFILE,
    notty = ENOTTY,
    fbig = EFBIG,
    nospc = ENOSPC,
    spipe = ESPIPE,
    rofs = EROFS,
    mlink = EMLINK,
    pipe = EPIPE,
    dom = EDOM,
    range = ERANGE,

#ifdef EDEADLK
    deadlk = EDEADLK,
#endif
#ifdef ENAMETOOLONG
    nametoolong = ENAMETOOLONG,
#endif
#ifdef ENOLCK
    nolck = ENOLCK,
#endif
#ifdef ENOSYS
    nosys = ENOSYS,
#endif
#ifdef ENOTEMPTY
    notempty = ENOTEMPTY,
#endif
#ifdef EILSEQ
    ilseq = EILSEQ,
#endif

    logic_error = -1,
    domain_error = -2,
    invalid_argument = -3,
    length_error = -4,
    out_of_range = -5,
    runtime_error = -6,
    range_error = -7,
    overflow_error = -8,
    underflow_error = -9,
    io_error = -10,
    encoding_error = -11,
    not_found = -12,
    divide_by_zero = -13,
    network_error = -14,
    process_error = -15,

    user_error = -1000,
};

template<class Detail = void>
struct error;

/**
 * @brief Standard XER result type.
 *
 * @tparam T Success value type.
 * @tparam Detail Additional error detail type.
 */
template<class T, class Detail = void>
using result = std::expected<T, error<Detail>>;

namespace detail {

/**
 * @brief Storage for class detail types.
 */
template<class Detail>
struct error_detail_storage_class : Detail {
    template<class T>
        requires(std::constructible_from<Detail, T&&>)
    constexpr explicit error_detail_storage_class(T&& value) noexcept(
        std::is_nothrow_constructible_v<Detail, T&&>)
        : Detail(std::forward<T>(value)) {}
};

/**
 * @brief Storage for non-class detail types.
 */
template<class Detail>
struct error_detail_storage_scalar {
    Detail detail;

    template<class T>
        requires(std::constructible_from<Detail, T&&>)
    constexpr explicit error_detail_storage_scalar(T&& value) noexcept(
        std::is_nothrow_constructible_v<Detail, T&&>)
        : detail(std::forward<T>(value)) {}
};

template<class Detail>
using error_detail_storage = std::conditional_t<
    std::is_class_v<Detail>,
    error_detail_storage_class<Detail>,
    error_detail_storage_scalar<Detail>>;

} // namespace detail

/**
 * @brief Specialization for errors without detail.
 */
template<>
struct error<void> {
    /**
     * @brief Constructs an error object.
     *
     * @param code_ Error code.
     * @param location_ Source location where the error was created.
     */
    constexpr explicit error(
        error_t code_,
        std::source_location location_) noexcept
        : code(code_),
          location(location_) {}

    error_t code;
    std::source_location location;
};

/**
 * @brief Represents an error with additional detail.
 *
 * For class detail types, this type inherits from Detail.
 * For non-class detail types, this type contains a direct data member named detail.
 *
 * @tparam Detail Detail type.
 */
template<class Detail>
struct error : error<void>, detail::error_detail_storage<Detail> {
    static_assert(!std::same_as<Detail, void>);

    using detail_storage_type = detail::error_detail_storage<Detail>;

    /**
     * @brief Constructs an error object.
     *
     * @tparam T Type passed to Detail construction.
     * @param code_ Error code.
     * @param value_ Value forwarded to Detail construction.
     * @param location_ Source location where the error was created.
     */
    template<class T>
        requires(std::constructible_from<Detail, T&&>)
    constexpr explicit error(
        error_t code_,
        T&& value_,
        std::source_location location_) noexcept(
        std::is_nothrow_constructible_v<detail_storage_type, T&&>)
        : error<void>(code_, location_),
          detail_storage_type(std::forward<T>(value_)) {}
};

/**
 * @brief Creates an error without additional detail.
 *
 * @param code Error code.
 * @param location Source location where the error is created.
 * @return Created error object.
 */
[[nodiscard]] constexpr error<void> make_error(
    error_t code,
    std::source_location location = std::source_location::current()) noexcept {
    return error<void>(code, location);
}

/**
 * @brief Creates an error with additional detail.
 *
 * @tparam Detail Detail type.
 * @tparam T Type passed to Detail construction.
 * @param code Error code.
 * @param value Value forwarded to Detail construction.
 * @param location Source location where the error is created.
 * @return Created error object.
 */
template<class Detail, class T>
    requires(!std::same_as<Detail, void> && std::constructible_from<Detail, T&&>)
[[nodiscard]] constexpr error<Detail> make_error(
    error_t code,
    T&& value,
    std::source_location location = std::source_location::current()) noexcept(
    noexcept(error<Detail>(code, std::forward<T>(value), location))) {
    return error<Detail>(code, std::forward<T>(value), location);
}

namespace detail {

/**
 * @brief Returns the XER error enumerator name for stream output.
 *
 * This helper is intentionally placed in error.h instead of using strerror.h
 * because strerror.h depends on error.h. The names are diagnostic spellings and
 * are therefore kept close to error_t.
 *
 * @param value Error code.
 * @return Enumerator name.
 */
[[nodiscard]] constexpr auto error_name_for_ostream(error_t value) noexcept -> std::string_view
{
    switch (value) {
    case error_t::perm:
        return "perm";
    case error_t::noent:
        return "noent";
    case error_t::srch:
        return "srch";
    case error_t::intr:
        return "intr";
    case error_t::io:
        return "io";
    case error_t::nxio:
        return "nxio";
    case error_t::toobig:
        return "toobig";
    case error_t::noexec:
        return "noexec";
    case error_t::badf:
        return "badf";
    case error_t::child:
        return "child";
    case error_t::again:
        return "again";
    case error_t::nomem:
        return "nomem";
    case error_t::acces:
        return "acces";
    case error_t::fault:
        return "fault";
    case error_t::busy:
        return "busy";
    case error_t::exist:
        return "exist";
    case error_t::xdev:
        return "xdev";
    case error_t::nodev:
        return "nodev";
    case error_t::notdir:
        return "notdir";
    case error_t::isdir:
        return "isdir";
    case error_t::inval:
        return "inval";
    case error_t::nfile:
        return "nfile";
    case error_t::mfile:
        return "mfile";
    case error_t::notty:
        return "notty";
    case error_t::fbig:
        return "fbig";
    case error_t::nospc:
        return "nospc";
    case error_t::spipe:
        return "spipe";
    case error_t::rofs:
        return "rofs";
    case error_t::mlink:
        return "mlink";
    case error_t::pipe:
        return "pipe";
    case error_t::dom:
        return "dom";
    case error_t::range:
        return "range";

#ifdef EDEADLK
    case error_t::deadlk:
        return "deadlk";
#endif
#ifdef ENAMETOOLONG
    case error_t::nametoolong:
        return "nametoolong";
#endif
#ifdef ENOLCK
    case error_t::nolck:
        return "nolck";
#endif
#ifdef ENOSYS
    case error_t::nosys:
        return "nosys";
#endif
#ifdef ENOTEMPTY
    case error_t::notempty:
        return "notempty";
#endif
#ifdef EILSEQ
    case error_t::ilseq:
        return "ilseq";
#endif

    case static_cast<error_t>(0):
        return "0";
    case error_t::logic_error:
        return "logic_error";
    case error_t::domain_error:
        return "domain_error";
    case error_t::invalid_argument:
        return "invalid_argument";
    case error_t::length_error:
        return "length_error";
    case error_t::out_of_range:
        return "out_of_range";
    case error_t::runtime_error:
        return "runtime_error";
    case error_t::range_error:
        return "range_error";
    case error_t::overflow_error:
        return "overflow_error";
    case error_t::underflow_error:
        return "underflow_error";
    case error_t::io_error:
        return "io_error";
    case error_t::encoding_error:
        return "encoding_error";
    case error_t::not_found:
        return "not_found";
    case error_t::divide_by_zero:
        return "divide_by_zero";
    case error_t::network_error:
        return "network_error";
    case error_t::process_error:
        return "process_error";
    case error_t::user_error:
        return "user_error";
    }

    return "unknown";
}

/**
 * @brief Writes a UTF-8 string view to a narrow stream as bytes.
 *
 * @param os Output stream.
 * @param value UTF-8 string view.
 * @return Output stream.
 */
inline auto write_ostream_value(std::ostream& os, std::u8string_view value) -> std::ostream&
{
    return os.write(reinterpret_cast<const char*>(value.data()), static_cast<std::streamsize>(value.size()));
}

/**
 * @brief Writes a UTF-8 string to a narrow stream as bytes.
 *
 * @param os Output stream.
 * @param value UTF-8 string.
 * @return Output stream.
 */
inline auto write_ostream_value(std::ostream& os, const std::u8string& value) -> std::ostream&
{
    return write_ostream_value(os, std::u8string_view(value));
}

/**
 * @brief Writes a UTF-8 C string to a narrow stream as bytes.
 *
 * @param os Output stream.
 * @param value UTF-8 C string.
 * @return Output stream.
 */
inline auto write_ostream_value(std::ostream& os, const char8_t* value) -> std::ostream&
{
    if (value == nullptr) {
        return os << "(null)";
    }

    return write_ostream_value(os, std::u8string_view(value));
}

/**
 * @brief Writes a UTF-8 C string to a narrow stream as bytes.
 *
 * @param os Output stream.
 * @param value UTF-8 C string.
 * @return Output stream.
 */
inline auto write_ostream_value(std::ostream& os, char8_t* value) -> std::ostream&
{
    return write_ostream_value(os, static_cast<const char8_t*>(value));
}

/**
 * @brief Writes an arbitrary stream-insertable value.
 *
 * @tparam T Value type.
 * @param os Output stream.
 * @param value Value.
 * @return Output stream.
 */
template<class T>
inline auto write_ostream_value(std::ostream& os, const T& value) -> std::ostream&
{
    return os << value;
}

/**
 * @brief Returns the detail value stored in xer::error.
 *
 * @tparam Detail Detail type.
 * @param value Error object.
 * @return Detail value reference.
 */
template<class Detail>
[[nodiscard]] constexpr auto get_error_detail(const error<Detail>& value) noexcept -> const Detail&
{
    if constexpr (std::is_class_v<Detail>) {
        return static_cast<const Detail&>(value);
    } else {
        return value.detail;
    }
}

} // namespace detail

/**
 * @brief Writes an error code to a narrow stream.
 *
 * @param os Output stream.
 * @param value Error code.
 * @return Output stream.
 */
inline auto operator<<(std::ostream& os, error_t value) -> std::ostream&
{
    return os << detail::error_name_for_ostream(value);
}

/**
 * @brief Writes an error without detail to a narrow stream.
 *
 * @param os Output stream.
 * @param value Error object.
 * @return Output stream.
 */
inline auto operator<<(std::ostream& os, const error<void>& value) -> std::ostream&
{
    return os << "xer::error{code=" << value.code << "}";
}

/**
 * @brief Writes an error with detail to a narrow stream.
 *
 * @tparam Detail Detail type.
 * @param os Output stream.
 * @param value Error object.
 * @return Output stream.
 */
template<class Detail>
inline auto operator<<(std::ostream& os, const error<Detail>& value) -> std::ostream&
{
    os << "xer::error{code=" << value.code << ", detail=";
    detail::write_ostream_value(os, detail::get_error_detail(value));
    return os << "}";
}

/**
 * @brief Writes a non-void result to a narrow stream.
 *
 * @tparam T Success value type.
 * @tparam Detail Error detail type.
 * @param os Output stream.
 * @param value Result object.
 * @return Output stream.
 */
template<class T, class Detail>
    requires(!std::same_as<T, void>)
inline auto operator<<(std::ostream& os, const std::expected<T, error<Detail>>& value) -> std::ostream&
{
    if (value.has_value()) {
        os << "xer::result{value=";
        detail::write_ostream_value(os, *value);
        return os << "}";
    }

    return os << "xer::result{error=" << value.error() << "}";
}

/**
 * @brief Writes a void result to a narrow stream.
 *
 * @tparam Detail Error detail type.
 * @param os Output stream.
 * @param value Result object.
 * @return Output stream.
 */
template<class Detail>
inline auto operator<<(std::ostream& os, const std::expected<void, error<Detail>>& value) -> std::ostream&
{
    if (value.has_value()) {
        return os << "xer::result{value=void}";
    }

    return os << "xer::result{error=" << value.error() << "}";
}

} // namespace xer

#endif

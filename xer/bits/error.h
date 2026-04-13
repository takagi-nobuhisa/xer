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

} // namespace xer

#endif

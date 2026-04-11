/**
 * @file xer/bits/time_clock.h
 * @brief Time acquisition and elapsed-time utilities.
 */

#pragma once

#ifndef XER_BITS_TIME_CLOCK_H_INCLUDED_
#define XER_BITS_TIME_CLOCK_H_INCLUDED_

#include <ctime>
#include <expected>
#include <limits>

#include <xer/bits/common.h>
#include <xer/bits/time_types.h>
#include <xer/error.h>

namespace xer {

/**
 * @brief Processor time type.
 *
 * This type follows the implementation-defined processor time type used by the
 * underlying C library.
 */
using clock_t = std::clock_t;

/**
 * @brief Gets the current calendar time.
 *
 * The returned value is measured in seconds since 1970-01-01 00:00:00 UTC.
 * The practical target resolution is microsecond-level, although the exact
 * resolution depends on the underlying platform.
 *
 * @return The current calendar time on success.
 * @return An error with @ref error_t::runtime_error on failure.
 */
[[nodiscard]] inline std::expected<time_t, error<void>> time() noexcept {
#if defined(TIME_UTC)
    std::timespec ts {};

    if (std::timespec_get(&ts, TIME_UTC) != TIME_UTC) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    if (ts.tv_sec < 0 || ts.tv_nsec < 0 || ts.tv_nsec >= 1000000000L) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    return static_cast<time_t>(ts.tv_sec) +
           (static_cast<time_t>(ts.tv_nsec) / 1000000000.0);
#else
    const std::time_t now = std::time(nullptr);

    if (now == static_cast<std::time_t>(-1)) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    if (now < static_cast<std::time_t>(0)) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    return static_cast<time_t>(now);
#endif
}

/**
 * @brief Gets the processor time consumed by the program.
 *
 * This function follows the behavior of C's `clock()`. The unit is
 * implementation-defined and should be interpreted together with
 * `CLOCKS_PER_SEC`.
 *
 * @return The processor time on success.
 * @return An error with @ref error_t::runtime_error on failure.
 */
[[nodiscard]] inline std::expected<clock_t, error<void>> clock() noexcept {
    const std::clock_t value = std::clock();

    if (value == static_cast<std::clock_t>(-1)) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    return value;
}

/**
 * @brief Computes the difference between two calendar times.
 *
 * The result is equivalent to `left - right` and is expressed in seconds.
 *
 * @param left Left operand.
 * @param right Right operand.
 * @return Difference in seconds.
 */
[[nodiscard]] constexpr double difftime(time_t left, time_t right) noexcept {
    return left - right;
}

} // namespace xer

#endif /* XER_BITS_TIME_CLOCK_H_INCLUDED_ */

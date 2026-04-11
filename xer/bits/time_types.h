/**
 * @file xer/bits/time_types.h
 * @brief Time-related fundamental type definitions.
 */

#pragma once

#ifndef XER_BITS_TIME_TYPES_H_INCLUDED_
#define XER_BITS_TIME_TYPES_H_INCLUDED_

#include <xer/bits/common.h>

namespace xer {

/**
 * @brief Calendar time value measured in seconds since the Unix epoch.
 *
 * XER defines the epoch as 1970-01-01 00:00:00 UTC and represents calendar
 * time as a floating-point number of seconds. The integer part represents whole
 * seconds, and the fractional part represents sub-second time.
 *
 * The practical target resolution is microsecond-level, but the actual
 * representation is still a `double`.
 */
using time_t = double;

/**
 * @brief Broken-down calendar time with microsecond precision.
 *
 * This type follows the layout and meaning of C's `struct tm` and adds
 * @ref tm_microsec to preserve the fractional part of @ref time_t.
 *
 * Field meanings are the same as in C:
 * - `tm_sec`: seconds after the minute
 * - `tm_min`: minutes after the hour
 * - `tm_hour`: hours since midnight
 * - `tm_mday`: day of the month
 * - `tm_mon`: months since January in the range `0..11`
 * - `tm_year`: years since 1900
 * - `tm_wday`: days since Sunday in the range `0..6`
 * - `tm_yday`: days since January 1 in the range `0..365`
 * - `tm_isdst`: daylight saving time flag
 * - `tm_microsec`: microseconds in the range `0..999999`
 */
struct tm {
    /**
     * @brief Seconds after the minute.
     */
    int tm_sec;

    /**
     * @brief Minutes after the hour.
     */
    int tm_min;

    /**
     * @brief Hours since midnight.
     */
    int tm_hour;

    /**
     * @brief Day of the month.
     */
    int tm_mday;

    /**
     * @brief Months since January in the range `0..11`.
     */
    int tm_mon;

    /**
     * @brief Years since 1900.
     */
    int tm_year;

    /**
     * @brief Days since Sunday in the range `0..6`.
     */
    int tm_wday;

    /**
     * @brief Days since January 1 in the range `0..365`.
     */
    int tm_yday;

    /**
     * @brief Daylight saving time flag.
     *
     * The meaning follows C's `struct tm`.
     */
    int tm_isdst;

    /**
     * @brief Microseconds after the current second.
     *
     * Valid values are in the range `0..999999`.
     */
    int tm_microsec;

    /**
     * @brief Constructs a zero-initialized broken-down time.
     */
    constexpr tm() noexcept
        : tm_sec(0),
          tm_min(0),
          tm_hour(0),
          tm_mday(0),
          tm_mon(0),
          tm_year(0),
          tm_wday(0),
          tm_yday(0),
          tm_isdst(0),
          tm_microsec(0) {
    }
};

} // namespace xer

#endif /* XER_BITS_TIME_TYPES_H_INCLUDED_ */

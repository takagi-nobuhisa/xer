/**
 * @file xer/bits/time_convert.h
 * @brief Calendar time conversion utilities.
 */

#pragma once

#ifndef XER_BITS_TIME_CONVERT_H_INCLUDED_
#define XER_BITS_TIME_CONVERT_H_INCLUDED_

#include <cmath>
#include <ctime>
#include <expected>

#include <xer/bits/common.h>
#include <xer/bits/time_types.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Converts a XER broken-down time object to a C broken-down time object.
 *
 * @param value Source broken-down time.
 * @return Converted C broken-down time.
 */
[[nodiscard]] inline auto to_std_tm(const xer::tm& value) noexcept -> std::tm
{
    std::tm result {};
    result.tm_sec = value.tm_sec;
    result.tm_min = value.tm_min;
    result.tm_hour = value.tm_hour;
    result.tm_mday = value.tm_mday;
    result.tm_mon = value.tm_mon;
    result.tm_year = value.tm_year;
    result.tm_wday = value.tm_wday;
    result.tm_yday = value.tm_yday;
    result.tm_isdst = value.tm_isdst;
    return result;
}

/**
 * @brief Converts a C broken-down time object to a XER broken-down time object.
 *
 * @param value Source broken-down time.
 * @param microsec Microsecond part.
 * @return Converted XER broken-down time.
 */
[[nodiscard]] inline auto from_std_tm(const std::tm& value, int microsec) noexcept -> xer::tm
{
    xer::tm result {};
    result.tm_sec = value.tm_sec;
    result.tm_min = value.tm_min;
    result.tm_hour = value.tm_hour;
    result.tm_mday = value.tm_mday;
    result.tm_mon = value.tm_mon;
    result.tm_year = value.tm_year;
    result.tm_wday = value.tm_wday;
    result.tm_yday = value.tm_yday;
    result.tm_isdst = value.tm_isdst;
    result.tm_microsec = microsec;
    return result;
}

/**
 * @brief Validates the microsecond field of a XER broken-down time object.
 *
 * @param value Source broken-down time.
 * @return `true` if valid; otherwise `false`.
 */
[[nodiscard]] constexpr auto has_valid_microsec(const xer::tm& value) noexcept -> bool
{
    return value.tm_microsec >= 0 && value.tm_microsec <= 999999;
}

/**
 * @brief Splits a XER calendar time into whole seconds and microseconds.
 *
 * @param value Source calendar time.
 * @param seconds_out Receives the whole seconds.
 * @param microsec_out Receives the microseconds in the range `0..999999`.
 * @return `true` on success; otherwise `false`.
 */
[[nodiscard]] inline auto split_time(xer::time_t value, std::time_t& seconds_out, int& microsec_out) noexcept -> bool
{
    if (!(value >= 0.0)) {
        return false;
    }

    double integral_part = 0.0;
    const double fractional_part = std::modf(value, &integral_part);

    if (integral_part < 0.0) {
        return false;
    }

    const auto seconds = static_cast<std::time_t>(integral_part);
    const auto microsec = static_cast<int>(fractional_part * 1000000.0);

    if (microsec < 0 || microsec > 999999) {
        return false;
    }

    seconds_out = seconds;
    microsec_out = microsec;
    return true;
}

/**
 * @brief Loads UTC broken-down time from whole seconds.
 *
 * @param value Whole seconds since the Unix epoch.
 * @param out Receives the converted broken-down time.
 * @return `true` on success; otherwise `false`.
 */
[[nodiscard]] inline auto gmtime_impl(std::time_t value, std::tm& out) noexcept -> bool
{
#if defined(_WIN32)
    return ::gmtime_s(&out, &value) == 0;
#elif defined(__STDC_LIB_EXT1__)
    return ::gmtime_s(&value, &out) != nullptr;
#elif defined(__unix__) || defined(__APPLE__)
    return ::gmtime_r(&value, &out) != nullptr;
#else
    const std::tm* const temp = std::gmtime(&value);

    if (temp == nullptr) {
        return false;
    }

    out = *temp;
    return true;
#endif
}

/**
 * @brief Loads local broken-down time from whole seconds.
 *
 * @param value Whole seconds since the Unix epoch.
 * @param out Receives the converted broken-down time.
 * @return `true` on success; otherwise `false`.
 */
[[nodiscard]] inline auto localtime_impl(std::time_t value, std::tm& out) noexcept -> bool
{
#if defined(_WIN32)
    return ::localtime_s(&out, &value) == 0;
#elif defined(__STDC_LIB_EXT1__)
    return ::localtime_s(&value, &out) != nullptr;
#elif defined(__unix__) || defined(__APPLE__)
    return ::localtime_r(&value, &out) != nullptr;
#else
    const std::tm* const temp = std::localtime(&value);

    if (temp == nullptr) {
        return false;
    }

    out = *temp;
    return true;
#endif
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Converts a calendar time to UTC broken-down time.
 *
 * @param value Calendar time in seconds since 1970-01-01 00:00:00 UTC.
 * @return UTC broken-down time on success.
 * @return An error with @ref error_t::invalid_argument if `value` is negative.
 * @return An error with @ref error_t::runtime_error on conversion failure.
 */
[[nodiscard]] inline auto gmtime(time_t value) noexcept -> std::expected<tm, error<void>>
{
    std::time_t seconds = 0;
    int microsec = 0;

    if (!detail::split_time(value, seconds, microsec)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::tm std_value {};

    if (!detail::gmtime_impl(seconds, std_value)) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    return detail::from_std_tm(std_value, microsec);
}

/**
 * @brief Converts a calendar time to local broken-down time.
 *
 * @param value Calendar time in seconds since 1970-01-01 00:00:00 UTC.
 * @return Local broken-down time on success.
 * @return An error with @ref error_t::invalid_argument if `value` is negative.
 * @return An error with @ref error_t::runtime_error on conversion failure.
 */
[[nodiscard]] inline auto localtime(time_t value) noexcept -> std::expected<tm, error<void>>
{
    std::time_t seconds = 0;
    int microsec = 0;

    if (!detail::split_time(value, seconds, microsec)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::tm std_value {};

    if (!detail::localtime_impl(seconds, std_value)) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    return detail::from_std_tm(std_value, microsec);
}

/**
 * @brief Converts local broken-down time to a calendar time.
 *
 * @param value Local broken-down time.
 * @return Calendar time on success.
 * @return An error with @ref error_t::invalid_argument if the input is invalid,
 *         represents a time before the Unix epoch, or has an out-of-range
 *         microsecond field.
 * @return An error with @ref error_t::runtime_error on conversion failure.
 */
[[nodiscard]] inline auto mktime(const tm& value) noexcept -> std::expected<time_t, error<void>>
{
    if (!detail::has_valid_microsec(value)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    std::tm std_value = detail::to_std_tm(value);
    const std::time_t result = std::mktime(&std_value);

    if (result == static_cast<std::time_t>(-1)) {
        return std::unexpected(make_error(error_t::runtime_error));
    }

    if (result < static_cast<std::time_t>(0)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    return static_cast<time_t>(result) +
           (static_cast<time_t>(value.tm_microsec) / 1000000.0);
}

} // namespace xer

#endif /* XER_BITS_TIME_CONVERT_H_INCLUDED_ */

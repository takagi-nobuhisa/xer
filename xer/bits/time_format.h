/**
 * @file xer/bits/time_format.h
 * @brief Time string formatting utilities.
 */

#pragma once

#ifndef XER_BITS_TIME_FORMAT_H_INCLUDED_
#define XER_BITS_TIME_FORMAT_H_INCLUDED_

#include <charconv>
#include <cstddef>
#include <ctime>
#include <expected>
#include <optional>
#include <string>
#include <string_view>

#include <xer/bits/common.h>
#include <xer/bits/time_convert.h>
#include <xer/bits/time_types.h>
#include <xer/error.h>

namespace xer::detail {

/**
 * @brief Converts a XER broken-down time object to a C broken-down time object.
 *
 * @param value Source broken-down time.
 * @return Converted C broken-down time.
 */
[[nodiscard]] inline auto time_format_to_std_tm(const xer::tm& value) noexcept -> std::tm
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
 * @brief Checks whether the microsecond field is valid.
 *
 * @param value Source broken-down time.
 * @return `true` if valid; otherwise `false`.
 */
[[nodiscard]] constexpr auto time_format_has_valid_microsec(const xer::tm& value) noexcept -> bool
{
    return value.tm_microsec >= 0 && value.tm_microsec <= 999999;
}

/**
 * @brief Appends a UTF-8 string view to a UTF-8 string.
 *
 * @param out Destination string.
 * @param value Source text.
 */
inline auto append_text(std::u8string& out, std::u8string_view value) -> void
{
    out.append(value.data(), value.size());
}

/**
 * @brief Appends a zero-padded decimal integer.
 *
 * @param out Destination string.
 * @param value Integer value.
 * @param width Minimum decimal width.
 */
inline auto append_zero_padded_decimal(std::u8string& out, int value, std::size_t width) -> void
{
    char buffer[32] {};
    const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);

    if (result.ec != std::errc()) {
        return;
    }

    const std::size_t length = static_cast<std::size_t>(result.ptr - buffer);

    if (value >= 0 && length < width) {
        out.append(width - length, u8'0');
    }

    out.append(reinterpret_cast<const char8_t*>(buffer), length);
}

/**
 * @brief Scans a UTF-8 format string for explicitly unsupported constructs.
 *
 * The initial implementation rejects XER's future `%f` extension and a trailing
 * standalone `%`.
 *
 * @param format Format string.
 * @return `true` if supported; otherwise `false`.
 */
[[nodiscard]] constexpr auto is_supported_strftime_format(std::u8string_view format) noexcept -> bool
{
    for (std::size_t index = 0; index < format.size(); ++index) {
        if (format[index] != u8'%') {
            continue;
        }

        ++index;

        if (index >= format.size()) {
            return false;
        }

        if (format[index] == u8'f') {
            return false;
        }
    }

    return true;
}

/**
 * @brief Formats a broken-down time in a ctime-like representation.
 *
 * The output is stable and locale-independent. A trailing newline is not added.
 * If `tm_microsec` is non-zero, `.uuuuuu` is appended after the seconds field.
 *
 * @param value Source broken-down time.
 * @return Formatted UTF-8 string.
 */
[[nodiscard]] inline auto format_ctime_string(const xer::tm& value) -> std::u8string
{
    static constexpr std::u8string_view weekday_names[] = {
        u8"Sun",
        u8"Mon",
        u8"Tue",
        u8"Wed",
        u8"Thu",
        u8"Fri",
        u8"Sat",
    };

    static constexpr std::u8string_view month_names[] = {
        u8"Jan",
        u8"Feb",
        u8"Mar",
        u8"Apr",
        u8"May",
        u8"Jun",
        u8"Jul",
        u8"Aug",
        u8"Sep",
        u8"Oct",
        u8"Nov",
        u8"Dec",
    };

    std::u8string result;
    result.reserve(40);

    if (value.tm_wday >= 0 && value.tm_wday <= 6) {
        append_text(result, weekday_names[value.tm_wday]);
    }
    else {
        append_text(result, u8"???");
    }

    result.push_back(u8' ');

    if (value.tm_mon >= 0 && value.tm_mon <= 11) {
        append_text(result, month_names[value.tm_mon]);
    }
    else {
        append_text(result, u8"???");
    }

    result.push_back(u8' ');
    append_zero_padded_decimal(result, value.tm_mday, 2);
    result.push_back(u8' ');
    append_zero_padded_decimal(result, value.tm_hour, 2);
    result.push_back(u8':');
    append_zero_padded_decimal(result, value.tm_min, 2);
    result.push_back(u8':');
    append_zero_padded_decimal(result, value.tm_sec, 2);

    if (time_format_has_valid_microsec(value) && value.tm_microsec != 0) {
        result.push_back(u8'.');
        append_zero_padded_decimal(result, value.tm_microsec, 6);
    }

    result.push_back(u8' ');
    append_zero_padded_decimal(result, value.tm_year + 1900, 4);

    return result;
}

/**
 * @brief Formats a single strftime conversion specification.
 *
 * @param spec Conversion specification including the leading `%`.
 * @param value Source broken-down time.
 * @return Formatted narrow string on success.
 * @return `std::nullopt` on failure.
 */
[[nodiscard]] inline auto format_single_strftime_spec(
    std::string_view spec,
    const std::tm& value) noexcept -> std::optional<std::string>
{
    std::string buffer(64, '\0');

    for (;;) {
        const std::size_t count =
            std::strftime(buffer.data(), buffer.size(), spec.data(), &value);

        if (count != 0) {
            buffer.resize(count);
            return buffer;
        }

        if (buffer.size() >= 4096) {
            return std::nullopt;
        }

        buffer.resize(buffer.size() * 2);
    }
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Formats a broken-down time as a UTF-8 string.
 *
 * The output is a locale-independent ctime-like representation without the
 * trailing newline used by C's `ctime()` and `asctime()`.
 *
 * Example:
 * `Wed Jun 30 21:49:08 1993`
 *
 * If `tm_microsec` is non-zero, `.uuuuuu` is appended after the seconds field.
 *
 * @param value Broken-down time.
 * @return Formatted UTF-8 string.
 */
[[nodiscard]] inline auto ctime(const tm& value) -> std::u8string
{
    return detail::format_ctime_string(value);
}

/**
 * @brief Formats a calendar time as a UTF-8 string.
 *
 * This function converts the calendar time to local broken-down time and then
 * formats it with @ref ctime(const tm&).
 *
 * @param value Calendar time.
 * @return Formatted UTF-8 string. Returns an empty string on conversion failure.
 */
[[nodiscard]] inline auto ctime(time_t value) -> std::u8string
{
    const auto broken = xer::localtime(value);

    if (!broken.has_value()) {
        return std::u8string();
    }

    return xer::ctime(broken.value());
}

/**
 * @brief Formats a broken-down time according to a UTF-8 format string.
 *
 * The initial implementation delegates each conversion specification to the
 * underlying C library's `strftime()`, while preserving UTF-8 literal text
 * outside the conversion specifications as-is.
 *
 * The `%f` conversion specifier is intentionally unsupported at this stage.
 *
 * @param format UTF-8 format string.
 * @param value Broken-down time.
 * @return Formatted UTF-8 string on success.
 * @return An error with @ref error_t::invalid_argument if the format string or
 *         `tm_microsec` is invalid.
 * @return An error with @ref error_t::runtime_error on formatting failure.
 */
[[nodiscard]] inline auto strftime(
    std::u8string_view format,
    const tm& value) noexcept -> std::expected<std::u8string, error<void>>
{
    if (!detail::time_format_has_valid_microsec(value)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (!detail::is_supported_strftime_format(format)) {
        return std::unexpected(make_error(error_t::invalid_argument));
    }

    if (format.empty()) {
        return std::u8string();
    }

    const std::tm std_value = detail::time_format_to_std_tm(value);

    std::u8string result;
    result.reserve(format.size() + 32);

    for (std::size_t index = 0; index < format.size(); ++index) {
        if (format[index] != u8'%') {
            result.push_back(format[index]);
            continue;
        }

        const std::size_t spec_begin = index;
        ++index;

        while (index < format.size()) {
            const char8_t ch = format[index];
            const bool is_alpha =
                (ch >= u8'A' && ch <= u8'Z') ||
                (ch >= u8'a' && ch <= u8'z');

            if (is_alpha || ch == u8'%') {
                break;
            }

            ++index;
        }

        if (index >= format.size()) {
            return std::unexpected(make_error(error_t::invalid_argument));
        }

        std::string spec;
        spec.reserve(index - spec_begin + 2);

        for (std::size_t pos = spec_begin; pos <= index; ++pos) {
            spec.push_back(static_cast<char>(format[pos]));
        }

        spec.push_back('\0');

        const auto formatted =
            detail::format_single_strftime_spec(std::string_view(spec.data(), spec.size() - 1), std_value);

        if (!formatted.has_value()) {
            return std::unexpected(make_error(error_t::runtime_error));
        }

        result.append(
            reinterpret_cast<const char8_t*>(formatted->data()),
            reinterpret_cast<const char8_t*>(formatted->data() + formatted->size()));
    }

    return result;
}

} // namespace xer

#endif /* XER_BITS_TIME_FORMAT_H_INCLUDED_ */

/**
 * @file xer/bits/log.h
 * @brief Log output support for XER diagnostics.
 */

#pragma once

#ifndef XER_BITS_LOG_H_INCLUDED_
#define XER_BITS_LOG_H_INCLUDED_

#ifndef XER_ENABLE_LOG
#    define XER_ENABLE_LOG 1
#endif

#include <array>
#include <charconv>
#include <cstdio>
#include <ctime>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include <xer/bits/diag_common.h>
#include <xer/bits/printf.h>
#include <xer/bits/standard_streams.h>
#include <xer/bits/text_stream.h>
#include <xer/bits/text_stream_io.h>

namespace xer {

[[nodiscard]] inline auto get_log_stream() noexcept -> text_stream&;

} // namespace xer

namespace xer::detail {

inline text_stream* log_output_stream = nullptr;
inline diag_level_t current_log_level = diag_info;

/**
 * @brief Cached timestamp prefix for one thread.
 *
 * The log timestamp format is YYYY-MM-DD HH:MM:SS.mmm. The part up to seconds
 * is cached per thread so that repeated log records within the same second do
 * not repeatedly perform local-time formatting.
 */
struct log_timestamp_cache {
    std::time_t second = static_cast<std::time_t>(-1);
    std::array<char8_t, 19> prefix{};
};

/**
 * @brief Timestamp text used by one log record.
 */
struct log_timestamp_text {
    std::array<char8_t, 23> text{};

    [[nodiscard]] auto view() const noexcept -> std::u8string_view
    {
        return std::u8string_view(text.data(), text.size());
    }
};

/**
 * @brief Converts a narrow ASCII byte to char8_t.
 * @param value Source byte.
 * @return Converted char8_t value.
 */
[[nodiscard]] constexpr auto ascii_to_char8(char value) noexcept -> char8_t
{
    return static_cast<char8_t>(static_cast<unsigned char>(value));
}

/**
 * @brief Produces a local timestamp with millisecond precision.
 * @return Timestamp text in YYYY-MM-DD HH:MM:SS.mmm form.
 */
[[nodiscard]] inline auto make_log_timestamp() -> log_timestamp_text
{
    std::timespec time_spec{};
    if (std::timespec_get(&time_spec, TIME_UTC) != TIME_UTC) {
        time_spec.tv_sec = std::time(nullptr);
        time_spec.tv_nsec = 0;
    }

    const auto second = static_cast<std::time_t>(time_spec.tv_sec);
    const auto millisecond = static_cast<int>(time_spec.tv_nsec / 1000000L);

    thread_local log_timestamp_cache cache;

    if (cache.second != second) {
        cache.second = second;

        std::tm tm_value{};
#if defined(_WIN32)
        localtime_s(&tm_value, &second);
#else
        localtime_r(&second, &tm_value);
#endif

        char buffer[20]{};
        static_cast<void>(std::snprintf(
            buffer,
            sizeof(buffer),
            "%04d-%02d-%02d %02d:%02d:%02d",
            tm_value.tm_year + 1900,
            tm_value.tm_mon + 1,
            tm_value.tm_mday,
            tm_value.tm_hour,
            tm_value.tm_min,
            tm_value.tm_sec));

        for (std::size_t i = 0; i < cache.prefix.size(); ++i) {
            cache.prefix[i] = ascii_to_char8(buffer[i]);
        }
    }

    log_timestamp_text result;

    for (std::size_t i = 0; i < cache.prefix.size(); ++i) {
        result.text[i] = cache.prefix[i];
    }

    result.text[19] = u8'.';
    result.text[20] = static_cast<char8_t>(u8'0' + ((millisecond / 100) % 10));
    result.text[21] = static_cast<char8_t>(u8'0' + ((millisecond / 10) % 10));
    result.text[22] = static_cast<char8_t>(u8'0' + (millisecond % 10));

    return result;
}

/**
 * @brief Converts a diagnostic level to a small stack-backed UTF-8 buffer.
 */
struct log_level_text {
    std::array<char8_t, 32> text{};
    std::size_t size = 0;

    [[nodiscard]] auto view() const noexcept -> std::u8string_view
    {
        return std::u8string_view(text.data(), size);
    }
};

/**
 * @brief Converts a diagnostic level to UTF-8 decimal text without printf.
 * @param level Source level.
 * @return Decimal level text.
 */
[[nodiscard]] inline auto make_log_level_text(diag_level_t level) noexcept -> log_level_text
{
    log_level_text result;

    char buffer[32]{};
    auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), level);
    if (ec != std::errc{}) {
        result.text[0] = u8'0';
        result.size = 1;
        return result;
    }

    result.size = static_cast<std::size_t>(ptr - buffer);
    for (std::size_t i = 0; i < result.size; ++i) {
        result.text[i] = ascii_to_char8(buffer[i]);
    }

    return result;
}

/**
 * @brief Writes UTF-8 text to a text stream and reports only success/failure.
 * @param stream Destination stream.
 * @param text Source text.
 * @return Success or failure.
 */
[[nodiscard]] inline auto write_log_text(
    text_stream& stream,
    std::u8string_view text) -> result<void>
{
    const auto written = fputs(text, stream);
    if (!written.has_value()) {
        return std::unexpected(written.error());
    }

    return {};
}

/**
 * @brief Writes one ASCII character to a text stream.
 * @param stream Destination stream.
 * @param ch Source character.
 * @return Success or failure.
 */
[[nodiscard]] inline auto write_log_ascii(
    text_stream& stream,
    char32_t ch) -> result<void>
{
    const auto written = fputc(ch, stream);
    if (!written.has_value()) {
        return std::unexpected(written.error());
    }

    return {};
}

/**
 * @brief Writes the log message field as a quoted CSV field.
 *
 * The message field is the only user-controlled CSV field in XER's built-in
 * log record. It is always enclosed and written directly to the stream without
 * constructing a separately quoted string. Double quote bytes in the UTF-8 text
 * are escaped by writing two double quote characters.
 *
 * @param stream Destination stream.
 * @param message Log message text.
 * @return Success or failure.
 */
[[nodiscard]] inline auto write_log_message_field(
    text_stream& stream,
    std::u8string_view message) -> result<void>
{
    if (const auto result = write_log_ascii(stream, U'"'); !result.has_value()) {
        return result;
    }

    std::size_t start = 0;
    for (std::size_t i = 0; i < message.size(); ++i) {
        if (message[i] != u8'"') {
            continue;
        }

        if (i > start) {
            const auto chunk = message.substr(start, i - start);
            if (const auto result = write_log_text(stream, chunk); !result.has_value()) {
                return result;
            }
        }

        if (const auto result = write_log_ascii(stream, U'"'); !result.has_value()) {
            return result;
        }
        if (const auto result = write_log_ascii(stream, U'"'); !result.has_value()) {
            return result;
        }

        start = i + 1;
    }

    if (start < message.size()) {
        if (const auto result = write_log_text(stream, message.substr(start)); !result.has_value()) {
            return result;
        }
    }

    return write_log_ascii(stream, U'"');
}

/**
 * @brief Writes one fixed-layout CSV log record.
 *
 * Columns are timestamp, category, level, and message. The first three fields
 * are controlled by XER and are written without quoting. The message field is
 * quoted and escaped as a CSV field.
 *
 * @param category Log category.
 * @param level Log level.
 * @param message Log message text.
 */
inline auto write_log_record(
    diag_category category,
    diag_level_t level,
    std::u8string_view message) -> void
{
    text_stream& stream = get_log_stream();
    const auto timestamp = make_log_timestamp();
    const auto level_text = make_log_level_text(level);

    if (!write_log_text(stream, timestamp.view()).has_value()) {
        return;
    }
    if (!write_log_ascii(stream, U',').has_value()) {
        return;
    }
    if (!write_log_text(stream, get_diag_category_name(category)).has_value()) {
        return;
    }
    if (!write_log_ascii(stream, U',').has_value()) {
        return;
    }
    if (!write_log_text(stream, level_text.view()).has_value()) {
        return;
    }
    if (!write_log_ascii(stream, U',').has_value()) {
        return;
    }
    if (!write_log_message_field(stream, message).has_value()) {
        return;
    }

    static_cast<void>(write_log_ascii(stream, U'\n'));
}

} // namespace xer::detail

namespace xer {

/**
 * @brief Sets the destination stream for log output.
 *
 * The stream is borrowed and must outlive log use. Passing a temporary stream
 * object is therefore not appropriate.
 *
 * @param stream New log output stream.
 */
inline auto set_log_stream(text_stream& stream) noexcept -> void
{
    detail::log_output_stream = &stream;
}

/**
 * @brief Resets log output to the default standard error stream.
 */
inline auto reset_log_stream() noexcept -> void
{
    detail::log_output_stream = nullptr;
}

/**
 * @brief Returns the current log output stream.
 *
 * @return Log output stream, or standard error when no override is set.
 */
[[nodiscard]] inline auto get_log_stream() noexcept -> text_stream&
{
    if (detail::log_output_stream != nullptr) {
        return *detail::log_output_stream;
    }

    return xer_stderr;
}

/**
 * @brief Sets the current log level.
 *
 * Log messages whose level is numerically larger than this value are omitted.
 *
 * @param level New log level.
 */
inline auto set_log_level(diag_level_t level) noexcept -> void
{
    detail::current_log_level = level;
}

/**
 * @brief Returns the current log level.
 * @return Current log level.
 */
[[nodiscard]] inline auto get_log_level() noexcept -> diag_level_t
{
    return detail::current_log_level;
}

/**
 * @brief Tests whether a log message should be emitted.
 *
 * The category is currently used for output and future filtering. The initial
 * implementation filters by level only.
 *
 * @param category Log category.
 * @param level Requested log level.
 * @return true if the log message is enabled.
 */
[[nodiscard]] inline auto is_log_enabled(
    diag_category category,
    diag_level_t level) noexcept -> bool
{
    static_cast<void>(category);
    return is_diag_level_enabled(get_log_level(), level);
}

/**
 * @brief Emits one simple CSV log record.
 *
 * This overload writes the message directly without printf formatting. It is
 * intended for the common case where the log message is already fixed text.
 *
 * @param category Log category.
 * @param level Log level.
 * @param message Log message text.
 */
inline auto log(
    diag_category category,
    diag_level_t level,
    std::u8string_view message) -> void
{
    if (!is_log_enabled(category, level)) {
        return;
    }

    detail::write_log_record(category, level, message);
}

/**
 * @brief Emits one formatted CSV log record.
 *
 * The message body is formatted with XER printf rules, so `%@` can be used for
 * generic display output. Formatting is performed only for this overload; the
 * simple-message overload avoids the formatting step.
 *
 * @tparam Args Format argument types.
 * @param category Log category.
 * @param level Log level.
 * @param format UTF-8 log message format.
 * @param args Format arguments.
 */
template<class... Args>
    requires(sizeof...(Args) > 0)
inline auto log(
    diag_category category,
    diag_level_t level,
    std::u8string_view format,
    Args&&... args) -> void
{
    if (!is_log_enabled(category, level)) {
        return;
    }

    std::u8string message;
    const auto formatted = sprintf(message, format, std::forward<Args>(args)...);
    if (!formatted.has_value()) {
        return;
    }

    detail::write_log_record(category, level, message);
}

} // namespace xer

#if !XER_ENABLE_LOG
#    define xer_log(category, level, ...) static_cast<void>(0)
#else
#    define xer_log(category, level, ...)                                       \
        [&]() -> void {                                                         \
            const auto xer_log_category__ = (category);                         \
            const auto xer_log_level__ = (level);                               \
            if (::xer::is_log_enabled(xer_log_category__, xer_log_level__)) {    \
                ::xer::log(xer_log_category__, xer_log_level__, __VA_ARGS__);    \
            }                                                                   \
        }()
#endif

#endif /* XER_BITS_LOG_H_INCLUDED_ */

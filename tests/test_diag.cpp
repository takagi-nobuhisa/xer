/**
 * @file tests/test_diag.cpp
 * @brief Tests for xer/diag.h.
 */

#include <cstddef>
#include <string>
#include <string_view>

#include <xer/assert.h>
#include <xer/diag.h>
#include <xer/stdio.h>
#include <xer/string.h>
#include <xer/typeinfo.h>

namespace {

/**
 * @brief Tests whether a UTF-8 code unit is an ASCII decimal digit.
 * @param value Source code unit.
 * @return true if value is a digit.
 */
[[nodiscard]] auto is_ascii_digit(char8_t value) noexcept -> bool
{
    return value >= u8'0' && value <= u8'9';
}

/**
 * @brief Tests the fixed timestamp prefix used by CSV log records.
 * @param text Complete log line.
 */
auto assert_log_timestamp_shape(std::u8string_view text) -> void
{
    xer_assert(text.size() >= 24);

    for (std::size_t index : {0U, 1U, 2U, 3U, 5U, 6U, 8U, 9U, 11U, 12U,
                              14U, 15U, 17U, 18U, 20U, 21U, 22U}) {
        xer_assert(is_ascii_digit(text[index]));
    }

    xer_assert_eq(text[4], u8'-');
    xer_assert_eq(text[7], u8'-');
    xer_assert_eq(text[10], u8' ');
    xer_assert_eq(text[13], u8':');
    xer_assert_eq(text[16], u8':');
    xer_assert_eq(text[19], u8'.');
    xer_assert_eq(text[23], u8',');
}

/**
 * @brief Tests whether a log line has the expected suffix after timestamp.
 * @param text Complete log line.
 * @param suffix Expected suffix including the comma after timestamp.
 */
auto assert_log_suffix(
    std::u8string_view text,
    std::u8string_view suffix) -> void
{
    assert_log_timestamp_shape(text);
    xer_assert(text.size() >= 23 + suffix.size());
    xer_assert_eq(text.substr(23), suffix);
}

/**
 * @brief Tests diagnostic category names.
 */
auto test_diag_category_names() -> void
{
    xer_assert_eq(xer::get_diag_category_name(xer::diag_category::general), u8"general");
    xer_assert_eq(xer::get_diag_category_name(xer::diag_category::io), u8"io");
    xer_assert_eq(xer::get_diag_category_name(xer::diag_category::path), u8"path");
    xer_assert_eq(xer::get_diag_category_name(xer::diag_category::process), u8"process");
    xer_assert_eq(xer::get_diag_category_name(xer::diag_category::socket), u8"socket");
    xer_assert_eq(xer::get_diag_category_name(xer::diag_category::user), u8"user");
}

/**
 * @brief Tests diagnostic level filtering.
 */
auto test_diag_level_filtering() -> void
{
    xer_assert(xer::is_diag_level_enabled(xer::diag_info, xer::diag_error));
    xer_assert(xer::is_diag_level_enabled(xer::diag_info, xer::diag_warning));
    xer_assert(xer::is_diag_level_enabled(xer::diag_info, xer::diag_info));
    xer_assert_not(xer::is_diag_level_enabled(xer::diag_info, xer::diag_debug));
    xer_assert_not(xer::is_diag_level_enabled(xer::diag_info, xer::diag_verbose));
}

/**
 * @brief Tests concise type names used by trace output.
 */
auto test_type_name_shortcuts() -> void
{
    xer_assert_eq(xer::type_name<int>(), std::u8string(u8"int"));
    xer_assert_eq(xer::type_name<const int&>(), std::u8string(u8"int"));
    xer_assert_eq(xer::type_name<std::u8string>(), std::u8string(u8"std::u8string"));
    xer_assert_eq(xer::type_name<std::string_view>(), std::u8string(u8"std::string_view"));
    xer_assert_eq(xer::type_name<xer::error_t>(), std::u8string(u8"xer::error_t"));
}

/**
 * @brief Tests trace output to a custom stream.
 */
auto test_trace_output_to_custom_stream() -> void
{
    std::u8string output;
    auto stream_result = xer::stropen(output, "w");
    xer_assert(stream_result.has_value());

    xer::set_trace_stream(*stream_result);
    xer::set_trace_level(xer::diag_debug);

    const int value = 42;
    xer_trace(xer::diag_category::general, xer::diag_debug, value);

    xer::reset_trace_stream();
    xer::set_trace_level(xer::diag_info);

    xer_assert_eq(output, std::u8string(u8"[general][40] value (int) = 42\n"));
}

/**
 * @brief Tests trace level suppression.
 */
auto test_trace_level_suppression() -> void
{
    std::u8string output;
    auto stream_result = xer::stropen(output, "w");
    xer_assert(stream_result.has_value());

    xer::set_trace_stream(*stream_result);
    xer::set_trace_level(xer::diag_info);

    const int value = 42;
    xer_trace(xer::diag_category::general, xer::diag_debug, value);

    xer::reset_trace_stream();

    xer_assert_eq(output, std::u8string());
}

/**
 * @brief Tests simple log output without printf formatting.
 */
auto test_log_simple_output_to_custom_stream() -> void
{
    std::u8string output;
    auto stream_result = xer::stropen(output, "w");
    xer_assert(stream_result.has_value());

    xer::set_log_stream(*stream_result);
    xer::set_log_level(xer::diag_info);

    xer_log(xer::diag_category::io, xer::diag_info, u8"opened file.txt");

    xer::reset_log_stream();

    assert_log_suffix(output, u8",io,30,\"opened file.txt\"\n");
}

/**
 * @brief Tests formatted log output and CSV escaping of double quotes.
 */
auto test_log_formatted_output_to_custom_stream() -> void
{
    std::u8string output;
    auto stream_result = xer::stropen(output, "w");
    xer_assert(stream_result.has_value());

    xer::set_log_stream(*stream_result);
    xer::set_log_level(xer::diag_info);

    xer_log(xer::diag_category::io, xer::diag_info, u8"opened %@", u8"\"file\".txt");

    xer::reset_log_stream();

    assert_log_suffix(output, u8",io,30,\"opened \"\"file\"\".txt\"\n");
}

/**
 * @brief Tests log level suppression.
 */
auto test_log_level_suppression() -> void
{
    std::u8string output;
    auto stream_result = xer::stropen(output, "w");
    xer_assert(stream_result.has_value());

    xer::set_log_stream(*stream_result);
    xer::set_log_level(xer::diag_warning);

    xer_log(xer::diag_category::io, xer::diag_info, u8"not shown");

    xer::reset_log_stream();
    xer::set_log_level(xer::diag_info);

    xer_assert_eq(output, std::u8string());
}

} // namespace

/**
 * @brief Entry point.
 * @return Zero on success.
 */
auto main() -> int
{
    test_diag_category_names();
    test_diag_level_filtering();
    test_type_name_shortcuts();
    test_trace_output_to_custom_stream();
    test_trace_level_suppression();
    test_log_simple_output_to_custom_stream();
    test_log_formatted_output_to_custom_stream();
    test_log_level_suppression();

    return 0;
}

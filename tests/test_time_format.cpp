/**
 * @file tests/test_time_format.cpp
 * @brief Tests for xer/bits/time_format.h.
 */

#include <xer/assert.h>
#include <xer/bits/time_format.h>

namespace {

/**
 * @brief Tests ctime(const tm&) with a basic value.
 */
void test_ctime_tm_basic()
{
    xer::tm value {};
    value.tm_year = 93;
    value.tm_mon = 5;
    value.tm_mday = 30;
    value.tm_hour = 21;
    value.tm_min = 49;
    value.tm_sec = 8;
    value.tm_wday = 3;
    value.tm_yday = 180;
    value.tm_isdst = 0;
    value.tm_microsec = 0;

    const std::u8string result = xer::ctime(value);

    xer_assert_eq(result, u8"Wed Jun 30 21:49:08 1993");
}

/**
 * @brief Tests ctime(const tm&) with a non-zero microsecond field.
 */
void test_ctime_tm_with_microsec()
{
    xer::tm value {};
    value.tm_year = 124;
    value.tm_mon = 0;
    value.tm_mday = 2;
    value.tm_hour = 3;
    value.tm_min = 4;
    value.tm_sec = 5;
    value.tm_wday = 2;
    value.tm_yday = 1;
    value.tm_isdst = 0;
    value.tm_microsec = 123456;

    const std::u8string result = xer::ctime(value);

    xer_assert_eq(result, u8"Tue Jan 02 03:04:05.123456 2024");
}

/**
 * @brief Tests ctime(const tm&) with unknown weekday and month values.
 */
void test_ctime_tm_unknown_fields()
{
    xer::tm value {};
    value.tm_year = 100;
    value.tm_mon = -1;
    value.tm_mday = 1;
    value.tm_hour = 0;
    value.tm_min = 0;
    value.tm_sec = 0;
    value.tm_wday = 7;
    value.tm_yday = 0;
    value.tm_isdst = 0;
    value.tm_microsec = 0;

    const std::u8string result = xer::ctime(value);

    xer_assert_eq(result, u8"??? ??? 01 00:00:00 2000");
}

/**
 * @brief Tests ctime(time_t) for the Unix epoch.
 *
 * The exact broken-down representation depends on the local time zone,
 * so this test compares against the result of localtime() formatted via
 * ctime(const tm&).
 */
void test_ctime_time_epoch()
{
    const auto broken = xer::localtime(0.0);
    xer_assert(broken.has_value());

    const std::u8string expected = xer::ctime(broken.value());
    const std::u8string result = xer::ctime(0.0);

    xer_assert_eq(result, expected);
}

/**
 * @brief Tests ctime(time_t) with a fractional calendar time.
 */
void test_ctime_time_fractional()
{
    const auto broken = xer::localtime(12345.25);
    xer_assert(broken.has_value());

    const std::u8string expected = xer::ctime(broken.value());
    const std::u8string result = xer::ctime(12345.25);

    xer_assert_eq(result, expected);
}

/**
 * @brief Tests ctime(time_t) for a negative calendar time.
 *
 * According to the current design, negative time_t is unsupported and
 * ctime(time_t) returns an empty string when the internal conversion fails.
 */
void test_ctime_time_negative()
{
    const std::u8string result = xer::ctime(-1.0);

    xer_assert(result.empty());
}

/**
 * @brief Tests strftime() with a basic ASCII format.
 */
void test_strftime_basic_ascii()
{
    xer::tm value {};
    value.tm_year = 124;
    value.tm_mon = 0;
    value.tm_mday = 2;
    value.tm_hour = 3;
    value.tm_min = 4;
    value.tm_sec = 5;
    value.tm_wday = 2;
    value.tm_yday = 1;
    value.tm_isdst = 0;
    value.tm_microsec = 0;

    const auto result = xer::strftime(u8"%Y-%m-%d %H:%M:%S", value);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), u8"2024-01-02 03:04:05");
}

/**
 * @brief Tests strftime() with UTF-8 literal text in the format string.
 */
void test_strftime_utf8_literal_text()
{
    xer::tm value {};
    value.tm_year = 124;
    value.tm_mon = 0;
    value.tm_mday = 2;
    value.tm_hour = 3;
    value.tm_min = 4;
    value.tm_sec = 5;
    value.tm_wday = 2;
    value.tm_yday = 1;
    value.tm_isdst = 0;
    value.tm_microsec = 0;

    const auto result = xer::strftime(u8"%Y年%m月%d日 %H時%M分%S秒", value);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), u8"2024年01月02日 03時04分05秒");
}

/**
 * @brief Tests strftime() with escaped percent.
 */
void test_strftime_percent_escape()
{
    xer::tm value {};
    value.tm_year = 124;
    value.tm_mon = 0;
    value.tm_mday = 2;
    value.tm_hour = 3;
    value.tm_min = 4;
    value.tm_sec = 5;
    value.tm_wday = 2;
    value.tm_yday = 1;
    value.tm_isdst = 0;
    value.tm_microsec = 0;

    const auto result = xer::strftime(u8"%%Y=%Y", value);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), u8"%Y=2024");
}

/**
 * @brief Tests strftime() with an empty format string.
 */
void test_strftime_empty_format()
{
    xer::tm value {};
    value.tm_year = 124;
    value.tm_mon = 0;
    value.tm_mday = 2;
    value.tm_hour = 3;
    value.tm_min = 4;
    value.tm_sec = 5;
    value.tm_wday = 2;
    value.tm_yday = 1;
    value.tm_isdst = 0;
    value.tm_microsec = 0;

    const auto result = xer::strftime(u8"", value);

    xer_assert(result.has_value());
    xer_assert(result.value().empty());
}

/**
 * @brief Tests strftime() with XER's microsecond extension.
 */
void test_strftime_microsecond_extension()
{
    xer::tm value {};
    value.tm_year = 124;
    value.tm_mon = 0;
    value.tm_mday = 2;
    value.tm_hour = 3;
    value.tm_min = 4;
    value.tm_sec = 5;
    value.tm_wday = 2;
    value.tm_yday = 1;
    value.tm_isdst = 0;
    value.tm_microsec = 123456;

    const auto result = xer::strftime(u8"%Y-%m-%d %H:%M:%S.%f", value);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), u8"2024-01-02 03:04:05.123456");
}

/**
 * @brief Tests strftime() with XER's millisecond extension.
 */
void test_strftime_millisecond_extension()
{
    xer::tm value {};
    value.tm_year = 124;
    value.tm_mon = 0;
    value.tm_mday = 2;
    value.tm_hour = 3;
    value.tm_min = 4;
    value.tm_sec = 5;
    value.tm_wday = 2;
    value.tm_yday = 1;
    value.tm_isdst = 0;
    value.tm_microsec = 123456;

    const auto result = xer::strftime(u8"%H:%M:%S.%L", value);

    xer_assert(result.has_value());
    xer_assert_eq(result.value(), u8"03:04:05.123");
}

/**
 * @brief Tests strftime() rejection of modified XER fractional specifiers.
 */
void test_strftime_modified_fractional_extension()
{
    xer::tm value {};
    value.tm_year = 124;
    value.tm_mon = 0;
    value.tm_mday = 2;
    value.tm_hour = 3;
    value.tm_min = 4;
    value.tm_sec = 5;
    value.tm_wday = 2;
    value.tm_yday = 1;
    value.tm_isdst = 0;
    value.tm_microsec = 123456;

    const auto result = xer::strftime(u8"%3f", value);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

/**
 * @brief Tests strftime() rejection of a trailing standalone percent.
 */
void test_strftime_trailing_percent()
{
    xer::tm value {};
    value.tm_year = 124;
    value.tm_mon = 0;
    value.tm_mday = 2;
    value.tm_hour = 3;
    value.tm_min = 4;
    value.tm_sec = 5;
    value.tm_wday = 2;
    value.tm_yday = 1;
    value.tm_isdst = 0;
    value.tm_microsec = 0;

    const auto result = xer::strftime(u8"%Y-%m-%d %", value);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

/**
 * @brief Tests strftime() rejection of an invalid microsecond field.
 */
void test_strftime_invalid_microsec()
{
    xer::tm value {};
    value.tm_year = 124;
    value.tm_mon = 0;
    value.tm_mday = 2;
    value.tm_hour = 3;
    value.tm_min = 4;
    value.tm_sec = 5;
    value.tm_wday = 2;
    value.tm_yday = 1;
    value.tm_isdst = 0;
    value.tm_microsec = 1000000;

    const auto result = xer::strftime(u8"%Y-%m-%d %H:%M:%S", value);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

} // namespace

int main()
{
    test_ctime_tm_basic();
    test_ctime_tm_with_microsec();
    test_ctime_tm_unknown_fields();
    test_ctime_time_epoch();
    test_ctime_time_fractional();
    test_ctime_time_negative();
    test_strftime_basic_ascii();
    test_strftime_utf8_literal_text();
    test_strftime_percent_escape();
    test_strftime_empty_format();
    test_strftime_microsecond_extension();
    test_strftime_millisecond_extension();
    test_strftime_modified_fractional_extension();
    test_strftime_trailing_percent();
    test_strftime_invalid_microsec();

    return 0;
}

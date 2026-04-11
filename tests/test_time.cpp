/**
 * @file tests/test_time.cpp
 * @brief Basic integration tests for xer/time.h.
 */

#include <cmath>

#include <xer/assert.h>
#include <xer/time.h>

namespace {

/**
 * @brief Comparison tolerance for floating-point calendar times.
 */
constexpr double time_epsilon = 1e-12;

/**
 * @brief Tests that the public header provides time().
 */
void test_time_available()
{
    const auto result = xer::time();

    xer_assert(result.has_value());
    xer_assert(result.value() >= 0.0);
}

/**
 * @brief Tests that the public header provides clock().
 */
void test_clock_available()
{
    const auto result = xer::clock();

    xer_assert(result.has_value());
}

/**
 * @brief Tests that the public header provides difftime().
 */
void test_difftime_available()
{
    xer_assert_eq(xer::difftime(10.5, 8.25), 2.25);
}

/**
 * @brief Tests that the public header provides gmtime().
 */
void test_gmtime_available()
{
    const auto result = xer::gmtime(0.25);

    xer_assert(result.has_value());
    xer_assert_eq(result.value().tm_year, 70);
    xer_assert_eq(result.value().tm_mon, 0);
    xer_assert_eq(result.value().tm_mday, 1);
    xer_assert_eq(result.value().tm_hour, 0);
    xer_assert_eq(result.value().tm_min, 0);
    xer_assert_eq(result.value().tm_sec, 0);
    xer_assert_eq(result.value().tm_microsec, 250000);
}

/**
 * @brief Tests that the public header provides localtime().
 */
void test_localtime_available()
{
    const auto result = xer::localtime(1.5);

    xer_assert(result.has_value());
    xer_assert_eq(result.value().tm_microsec, 500000);
}

/**
 * @brief Tests that the public header provides mktime().
 */
void test_mktime_available()
{
    const auto local = xer::localtime(12345.625);
    xer_assert(local.has_value());

    const auto result = xer::mktime(local.value());

    xer_assert(result.has_value());
    xer_assert(std::abs(result.value() - 12345.625) < time_epsilon);
}

/**
 * @brief Tests that the public header provides ctime().
 */
void test_ctime_available()
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
 * @brief Tests that the public header provides strftime().
 */
void test_strftime_available()
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

} // namespace

int main()
{
    test_time_available();
    test_clock_available();
    test_difftime_available();
    test_gmtime_available();
    test_localtime_available();
    test_mktime_available();
    test_ctime_available();
    test_strftime_available();

    return 0;
}

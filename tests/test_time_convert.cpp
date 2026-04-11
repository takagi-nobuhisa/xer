/**
 * @file tests/test_time_convert.cpp
 * @brief Tests for xer/bits/time_convert.h.
 */

#include <cmath>

#include <xer/assert.h>
#include <xer/bits/time_convert.h>

namespace {

/**
 * @brief Comparison tolerance for floating-point calendar times.
 */
constexpr double time_epsilon = 1e-12;

/**
 * @brief Tests that gmtime() rejects a negative calendar time.
 */
void test_gmtime_negative_time()
{
    const auto result = xer::gmtime(-0.25);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

/**
 * @brief Tests that localtime() rejects a negative calendar time.
 */
void test_localtime_negative_time()
{
    const auto result = xer::localtime(-1.0);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

/**
 * @brief Tests that gmtime() converts the Unix epoch correctly.
 */
void test_gmtime_epoch()
{
    const auto result = xer::gmtime(0.0);

    xer_assert(result.has_value());

    const xer::tm& value = result.value();
    xer_assert_eq(value.tm_year, 70);
    xer_assert_eq(value.tm_mon, 0);
    xer_assert_eq(value.tm_mday, 1);
    xer_assert_eq(value.tm_hour, 0);
    xer_assert_eq(value.tm_min, 0);
    xer_assert_eq(value.tm_sec, 0);
    xer_assert_eq(value.tm_wday, 4);
    xer_assert_eq(value.tm_yday, 0);
    xer_assert_eq(value.tm_microsec, 0);
}

/**
 * @brief Tests that gmtime() preserves the microsecond part.
 */
void test_gmtime_microseconds()
{
    const auto result = xer::gmtime(1.25);

    xer_assert(result.has_value());

    const xer::tm& value = result.value();
    xer_assert_eq(value.tm_year, 70);
    xer_assert_eq(value.tm_mon, 0);
    xer_assert_eq(value.tm_mday, 1);
    xer_assert_eq(value.tm_hour, 0);
    xer_assert_eq(value.tm_min, 0);
    xer_assert_eq(value.tm_sec, 1);
    xer_assert_eq(value.tm_microsec, 250000);
}

/**
 * @brief Tests that gmtime() converts one full day after the epoch correctly.
 */
void test_gmtime_next_day()
{
    const auto result = xer::gmtime(86400.0);

    xer_assert(result.has_value());

    const xer::tm& value = result.value();
    xer_assert_eq(value.tm_year, 70);
    xer_assert_eq(value.tm_mon, 0);
    xer_assert_eq(value.tm_mday, 2);
    xer_assert_eq(value.tm_hour, 0);
    xer_assert_eq(value.tm_min, 0);
    xer_assert_eq(value.tm_sec, 0);
    xer_assert_eq(value.tm_wday, 5);
    xer_assert_eq(value.tm_yday, 1);
    xer_assert_eq(value.tm_microsec, 0);
}

/**
 * @brief Tests that localtime() accepts the Unix epoch and preserves
 *        microseconds.
 *
 * This test does not check calendar fields because they depend on the local
 * time zone. It only checks properties that must hold regardless of the zone.
 */
void test_localtime_epoch_and_microseconds()
{
    const auto result = xer::localtime(0.75);

    xer_assert(result.has_value());

    const xer::tm& value = result.value();
    xer_assert(value.tm_sec >= 0);
    xer_assert(value.tm_sec <= 60);
    xer_assert(value.tm_min >= 0);
    xer_assert(value.tm_min <= 59);
    xer_assert(value.tm_hour >= 0);
    xer_assert(value.tm_hour <= 23);
    xer_assert(value.tm_mon >= 0);
    xer_assert(value.tm_mon <= 11);
    xer_assert(value.tm_mday >= 1);
    xer_assert(value.tm_mday <= 31);
    xer_assert(value.tm_wday >= 0);
    xer_assert(value.tm_wday <= 6);
    xer_assert(value.tm_yday >= 0);
    xer_assert(value.tm_yday <= 365);
    xer_assert_eq(value.tm_microsec, 750000);
}

/**
 * @brief Tests that mktime() rejects an out-of-range microsecond value.
 */
void test_mktime_invalid_microsec_negative()
{
    xer::tm value {};
    value.tm_year = 70;
    value.tm_mon = 0;
    value.tm_mday = 1;
    value.tm_hour = 0;
    value.tm_min = 0;
    value.tm_sec = 0;
    value.tm_microsec = -1;

    const auto result = xer::mktime(value);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

/**
 * @brief Tests that mktime() rejects an out-of-range microsecond value above
 *        the valid maximum.
 */
void test_mktime_invalid_microsec_too_large()
{
    xer::tm value {};
    value.tm_year = 70;
    value.tm_mon = 0;
    value.tm_mday = 1;
    value.tm_hour = 0;
    value.tm_min = 0;
    value.tm_sec = 0;
    value.tm_microsec = 1000000;

    const auto result = xer::mktime(value);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

/**
 * @brief Tests that mktime() converts the local Unix epoch correctly.
 *
 * The result depends on the local time zone, so this test uses localtime() to
 * construct a valid local broken-down time first and then checks round-trip
 * reconstruction.
 */
void test_mktime_localtime_roundtrip_integral()
{
    const auto local = xer::localtime(123456789.0);
    xer_assert(local.has_value());

    const auto result = xer::mktime(local.value());

    xer_assert(result.has_value());
    xer_assert(std::abs(result.value() - 123456789.0) < time_epsilon);
}

/**
 * @brief Tests that mktime() preserves the microsecond part in round-trip
 *        conversion through localtime().
 *
 * This test uses a fractional value that is exactly representable in binary
 * floating point.
 */
void test_mktime_localtime_roundtrip_fractional()
{
    const double original = 123456789.625;

    const auto local = xer::localtime(original);
    xer_assert(local.has_value());

    const auto result = xer::mktime(local.value());

    xer_assert(result.has_value());
    xer_assert(std::abs(result.value() - original) < time_epsilon);
}

/**
 * @brief Tests that mktime() rejects a broken-down time before the Unix epoch.
 */
void test_mktime_before_epoch()
{
    xer::tm value {};
    value.tm_year = 69;
    value.tm_mon = 11;
    value.tm_mday = 31;
    value.tm_hour = 23;
    value.tm_min = 59;
    value.tm_sec = 59;
    value.tm_isdst = -1;
    value.tm_microsec = 0;

    const auto result = xer::mktime(value);

    xer_assert(!result.has_value());
}

/**
 * @brief Tests a UTC round-trip using gmtime() and field comparison.
 *
 * Since mktime() interprets its input as local time, we cannot directly compare
 * gmtime() followed by mktime(). Instead, we validate that gmtime() returns the
 * expected UTC fields for a known timestamp.
 */
void test_gmtime_known_timestamp()
{
    const auto result = xer::gmtime(946684800.125);

    xer_assert(result.has_value());

    const xer::tm& value = result.value();
    xer_assert_eq(value.tm_year, 100);
    xer_assert_eq(value.tm_mon, 0);
    xer_assert_eq(value.tm_mday, 1);
    xer_assert_eq(value.tm_hour, 0);
    xer_assert_eq(value.tm_min, 0);
    xer_assert_eq(value.tm_sec, 0);
    xer_assert_eq(value.tm_wday, 6);
    xer_assert_eq(value.tm_yday, 0);
    xer_assert_eq(value.tm_microsec, 125000);
}

} // namespace

int main()
{
    test_gmtime_negative_time();
    test_localtime_negative_time();
    test_gmtime_epoch();
    test_gmtime_microseconds();
    test_gmtime_next_day();
    test_localtime_epoch_and_microseconds();
    test_mktime_invalid_microsec_negative();
    test_mktime_invalid_microsec_too_large();
    test_mktime_localtime_roundtrip_integral();
    test_mktime_localtime_roundtrip_fractional();
    test_mktime_before_epoch();
    test_gmtime_known_timestamp();

    return 0;
}

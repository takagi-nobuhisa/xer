/**
 * @file tests/test_time_clock.cpp
 * @brief Tests for xer/bits/time_clock.h.
 */

#include <cmath>
#include <cstdint>
#include <limits>

#include <xer/assert.h>
#include <xer/bits/time_clock.h>

namespace {

/**
 * @brief Consumes some CPU time for clock() tests.
 */
volatile std::uint64_t g_sink = 0;

/**
 * @brief Tests that time() succeeds and returns a finite non-negative value.
 */
void test_time_returns_non_negative_finite_value()
{
    const auto result = xer::time();

    xer_assert(result.has_value());
    xer_assert(std::isfinite(result.value()));
    xer_assert(result.value() >= 0.0);
}

/**
 * @brief Tests that repeated time() calls do not go backwards.
 */
void test_time_is_non_decreasing()
{
    const auto first = xer::time();
    xer_assert(first.has_value());

    const auto second = xer::time();
    xer_assert(second.has_value());

    xer_assert(second.value() >= first.value());
}

/**
 * @brief Tests that clock() succeeds.
 */
void test_clock_succeeds()
{
    const auto result = xer::clock();

    xer_assert(result.has_value());
    xer_assert(result.value() != static_cast<xer::clock_t>(-1));
}

/**
 * @brief Tests that clock() is non-decreasing across CPU work.
 */
void test_clock_is_non_decreasing()
{
    const auto before = xer::clock();
    xer_assert(before.has_value());

    for (std::uint64_t index = 0; index < UINT64_C(1000000); ++index) {
        g_sink += index;
    }

    const auto after = xer::clock();
    xer_assert(after.has_value());

    xer_assert(after.value() >= before.value());
}

/**
 * @brief Tests difftime() with integer-second values.
 */
void test_difftime_integer_values()
{
    xer_assert_eq(xer::difftime(10.0, 3.0), 7.0);
    xer_assert_eq(xer::difftime(3.0, 10.0), -7.0);
    xer_assert_eq(xer::difftime(5.0, 5.0), 0.0);
}

/**
 * @brief Tests difftime() with fractional-second values.
 */
void test_difftime_fractional_values()
{
    constexpr double epsilon = 1e-12;

    const double value1 = xer::difftime(12.75, 10.5);
    const double value2 = xer::difftime(10.5, 12.75);
    const double value3 = xer::difftime(100.125, 99.875);

    xer_assert(std::abs(value1 - 2.25) < epsilon);
    xer_assert(std::abs(value2 - (-2.25)) < epsilon);
    xer_assert(std::abs(value3 - 0.25) < epsilon);
}

} // namespace

int main()
{
    test_time_returns_non_negative_finite_value();
    test_time_is_non_decreasing();
    test_clock_succeeds();
    test_clock_is_non_decreasing();
    test_difftime_integer_values();
    test_difftime_fractional_values();

    return 0;
}

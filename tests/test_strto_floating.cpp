/**
 * @file tests/test_strto_floating.cpp
 * @brief Execution tests for xer/bits/strto_floating.h.
 */

#include <cmath>
#include <limits>
#include <string>
#include <string_view>

#include <xer/assert.h>
#include <xer/bits/strto_floating.h>

namespace {

/**
 * @brief Tests decimal floating-point conversion by strtod.
 */
void test_strtod_decimal()
{
    std::u8string_view::const_iterator endit;
    constexpr std::u8string_view input = u8"123.5xyz";
    const auto value = xer::strtod(input, &endit);

    xer_assert(value.has_value());
    xer_assert(std::fabs(*value - 123.5) < 1e-12);
    xer_assert(endit == input.begin() + 5);
}

/**
 * @brief Tests whitespace and sign handling by strto.
 */
void test_strto_space_and_sign()
{
    const auto value = xer::strto<double>(u8" \t\n-42.25rest");

    xer_assert(value.has_value());
    xer_assert(std::fabs(*value + 42.25) < 1e-12);
}

/**
 * @brief Tests hexadecimal floating-point conversion.
 */
void test_strto_hexadecimal_floating()
{
    const char8_t* endptr = nullptr;
    constexpr const char8_t* input = u8"0x1.8p+1!";
    const auto value = xer::strto<double>(input, &endptr);

    xer_assert(value.has_value());
    xer_assert(std::fabs(*value - 3.0) < 1e-12);
    xer_assert(endptr == input + 8);
}

/**
 * @brief Tests infinity parsing.
 */
void test_strto_infinity()
{
    const auto value = xer::strto<double>(u8"-infinity");

    xer_assert(value.has_value());
    xer_assert(std::isinf(*value));
    xer_assert(*value < 0.0);
}

/**
 * @brief Tests NaN parsing.
 */
void test_strto_nan()
{
    std::u8string input = u8"nan(payload)tail";
    std::u8string::iterator endit;
    const auto value = xer::strto<long double>(input, &endit);

    xer_assert(value.has_value());
    xer_assert(std::isnan(*value));
    xer_assert(endit == input.begin() + 12);
}

/**
 * @brief Tests the no-digit case by strto.
 */
void test_strto_no_digits()
{
    constexpr std::u8string_view input = u8"xyz";
    std::u8string_view::const_iterator endit;
    const auto value = xer::strto<double>(input, &endit);

    xer_assert(value.has_value());
    xer_assert_eq(*value, 0.0);
    xer_assert(endit == input.begin());
}

/**
 * @brief Tests range error by overflow.
 */
void test_strto_range_error_overflow()
{
    const auto value = xer::strto<double>(u8"1e100000");

    xer_assert_not(value.has_value());
    xer_assert_eq(value.error().code, xer::error_t::range);
}

/**
 * @brief Tests range error by underflow.
 */
void test_strto_range_error_underflow()
{
    const auto value = xer::strto<float>(u8"1e-100000");

    xer_assert_not(value.has_value());
    xer_assert_eq(value.error().code, xer::error_t::range);
}

/**
 * @brief Tests char8_t pointer overloads.
 */
void test_pointer_overloads()
{
    constexpr const char8_t* const_input = u8"0.5rest";
    const char8_t* const_endptr = nullptr;
    const auto const_value = xer::strtof(const_input, &const_endptr);

    xer_assert(const_value.has_value());
    xer_assert(std::fabs(*const_value - 0.5f) < 1e-6f);
    xer_assert(const_endptr == const_input + 3);

    char8_t mutable_input[] = u8"2.25!";
    char8_t* mutable_endptr = nullptr;
    const auto mutable_value = xer::strtod(mutable_input, &mutable_endptr);

    xer_assert(mutable_value.has_value());
    xer_assert(std::fabs(*mutable_value - 2.25) < 1e-12);
    xer_assert(mutable_endptr == mutable_input + 4);
}

/**
 * @brief Tests wrapper functions for fixed-width aliases.
 */
void test_fixed_width_wrappers()
{
    const auto value32 = xer::strtof32(u8"1.25");
    const auto value64 = xer::strtof64(u8"1.5");

    xer_assert(value32.has_value());
    xer_assert(value64.has_value());
    xer_assert(std::fabs(*value32 - 1.25f) < 1e-6f);
    xer_assert(std::fabs(*value64 - 1.5) < 1e-12);
}

} // namespace

int main()
{
    test_strtod_decimal();
    test_strto_space_and_sign();
    test_strto_hexadecimal_floating();
    test_strto_infinity();
    test_strto_nan();
    test_strto_no_digits();
    test_strto_range_error_overflow();
    test_strto_range_error_underflow();
    test_pointer_overloads();
    test_fixed_width_wrappers();

    return 0;
}

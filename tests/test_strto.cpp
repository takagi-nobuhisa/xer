/**
 * @file tests/test_strto.cpp
 * @brief Execution tests for xer/bits/strto.h.
 */

#include <limits>
#include <string>
#include <string_view>

#include <xer/assert.h>
#include <xer/bits/strto.h>

namespace {

/**
 * @brief Tests successful decimal conversion by atoi.
 */
void test_atoi_decimal()
{
    const auto value = xer::atoi(u8"12345");

    xer_assert(value.has_value());
    xer_assert_eq(*value, 12345);
}

/**
 * @brief Tests whitespace and sign handling by atoi.
 */
void test_atoi_space_and_sign()
{
    const auto value = xer::atoi(u8" \t\n-42xyz");

    xer_assert(value.has_value());
    xer_assert_eq(*value, -42);
}

/**
 * @brief Tests the no-digit case by atoi.
 */
void test_atoi_no_digits()
{
    const auto value = xer::atoi(u8"   +xyz");

    xer_assert(value.has_value());
    xer_assert_eq(*value, 0);
}

/**
 * @brief Tests overflow detection by atoi.
 */
void test_atoi_overflow()
{
    const auto value = xer::atoi(u8"999999999999999999999999999999999999");

    xer_assert_not(value.has_value());
    xer_assert_eq(value.error().code, xer::error_t::range);
}

/**
 * @brief Tests binary prefix auto-detection.
 */
void test_strto_binary_auto_base()
{
    std::u8string_view::const_iterator endit;
    constexpr std::u8string_view input = u8"0b101101rest";
    const auto value = xer::strto<int>(input, &endit);

    xer_assert(value.has_value());
    xer_assert_eq(*value, 45);
    xer_assert(endit == input.begin() + 8);
}

/**
 * @brief Tests binary prefix acceptance with explicit base two.
 */
void test_strto_binary_explicit_base()
{
    const char8_t* endptr = nullptr;
    constexpr const char8_t* input = u8"0B1110!";
    const auto value = xer::strto<int>(input, &endptr, 2);

    xer_assert(value.has_value());
    xer_assert_eq(*value, 14);
    xer_assert(endptr == input + 6);
}

/**
 * @brief Tests hexadecimal and octal auto-detection.
 */
void test_strto_auto_base_prefixes()
{
    const auto octal = xer::strto<int>(u8"077");
    const auto hex = xer::strto<int>(u8"0x20");

    xer_assert(octal.has_value());
    xer_assert(hex.has_value());
    xer_assert_eq(*octal, 63);
    xer_assert_eq(*hex, 32);
}

/**
 * @brief Tests end iterator for std::u8string input.
 */
void test_strto_u8string_endit()
{
    std::u8string input = u8"123abc";
    std::u8string::iterator endit;
    const auto value = xer::strto<long>(input, &endit, 10);

    xer_assert(value.has_value());
    xer_assert_eq(*value, 123L);
    xer_assert(endit == input.begin() + 3);
}

/**
 * @brief Tests the no-digit case by strto.
 */
void test_strto_no_digits()
{
    constexpr std::u8string_view input = u8"xyz";
    std::u8string_view::const_iterator endit;
    const auto value = xer::strto<long>(input, &endit, 10);

    xer_assert(value.has_value());
    xer_assert_eq(*value, 0L);
    xer_assert(endit == input.begin());
}

/**
 * @brief Tests invalid base handling.
 */
void test_strto_invalid_base()
{
    const auto value = xer::strto<int>(u8"10", nullptr, 1);

    xer_assert_not(value.has_value());
    xer_assert_eq(value.error().code, xer::error_t::invalid_argument);
}

/**
 * @brief Tests unsigned conversion with leading minus sign.
 */
void test_strtoul_negative_input()
{
    const auto value = xer::strtoul(u8"-1", nullptr, 10);

    xer_assert(value.has_value());
    xer_assert_eq(*value, std::numeric_limits<unsigned long>::max());
}

/**
 * @brief Tests range error by strtoull.
 */
void test_strtoull_range_error()
{
    const auto value = xer::strtoull(
        u8"184467440737095516160",
        nullptr,
        10);

    xer_assert_not(value.has_value());
    xer_assert_eq(value.error().code, xer::error_t::range);
}

/**
 * @brief Tests wrapper functions.
 */
void test_wrapper_functions()
{
    const auto value_int = xer::atoi(u8"17");
    const auto value_long = xer::atol(u8"18");
    const auto value_long_long = xer::atoll(u8"19");
    const auto value_strtol = xer::strtol(u8"20");
    const auto value_strtoll = xer::strtoll(u8"21");
    const auto value_strtoull = xer::strtoull(u8"22");

    xer_assert(value_int.has_value());
    xer_assert(value_long.has_value());
    xer_assert(value_long_long.has_value());
    xer_assert(value_strtol.has_value());
    xer_assert(value_strtoll.has_value());
    xer_assert(value_strtoull.has_value());

    xer_assert_eq(*value_int, 17);
    xer_assert_eq(*value_long, 18L);
    xer_assert_eq(*value_long_long, 19LL);
    xer_assert_eq(*value_strtol, 20L);
    xer_assert_eq(*value_strtoll, 21LL);
    xer_assert_eq(*value_strtoull, 22ULL);
}

} // namespace

int main()
{
    test_atoi_decimal();
    test_atoi_space_and_sign();
    test_atoi_no_digits();
    test_atoi_overflow();
    test_strto_binary_auto_base();
    test_strto_binary_explicit_base();
    test_strto_auto_base_prefixes();
    test_strto_u8string_endit();
    test_strto_no_digits();
    test_strto_invalid_base();
    test_strtoul_negative_input();
    test_strtoull_range_error();
    test_wrapper_functions();

    return 0;
}

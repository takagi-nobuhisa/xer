/**
 * @file tests/test_string_compare.cpp
 * @brief Runtime tests for xer/bits/string_compare.h.
 */

#include <cstddef>
#include <string_view>

#include <xer/assert.h>
#include <xer/bits/string_compare.h>
#include <xer/error.h>

namespace {

void test_strcasecmp_ascii_equal()
{
    constexpr std::u8string_view lhs = u8"Alpha";
    constexpr std::u8string_view rhs = u8"aLPHA";

    const auto result = xer::strcasecmp(lhs, rhs);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 0);
}

void test_strcasecmp_ascii_less()
{
    constexpr std::u8string_view lhs = u8"Alpha";
    constexpr std::u8string_view rhs = u8"Bravo";

    const auto result = xer::strcasecmp(lhs, rhs);

    xer_assert(result.has_value());
    xer_assert_eq(*result, -1);
}

void test_strcasecmp_non_ascii_rejected()
{
    constexpr std::u8string_view lhs = u8"Ä";
    constexpr std::u8string_view rhs = u8"ä";

    const auto result = xer::strcasecmp(lhs, rhs);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_strncasecmp_uses_code_point_count()
{
    constexpr std::u8string_view lhs = u8"Äb";
    constexpr std::u8string_view rhs = u8"äc";

    const auto result = xer::strncasecmp(lhs, rhs, 1);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_stricmp_latin1_lowercase_equal_utf8()
{
    constexpr std::u8string_view lhs = u8"ÄÖÜ";
    constexpr std::u8string_view rhs = u8"äöü";

    const auto result = xer::stricmp(lhs, rhs, xer::ctrans_id::latin1_lower);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 0);
}

void test_strnicmp_latin1_uppercase_prefix_equal()
{
    constexpr std::u8string_view lhs = u8"äöx";
    constexpr std::u8string_view rhs = u8"ÄÖy";

    const auto result = xer::strnicmp(lhs, rhs, 2, xer::ctrans_id::latin1_upper);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 0);
}

void test_strcasechr_code_unit_found()
{
    constexpr std::string_view text = "Alpha";

    const auto result = xer::strcasechr(text, 'a');

    xer_assert(result.has_value());
    xer_assert_eq(result.value() - text.begin(), 0);
}

void test_strcaserchr_code_unit_found()
{
    constexpr std::string_view text = "Alpha";

    const auto result = xer::strcaserchr(text, 'A');

    xer_assert(result.has_value());
    xer_assert_eq(result.value() - text.begin(), 4);
}

void test_strichr_utf8_code_point_found()
{
    constexpr std::u8string_view text = u8"Äbcä";

    const auto result = xer::strichr(text, U'ä', xer::ctrans_id::latin1_lower);

    xer_assert(result.has_value());
    xer_assert_eq(result.value() - text.begin(), 0);
}

void test_strirchr_utf8_code_point_found()
{
    constexpr std::u8string_view text = u8"Äbcä";

    const auto result = xer::strirchr(text, U'Ä', xer::ctrans_id::latin1_upper);

    xer_assert(result.has_value());
    xer_assert_eq(result.value() - text.begin(), 4);
}

void test_strcasestr_ascii_found()
{
    constexpr std::u8string_view text = u8"xxAlphaYY";
    constexpr std::u8string_view pattern = u8"aLPHa";

    const auto result = xer::strcasestr(text, pattern);

    xer_assert(result.has_value());
    xer_assert_eq(result.value() - text.begin(), 2);
}

void test_strcasestr_empty_pattern_returns_begin()
{
    constexpr std::u8string_view text = u8"Alpha";
    constexpr std::u8string_view pattern = u8"";

    const auto result = xer::strcasestr(text, pattern);

    xer_assert(result.has_value());
    xer_assert_eq(result.value() - text.begin(), 0);
}

void test_stristr_utf8_latin1_found()
{
    constexpr std::u8string_view text = u8"xxÄÖÜyy";
    constexpr std::u8string_view pattern = u8"äöü";

    const auto result = xer::stristr(text, pattern, xer::ctrans_id::latin1_lower);

    xer_assert(result.has_value());
    xer_assert_eq(result.value() - text.begin(), 2);
}

void test_strcaserstr_ascii_found()
{
    constexpr std::u8string_view text = u8"AbcaBC";
    constexpr std::u8string_view pattern = u8"abc";

    const auto result = xer::strcaserstr(text, pattern);

    xer_assert(result.has_value());
    xer_assert_eq(result.value() - text.begin(), 3);
}

void test_strirstr_utf8_latin1_found()
{
    constexpr std::u8string_view text = u8"Äxäx";
    constexpr std::u8string_view pattern = u8"äx";

    const auto result = xer::strirstr(text, pattern, xer::ctrans_id::latin1_lower);

    xer_assert(result.has_value());
    xer_assert_eq(result.value() - text.begin(), 3);
}

void test_strcasepos_ascii_found()
{
    constexpr std::u8string_view text = u8"xxAlpha";
    constexpr std::u8string_view pattern = u8"ALPHA";

    const auto result = xer::strcasepos(text, pattern);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(2));
}

void test_strcasepos_empty_pattern_returns_zero()
{
    constexpr std::u8string_view text = u8"Alpha";
    constexpr std::u8string_view pattern = u8"";

    const auto result = xer::strcasepos(text, pattern);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(0));
}

void test_stripos_utf8_latin1_returns_code_unit_position()
{
    constexpr std::u8string_view text = u8"Äxxä";
    constexpr std::u8string_view pattern = u8"ä";

    const auto result = xer::stripos(text, pattern, xer::ctrans_id::latin1_lower);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(0));
}

void test_strcaserpos_ascii_found()
{
    constexpr std::u8string_view text = u8"AbcaBC";
    constexpr std::u8string_view pattern = u8"abc";

    const auto result = xer::strcaserpos(text, pattern);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(3));
}

void test_strirpos_utf8_latin1_returns_last_code_unit_position()
{
    constexpr std::u8string_view text = u8"Äxxä";
    constexpr std::u8string_view pattern = u8"ä";

    const auto result = xer::strirpos(text, pattern, xer::ctrans_id::latin1_lower);

    xer_assert(result.has_value());
    xer_assert_eq(*result, static_cast<std::size_t>(4));
}

void test_stripos_not_found()
{
    constexpr std::u8string_view text = u8"Alpha";
    constexpr std::u8string_view pattern = u8"Zulu";

    const auto result = xer::stripos(text, pattern, xer::ctrans_id::lower);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::not_found);
}

} // namespace

int main()
{
    test_strcasecmp_ascii_equal();
    test_strcasecmp_ascii_less();
    test_strcasecmp_non_ascii_rejected();
    test_strncasecmp_uses_code_point_count();

    test_stricmp_latin1_lowercase_equal_utf8();
    test_strnicmp_latin1_uppercase_prefix_equal();

    test_strcasechr_code_unit_found();
    test_strcaserchr_code_unit_found();
    test_strichr_utf8_code_point_found();
    test_strirchr_utf8_code_point_found();

    test_strcasestr_ascii_found();
    test_strcasestr_empty_pattern_returns_begin();
    test_stristr_utf8_latin1_found();
    test_strcaserstr_ascii_found();
    test_strirstr_utf8_latin1_found();

    test_strcasepos_ascii_found();
    test_strcasepos_empty_pattern_returns_zero();
    test_stripos_utf8_latin1_returns_code_unit_position();
    test_strcaserpos_ascii_found();
    test_strirpos_utf8_latin1_returns_last_code_unit_position();
    test_stripos_not_found();
}

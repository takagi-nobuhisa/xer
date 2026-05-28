/**
 * @file tests/test_itostr.cpp
 * @brief Execution tests for xer/bits/itostr.h.
 */

#include <cstdint>
#include <limits>
#include <string>

#include <xer/assert.h>
#include <xer/stdlib.h>

namespace {

void test_itostr_decimal()
{
    std::u8string str;
    const auto result = xer::itostr(12345, str);

    xer_assert(result.has_value());
    xer_assert(*result == &str);
    xer_assert_eq(str, std::u8string(u8"12345"));
}

void test_itostr_negative()
{
    std::u8string str;
    const auto result = xer::itostr(-42, str);

    xer_assert(result.has_value());
    xer_assert_eq(str, std::u8string(u8"-42"));
}

void test_itostr_minimum_value()
{
    std::u8string str;
    const auto result = xer::itostr(std::numeric_limits<std::int64_t>::min(), str);

    xer_assert(result.has_value());
    xer_assert_eq(str, std::u8string(u8"-9223372036854775808"));
}

void test_itostr_radix()
{
    std::u8string binary;
    std::u8string octal;
    std::u8string hex;
    std::u8string base36;

    const auto r1 = xer::itostr(255, binary, 2);
    const auto r2 = xer::itostr(255, octal, 8);
    const auto r3 = xer::itostr(255, hex, 16);
    const auto r4 = xer::itostr(35, base36, 36);

    xer_assert(r1.has_value());
    xer_assert(r2.has_value());
    xer_assert(r3.has_value());
    xer_assert(r4.has_value());
    xer_assert_eq(binary, std::u8string(u8"11111111"));
    xer_assert_eq(octal, std::u8string(u8"377"));
    xer_assert_eq(hex, std::u8string(u8"ff"));
    xer_assert_eq(base36, std::u8string(u8"z"));
}

void test_itostr_invalid_radix_keeps_string()
{
    std::u8string str = u8"unchanged";
    const auto result = xer::itostr(10, str, 1);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
    xer_assert_eq(str, std::u8string(u8"unchanged"));
}

void test_itoa_alias()
{
    std::u8string str;
    const auto result = xer::itoa(255U, str, 16);

    xer_assert(result.has_value());
    xer_assert_eq(str, std::u8string(u8"ff"));
}

void test_itostr_char_buffer()
{
    char8_t buffer[4]{};
    const auto result = xer::itostr(255, buffer, 16);

    xer_assert(result.has_value());
    xer_assert(*result == buffer);
    xer_assert_eq(std::u8string(buffer), std::u8string(u8"ff"));
}

void test_itostr_char_buffer_too_small()
{
    char8_t buffer[3] = u8"xx";
    const auto result = xer::itostr(255, buffer, 10);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::length_error);
    xer_assert_eq(buffer[0], u8'x');
    xer_assert_eq(buffer[1], u8'x');
}

void test_itostr_supported_character_types()
{
    std::string narrow;
    std::u8string utf8;
    std::wstring wide;
    std::u16string utf16;
    std::u32string utf32;

    const auto r1 = xer::itostr(255, narrow, 16);
    const auto r2 = xer::itostr(255, utf8, 16);
    const auto r3 = xer::itostr(255, wide, 16);
    const auto r4 = xer::itostr(255, utf16, 16);
    const auto r5 = xer::itostr(255, utf32, 16);

    xer_assert(r1.has_value());
    xer_assert(r2.has_value());
    xer_assert(r3.has_value());
    xer_assert(r4.has_value());
    xer_assert(r5.has_value());

    xer_assert_eq(narrow, std::string("ff"));
    xer_assert_eq(utf8, std::u8string(u8"ff"));
    xer_assert_eq(wide, std::wstring(L"ff"));
    xer_assert_eq(utf16, std::u16string(u"ff"));
    xer_assert_eq(utf32, std::u32string(U"ff"));
}

void test_itoa_supported_character_buffers()
{
    char narrow[8]{};
    wchar_t wide[8]{};
    char16_t utf16[8]{};
    char32_t utf32[8]{};

    const auto r1 = xer::itoa(-10, narrow, 10);
    const auto r2 = xer::itoa(-10, wide, 10);
    const auto r3 = xer::itoa(-10, utf16, 10);
    const auto r4 = xer::itoa(-10, utf32, 10);

    xer_assert(r1.has_value());
    xer_assert(r2.has_value());
    xer_assert(r3.has_value());
    xer_assert(r4.has_value());

    xer_assert_eq(std::string(narrow), std::string("-10"));
    xer_assert_eq(std::wstring(wide), std::wstring(L"-10"));
    xer_assert_eq(std::u16string(utf16), std::u16string(u"-10"));
    xer_assert_eq(std::u32string(utf32), std::u32string(U"-10"));
}

} // namespace

auto main() -> int
{
    test_itostr_decimal();
    test_itostr_negative();
    test_itostr_minimum_value();
    test_itostr_radix();
    test_itostr_invalid_radix_keeps_string();
    test_itoa_alias();
    test_itostr_char_buffer();
    test_itostr_char_buffer_too_small();
    test_itostr_supported_character_types();
    test_itoa_supported_character_buffers();
    return 0;
}

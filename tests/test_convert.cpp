/**
 * @file tests/test_convert.cpp
 * @brief Execution tests for xer/convert.h.
 */

#include <cstdint>
#include <limits>
#include <string>
#include <string_view>

#include <xer/assert.h>
#include <xer/convert.h>
#include <xer/path.h>

namespace {

void test_same_type()
{
    const auto result = xer::to<int>(123);

    xer_assert(result.has_value());
    xer_assert_eq(*result, 123);
}

void test_arithmetic_range_checked()
{
    const auto ok = xer::to<unsigned char>(255);
    const auto too_large = xer::to<unsigned char>(256);
    const auto negative = xer::to<unsigned>(-1);

    xer_assert(ok.has_value());
    xer_assert_eq(*ok, static_cast<unsigned char>(255));

    xer_assert_not(too_large.has_value());
    xer_assert_eq(too_large.error().code, xer::error_t::range);

    xer_assert_not(negative.has_value());
    xer_assert_eq(negative.error().code, xer::error_t::range);
}

void test_signed_and_unsigned_char_are_integers()
{
    const auto signed_value = xer::to<int>(static_cast<signed char>(-5));
    const auto unsigned_value = xer::to<int>(static_cast<unsigned char>(250));
    const auto parsed = xer::to<unsigned char>(u8"255");

    xer_assert(signed_value.has_value());
    xer_assert_eq(*signed_value, -5);

    xer_assert(unsigned_value.has_value());
    xer_assert_eq(*unsigned_value, 250);

    xer_assert(parsed.has_value());
    xer_assert_eq(*parsed, static_cast<unsigned char>(255));
}

void test_char_is_character()
{
    const auto text = xer::to<std::u8string>('A');
    const auto ch = xer::to<char>(u8"A");
    const auto number = xer::to<int>('A');

    xer_assert(text.has_value());
    xer_assert_eq(*text, std::u8string(u8"A"));

    xer_assert(ch.has_value());
    xer_assert_eq(*ch, 'A');

    xer_assert_not(number.has_value());
    xer_assert_eq(number.error().code, xer::error_t::invalid_argument);
}

void test_utf_text_to_numbers()
{
    const auto i8 = xer::to<int>(u8"123");
    const auto i16 = xer::to<int>(u"123");
    const auto i32 = xer::to<int>(U"123");
    const auto iw = xer::to<int>(L"123");
    const auto d = xer::to<double>(u8"3.5");
    const auto trailing = xer::to<int>(u8"123x");

    xer_assert(i8.has_value());
    xer_assert(i16.has_value());
    xer_assert(i32.has_value());
    xer_assert(iw.has_value());
    xer_assert(d.has_value());

    xer_assert_eq(*i8, 123);
    xer_assert_eq(*i16, 123);
    xer_assert_eq(*i32, 123);
    xer_assert_eq(*iw, 123);
    xer_assert(*d > 3.49 && *d < 3.51);

    xer_assert_not(trailing.has_value());
    xer_assert_eq(trailing.error().code, xer::error_t::invalid_argument);
}

void test_narrow_char_strings_are_not_interpreted()
{
    const auto i = xer::to<int>("123");
    const auto s = xer::to<std::u8string>(std::string("abc"));
    const auto p = xer::to<xer::path>("a/b");

    xer_assert_not(i.has_value());
    xer_assert_eq(i.error().code, xer::error_t::invalid_argument);

    xer_assert_not(s.has_value());
    xer_assert_eq(s.error().code, xer::error_t::invalid_argument);

    xer_assert_not(p.has_value());
    xer_assert_eq(p.error().code, xer::error_t::invalid_argument);
}

void test_arithmetic_to_text()
{
    const auto u8 = xer::to<std::u8string>(123);
    const auto u16 = xer::to<std::u16string>(123);
    const auto u32 = xer::to<std::u32string>(123);
    const auto w = xer::to<std::wstring>(123);

    xer_assert(u8.has_value());
    xer_assert(u16.has_value());
    xer_assert(u32.has_value());
    xer_assert(w.has_value());

    xer_assert_eq(*u8, std::u8string(u8"123"));
    xer_assert_eq(*u16, std::u16string(u"123"));
    xer_assert_eq(*u32, std::u32string(U"123"));
    xer_assert_eq(*w, std::wstring(L"123"));
}

void test_unicode_text_conversion()
{
    const auto u8 = xer::to<std::u8string>(u"あ");
    const auto u16 = xer::to<std::u16string>(u8"あ");
    const auto u32 = xer::to<std::u32string>(u8"あ");
    const auto w = xer::to<std::wstring>(u8"あ");

    xer_assert(u8.has_value());
    xer_assert(u16.has_value());
    xer_assert(u32.has_value());
    xer_assert(w.has_value());

    xer_assert_eq(*u8, std::u8string(u8"あ"));
    xer_assert_eq(*u16, std::u16string(u"あ"));
    xer_assert_eq(*u32, std::u32string(U"あ"));
    xer_assert_eq(*w, std::wstring(L"あ"));
}

void test_character_conversion()
{
    const auto c32 = xer::to<char32_t>(u8"あ");
    const auto c16 = xer::to<char16_t>(u"あ");
    const auto c8_fail = xer::to<char8_t>(u8"あ");

    xer_assert(c32.has_value());
    xer_assert_eq(*c32, U'あ');

    xer_assert(c16.has_value());
    xer_assert_eq(*c16, u'あ');

    xer_assert_not(c8_fail.has_value());
    xer_assert_eq(c8_fail.error().code, xer::error_t::range);
}

void test_path_conversion()
{
    const auto p1 = xer::to<xer::path>(u8"a\\b");
    const auto p2 = xer::to<xer::path>(u"a\\b");
    const auto p3 = xer::to<xer::path>(U"a\\b");
    const auto p4 = xer::to<xer::path>(L"a\\b");

    xer_assert(p1.has_value());
    xer_assert(p2.has_value());
    xer_assert(p3.has_value());
    xer_assert(p4.has_value());

    xer_assert_eq(p1->str(), std::u8string_view(u8"a/b"));
    xer_assert_eq(p2->str(), std::u8string_view(u8"a/b"));
    xer_assert_eq(p3->str(), std::u8string_view(u8"a/b"));
    xer_assert_eq(p4->str(), std::u8string_view(u8"a/b"));
}

void test_path_to_text()
{
    const xer::path p(u8"a\\b");
    const auto text = xer::to<std::u8string>(p);

    xer_assert(text.has_value());
    xer_assert_eq(*text, std::u8string(u8"a/b"));
}

} // namespace

auto main() -> int
{
    test_same_type();
    test_arithmetic_range_checked();
    test_signed_and_unsigned_char_are_integers();
    test_char_is_character();
    test_utf_text_to_numbers();
    test_narrow_char_strings_are_not_interpreted();
    test_arithmetic_to_text();
    test_unicode_text_conversion();
    test_character_conversion();
    test_path_conversion();
    test_path_to_text();
    return 0;
}

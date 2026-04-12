#include <array>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

#include <xer/assert.h>
#include <xer/string.h>

namespace {

void test_implode_empty_vector()
{
    const std::vector<std::u8string> parts;
    const auto result = xer::implode(u8",", parts);

    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8""));
}

void test_implode_single_element()
{
    const std::vector<std::u8string> parts = {
        u8"abc",
    };
    const auto result = xer::implode(u8",", parts);

    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"abc"));
}

void test_implode_multiple_u8strings()
{
    const std::vector<std::u8string> parts = {
        u8"a",
        u8"b",
        u8"c",
    };
    const auto result = xer::implode(u8",", parts);

    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"a,b,c"));
}

void test_implode_empty_separator()
{
    const std::vector<std::u8string> parts = {
        u8"a",
        u8"b",
        u8"c",
    };
    const auto result = xer::implode(u8"", parts);

    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"abc"));
}

void test_implode_preserves_empty_elements()
{
    const std::vector<std::u8string> parts = {
        u8"",
        u8"a",
        u8"",
        u8"b",
        u8"",
    };
    const auto result = xer::implode(u8",", parts);

    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8",a,,b,"));
}

void test_implode_u8string_view_vector()
{
    const std::vector<std::u8string_view> parts = {
        u8"left",
        u8"middle",
        u8"right",
    };
    const auto result = xer::implode(u8"/", parts);

    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"left/middle/right"));
}

void test_implode_array()
{
    const std::array<std::u8string_view, 3> parts = {
        u8"2026",
        u8"04",
        u8"12",
    };
    const auto result = xer::implode(u8"-", parts);

    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"2026-04-12"));
}

void test_implode_initializer_list()
{
    const std::initializer_list<std::u8string_view> parts = {
        u8"alpha",
        u8"beta",
        u8"gamma",
    };
    const auto result = xer::implode(u8"::", parts);

    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"alpha::beta::gamma"));
}

void test_implode_utf8_text()
{
    const std::vector<std::u8string_view> parts = {
        u8"あ",
        u8"い",
        u8"う",
    };
    const auto result = xer::implode(u8"☆", parts);

    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"あ☆い☆う"));
}

void test_implode_with_empty_piece_only()
{
    const std::vector<std::u8string_view> parts = {
        u8"",
    };
    const auto result = xer::implode(u8",", parts);

    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8""));
}

void test_implode_longer_separator()
{
    const std::vector<std::u8string_view> parts = {
        u8"a",
        u8"b",
        u8"c",
    };
    const auto result = xer::implode(u8" <-> ", parts);

    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"a <-> b <-> c"));
}

} // namespace

int main()
{
    test_implode_empty_vector();
    test_implode_single_element();
    test_implode_multiple_u8strings();
    test_implode_empty_separator();
    test_implode_preserves_empty_elements();
    test_implode_u8string_view_vector();
    test_implode_array();
    test_implode_initializer_list();
    test_implode_utf8_text();
    test_implode_with_empty_piece_only();
    test_implode_longer_separator();

    return 0;
}

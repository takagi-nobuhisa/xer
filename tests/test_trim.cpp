#include <string>
#include <string_view>
#include <expected>

#include <xer/assert.h>
#include <xer/error.h>
#include <xer/string.h>

namespace {

void test_trim_default_both_sides()
{
    constexpr char8_t source[] = {
        u8' ',
        u8'\t',
        u8'\r',
        u8'\n',
        u8'h',
        u8'e',
        u8'l',
        u8'l',
        u8'o',
        u8' ',
        u8'\v',
        u8'\0',
    };

    const auto result = xer::trim(std::u8string_view(source, sizeof(source)));
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"hello"));
}

void test_ltrim_default()
{
    const auto result = xer::ltrim(u8" \t\r\nhello");
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"hello"));
}

void test_rtrim_default()
{
    const auto result = xer::rtrim(u8"hello \t\r\n");
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"hello"));
}

void test_trim_no_change()
{
    const auto result = xer::trim(u8"hello");
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"hello"));
}

void test_trim_all_removed()
{
    const auto result = xer::trim(u8" \t\r\n\v");
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8""));
}

void test_trim_empty_string()
{
    const auto result = xer::trim(u8"");
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8""));
}

void test_trim_custom_character_list()
{
    const auto result = xer::trim(u8"..abcHello cab.", u8". abc");
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"Hello"));
}

void test_ltrim_custom_character_list()
{
    const auto result = xer::ltrim(u8"xyxyHello", u8"xy");
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"Hello"));
}

void test_rtrim_custom_character_list()
{
    const auto result = xer::rtrim(u8"Helloxyxy", u8"xy");
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"Hello"));
}

void test_trim_range_character_list()
{
    const auto result = xer::trim(u8"123abcXYZ321", u8"0..9");
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"abcXYZ"));
}

void test_trim_reverse_range_character_list()
{
    const auto result = xer::trim(u8"zyHelloxyz", u8"z..x");
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"Hello"));
}

void test_trim_utf8_text_default_does_not_trim_non_ascii()
{
    const auto result = xer::trim(u8"　あいう　");
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"　あいう　"));
}

void test_trim_view_basic()
{
    constexpr std::u8string_view source(u8"  hello  ");
    const auto result = xer::trim_view(source);

    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string_view(u8"hello"));
}

void test_ltrim_view_basic()
{
    constexpr std::u8string_view source(u8"  hello  ");
    const auto result = xer::ltrim_view(source);

    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string_view(u8"hello  "));
}

void test_rtrim_view_basic()
{
    constexpr std::u8string_view source(u8"  hello  ");
    const auto result = xer::rtrim_view(source);

    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string_view(u8"  hello"));
}

void test_trim_view_custom_character_list()
{
    constexpr std::u8string_view source(u8"xyHelloxy");
    const auto result = xer::trim_view(source, u8"xy");

    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string_view(u8"Hello"));
}

void test_trim_result_u8string_view_success()
{
    const xer::result<std::u8string_view> input = std::u8string_view(u8"  hello  ");
    const auto result = xer::trim(input);

    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"hello"));
}

void test_trim_result_u8string_success()
{
    const xer::result<std::u8string> input = std::u8string(u8"  hello  ");
    const auto result = xer::trim(input);

    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string(u8"hello"));
}

void test_ltrim_result_propagates_error()
{
    const xer::result<std::u8string_view> input =
        std::unexpected(xer::make_error(xer::error_t::invalid_argument));

    const auto result = xer::ltrim(input);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_rtrim_result_propagates_error()
{
    const xer::result<std::u8string> input =
        std::unexpected(xer::make_error(xer::error_t::io_error));

    const auto result = xer::rtrim(input);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::io_error);
}

void test_trim_view_result_success()
{
    const xer::result<std::u8string_view> input = std::u8string_view(u8"  hello  ");
    const auto result = xer::trim_view(input);

    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string_view(u8"hello"));
}

void test_trim_view_result_propagates_error()
{
    const xer::result<std::u8string_view> input =
        std::unexpected(xer::make_error(xer::error_t::invalid_argument));

    const auto result = xer::trim_view(input);

    xer_assert(!result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void test_trim_with_nul_in_default_character_set()
{
    constexpr char8_t source[] = {
        u8'\0',
        u8'\t',
        u8'a',
        u8'b',
        u8'c',
        u8'\r',
        u8'\0',
    };

    const std::u8string_view view(source, sizeof(source));
    const auto result = xer::trim(view);

    xer_assert(result.has_value());

    const std::u8string expected = {
        u8'a',
        u8'b',
        u8'c',
    };
    xer_assert_eq(*result, expected);
}

} // namespace

int main()
{
    test_trim_default_both_sides();
    test_ltrim_default();
    test_rtrim_default();
    test_trim_no_change();
    test_trim_all_removed();
    test_trim_empty_string();
    test_trim_custom_character_list();
    test_ltrim_custom_character_list();
    test_rtrim_custom_character_list();
    test_trim_range_character_list();
    test_trim_reverse_range_character_list();
    test_trim_utf8_text_default_does_not_trim_non_ascii();
    test_trim_view_basic();
    test_ltrim_view_basic();
    test_rtrim_view_basic();
    test_trim_view_custom_character_list();
    test_trim_result_u8string_view_success();
    test_trim_result_u8string_success();
    test_ltrim_result_propagates_error();
    test_rtrim_result_propagates_error();
    test_trim_view_result_success();
    test_trim_view_result_propagates_error();
    test_trim_with_nul_in_default_character_set();

    return 0;
}

#include <string_view>

#include <xer/assert.h>
#include <xer/error.h>
#include <xer/unicode.h>

// XER_TEST_FEATURES: icu

namespace {

void test_grapheme_length_u8string_view()
{
    constexpr std::u8string_view text = u8"A\u0301B👩‍💻";

    const auto result = xer::grapheme_length(text);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 3uz);
}

void test_grapheme_length_empty()
{
    constexpr std::u8string_view text = u8"";

    const auto result = xer::grapheme_length(text);
    xer_assert(result.has_value());
    xer_assert_eq(*result, 0uz);
}

void test_grapheme_substr_u8string_view()
{
    constexpr std::u8string_view text = u8"A\u0301B👩‍💻C";

    const auto result = xer::grapheme_substr(text, 1, 2);
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string_view(u8"B👩‍💻"));
}

void test_grapheme_substr_rest_u8string_view()
{
    constexpr std::u8string_view text = u8"A\u0301B👩‍💻C";

    const auto result = xer::grapheme_substr(text, 2);
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string_view(u8"👩‍💻C"));
}

void test_grapheme_substr_offset_at_end()
{
    constexpr std::u8string_view text = u8"A\u0301B";

    const auto result = xer::grapheme_substr(text, 2);
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string_view(u8""));
}

void test_grapheme_substr_offset_out_of_range()
{
    constexpr std::u8string_view text = u8"A\u0301B";

    const auto result = xer::grapheme_substr(text, 3);
    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::out_of_range);
}

void test_grapheme_left_u8string_view()
{
    constexpr std::u8string_view text = u8"A\u0301B👩‍💻C";

    const auto result = xer::grapheme_left(text, 2);
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string_view(u8"A\u0301B"));
}

void test_grapheme_right_u8string_view()
{
    constexpr std::u8string_view text = u8"A\u0301B👩‍💻C";

    const auto result = xer::grapheme_right(text, 2);
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u8string_view(u8"👩‍💻C"));
}

void test_grapheme_left_right_over_count()
{
    constexpr std::u8string_view text = u8"A\u0301B";

    const auto left = xer::grapheme_left(text, 20);
    const auto right = xer::grapheme_right(text, 20);
    xer_assert(left.has_value());
    xer_assert(right.has_value());
    xer_assert_eq(*left, text);
    xer_assert_eq(*right, text);
}

void test_grapheme_right_zero_validates_input()
{
    const char8_t bytes[] = {
        static_cast<char8_t>(0x41),
        static_cast<char8_t>(0xE3),
        static_cast<char8_t>(0x81),
    };
    const std::u8string_view text{bytes, sizeof(bytes) / sizeof(bytes[0])};

    const auto result = xer::grapheme_right(text, 0);
    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::encoding_error);
}

void test_grapheme_substr_invalid_input()
{
    const char8_t bytes[] = {
        static_cast<char8_t>(0x41),
        static_cast<char8_t>(0xE3),
        static_cast<char8_t>(0x81),
    };
    const std::u8string_view text{bytes, sizeof(bytes) / sizeof(bytes[0])};

    const auto result = xer::grapheme_substr(text, 0);
    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::encoding_error);
}

void test_grapheme_substr_u16string_view()
{
    constexpr std::u16string_view text = u"A\u0301B😀C";

    const auto result = xer::grapheme_substr(text, 1, 2);
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::u16string_view(u"B😀"));
}

void test_grapheme_left_wstring_view()
{
    constexpr std::wstring_view text = L"A\u0301B😀C";

    const auto result = xer::grapheme_left(text, 3);
    xer_assert(result.has_value());
    xer_assert_eq(*result, std::wstring_view(L"A\u0301B😀"));
}

} // namespace

auto main() -> int
{
    test_grapheme_length_u8string_view();
    test_grapheme_length_empty();
    test_grapheme_substr_u8string_view();
    test_grapheme_substr_rest_u8string_view();
    test_grapheme_substr_offset_at_end();
    test_grapheme_substr_offset_out_of_range();
    test_grapheme_left_u8string_view();
    test_grapheme_right_u8string_view();
    test_grapheme_left_right_over_count();
    test_grapheme_right_zero_validates_input();
    test_grapheme_substr_invalid_input();
    test_grapheme_substr_u16string_view();
    test_grapheme_left_wstring_view();
}

#include <array>
#include <string_view>

#include <xer/assert.h>
#include <xer/error.h>
#include <xer/unicode.h>

// XER_TEST_FEATURES: icu

namespace {

void test_is_emoji_code_point()
{
    xer_assert(xer::is_emoji(U'😀'));
    xer_assert(xer::is_emoji(U'©'));
    xer_assert(xer::is_emoji(static_cast<char32_t>(0x1F1EF)));
    xer_assert_not(xer::is_emoji(U'あ'));
    xer_assert_not(xer::is_emoji(U'A'));
    xer_assert_not(xer::is_emoji(U'1'));
}

void assert_is_emoji(std::u8string_view text, bool expected)
{
    const auto result = xer::is_emoji(text);
    xer_assert(result.has_value());
    xer_assert_eq(*result, expected);
}

void test_is_emoji_u8string_view()
{
    assert_is_emoji(u8"😀", true);
    assert_is_emoji(u8"👩‍💻", true);
    assert_is_emoji(u8"🇯🇵", true);
    assert_is_emoji(u8"1️⃣", true);
    assert_is_emoji(u8"©️", true);

    assert_is_emoji(u8"", false);
    assert_is_emoji(u8"A", false);
    assert_is_emoji(u8"1", false);
    assert_is_emoji(u8"😀😀", false);
    assert_is_emoji(u8"あ", false);
}

void test_is_emoji_u16string_view()
{
    const auto face = xer::is_emoji(std::u16string_view{u"😀"});
    xer_assert(face.has_value());
    xer_assert(*face);

    const auto zwj = xer::is_emoji(std::u16string_view{u"👩‍💻"});
    xer_assert(zwj.has_value());
    xer_assert(*zwj);

    const auto text = xer::is_emoji(std::u16string_view{u"あ"});
    xer_assert(text.has_value());
    xer_assert_not(*text);
}

void test_is_emoji_wstring_view()
{
    const auto face = xer::is_emoji(std::wstring_view{L"😀"});
    xer_assert(face.has_value());
    xer_assert(*face);

    const auto text = xer::is_emoji(std::wstring_view{L"A"});
    xer_assert(text.has_value());
    xer_assert_not(*text);
}

void test_is_emoji_reports_invalid_utf8()
{
    const std::array<char8_t, 2> bytes = {
        static_cast<char8_t>(0xC3),
        static_cast<char8_t>(0x28),
    };
    const std::u8string_view text{bytes.data(), bytes.size()};

    const auto result = xer::is_emoji(text);
    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::encoding_error);
}

} // namespace

auto main() -> int
{
    test_is_emoji_code_point();
    test_is_emoji_u8string_view();
    test_is_emoji_u16string_view();
    test_is_emoji_wstring_view();
    test_is_emoji_reports_invalid_utf8();
}

#include <string>
#include <string_view>

#include <xer/assert.h>
#include <xer/error.h>
#include <xer/unicode_normalize.h>

// XER_TEST_FEATURES: icu

namespace {

void test_normalize_nfc_empty()
{
    const auto result = xer::normalize_nfc(u8"");

    xer_assert(result.has_value());
    xer_assert_eq(*result, u8"");

    const auto check = xer::is_normalized_nfc(u8"");
    xer_assert(check.has_value());
    xer_assert(*check);
}

void test_normalize_nfc_ascii()
{
    const auto result = xer::normalize_nfc(u8"Hello, XER!");

    xer_assert(result.has_value());
    xer_assert_eq(*result, u8"Hello, XER!");

    const auto check = xer::is_normalized_nfc(u8"Hello, XER!");
    xer_assert(check.has_value());
    xer_assert(*check);
}

void test_normalize_nfc_japanese_voiced_mark()
{
    const auto result = xer::normalize_nfc(u8"か\u3099き\u3099く\u3099け\u3099こ\u3099");

    xer_assert(result.has_value());
    xer_assert_eq(*result, u8"がぎぐげご");

    const auto before = xer::is_normalized_nfc(u8"か\u3099");
    const auto after = xer::is_normalized_nfc(u8"が");

    xer_assert(before.has_value());
    xer_assert(after.has_value());
    xer_assert_not(*before);
    xer_assert(*after);
}

void test_normalize_nfc_latin_composition()
{
    const auto result = xer::normalize_nfc(u8"A\u030A");

    xer_assert(result.has_value());
    xer_assert_eq(*result, u8"\u00C5");

    const auto before = xer::is_normalized_nfc(u8"A\u030A");
    const auto after = xer::is_normalized_nfc(u8"\u00C5");

    xer_assert(before.has_value());
    xer_assert(after.has_value());
    xer_assert_not(*before);
    xer_assert(*after);
}

void test_normalize_nfc_invalid_utf8()
{
    const char8_t invalid_bytes[] = {
        static_cast<char8_t>(0xC3),
        static_cast<char8_t>(0x28),
    };
    const std::u8string_view invalid(invalid_bytes, 2);

    const auto normalized = xer::normalize_nfc(invalid);
    const auto checked = xer::is_normalized_nfc(invalid);

    xer_assert_not(normalized.has_value());
    xer_assert_not(checked.has_value());
    xer_assert_eq(normalized.error().code, xer::error_t::encoding_error);
    xer_assert_eq(checked.error().code, xer::error_t::encoding_error);
}

} // namespace

auto main() -> int
{
    test_normalize_nfc_empty();
    test_normalize_nfc_ascii();
    test_normalize_nfc_japanese_voiced_mark();
    test_normalize_nfc_latin_composition();
    test_normalize_nfc_invalid_utf8();

    return 0;
}

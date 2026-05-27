#include <xer/assert.h>
#include <xer/ja.h>

namespace {

void test_to_hiragana_basic()
{
    const auto result = xer::ja::to_hiragana(u8"カタカナ ヴ ヵ ヶ");

    xer_assert(result.has_value());
    xer_assert_eq(*result, u8"かたかな ゔ ゕ ゖ");
}

void test_to_hiragana_keeps_non_katakana()
{
    const auto result = xer::ja::to_hiragana(u8"ABC あいう 123 ー");

    xer_assert(result.has_value());
    xer_assert_eq(*result, u8"ABC あいう 123 ー");
}

void test_to_hiragana_iteration_marks()
{
    const auto result = xer::ja::to_hiragana(u8"ヽヾ");

    xer_assert(result.has_value());
    xer_assert_eq(*result, u8"ゝゞ");
}

void test_to_katakana_basic()
{
    const auto result = xer::ja::to_katakana(u8"ひらがな ゔ ゕ ゖ");

    xer_assert(result.has_value());
    xer_assert_eq(*result, u8"ヒラガナ ヴ ヵ ヶ");
}

void test_to_katakana_keeps_non_hiragana()
{
    const auto result = xer::ja::to_katakana(u8"ABC カタカナ 123 ー");

    xer_assert(result.has_value());
    xer_assert_eq(*result, u8"ABC カタカナ 123 ー");
}

void test_to_katakana_iteration_marks()
{
    const auto result = xer::ja::to_katakana(u8"ゝゞ");

    xer_assert(result.has_value());
    xer_assert_eq(*result, u8"ヽヾ");
}

void test_normalize_kana_halfwidth()
{
    const auto result = xer::ja::normalize_kana(u8"ﾊﾝｶｸｶﾅ ｶﾞｷﾞｸﾞｹﾞｺﾞ ﾊﾟﾋﾟﾌﾟﾍﾟﾎﾟ");

    xer_assert(result.has_value());
    xer_assert_eq(*result, u8"ハンカクカナ ガギグゲゴ パピプペポ");
}

void test_normalize_kana_punctuation_and_prolonged_sound_mark()
{
    const auto result = xer::ja::normalize_kana(u8"｡｢ｶﾅ｣､･ｰ");

    xer_assert(result.has_value());
    xer_assert_eq(*result, u8"。「カナ」、・ー");
}

void test_normalize_kana_combining_marks()
{
    const auto result = xer::ja::normalize_kana(u8"がき゛ぱハ゜ウ゛ﾜﾞ");

    xer_assert(result.has_value());
    xer_assert_eq(*result, u8"がぎぱパヴヷ");
}

void test_normalize_kana_keeps_script_choice()
{
    const auto result = xer::ja::normalize_kana(u8"ひらがな カタカナ ABC 123");

    xer_assert(result.has_value());
    xer_assert_eq(*result, u8"ひらがな カタカナ ABC 123");
}

void test_normalize_kana_invalid_utf8()
{
    const char8_t invalid[] = {
        static_cast<char8_t>(0xE3),
        static_cast<char8_t>(0x81),
    };

    const auto result = xer::ja::normalize_kana(
        std::u8string_view(invalid, sizeof(invalid)));

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::encoding_error);
}

void test_invalid_utf8()
{
    const char8_t invalid[] = {
        static_cast<char8_t>(0xE3),
        static_cast<char8_t>(0x81),
    };

    const auto result = xer::ja::to_hiragana(
        std::u8string_view(invalid, sizeof(invalid)));

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::encoding_error);
}

} // namespace

auto main() -> int
{
    test_to_hiragana_basic();
    test_to_hiragana_keeps_non_katakana();
    test_to_hiragana_iteration_marks();
    test_to_katakana_basic();
    test_to_katakana_keeps_non_hiragana();
    test_to_katakana_iteration_marks();
    test_normalize_kana_halfwidth();
    test_normalize_kana_punctuation_and_prolonged_sound_mark();
    test_normalize_kana_combining_marks();
    test_normalize_kana_keeps_script_choice();
    test_normalize_kana_invalid_utf8();
    test_invalid_utf8();

    return 0;
}

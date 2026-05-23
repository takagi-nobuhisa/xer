/**
 * @file tests/test_braille_chars.cpp
 * @brief Tests for basic one-character braille conversion helpers.
 */

#include <string>
#include <string_view>

#include <xer/assert.h>
#include <xer/braille.h>
#include <xer/error.h>

namespace {

void assert_braille_result_eq(
    const xer::result<std::u8string_view>& result,
    std::u8string_view expected)
{
    xer_assert(result.has_value());
    xer_assert_eq(*result, expected);
}

void assert_invalid_argument(const xer::result<std::u8string_view>& result)
{
    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void assert_string_invalid_argument(const xer::result<std::u8string>& result)
{
    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

void assert_braille_text_eq(
    const xer::result<std::u8string>& result,
    std::u8string_view expected)
{
    xer_assert(result.has_value());
    xer_assert_eq(*result, expected);
}

void test_alpha_to_braille_lowercase()
{
    assert_braille_result_eq(xer::braille::alpha_to_braille(U'a'), u8"⠁");
    assert_braille_result_eq(xer::braille::alpha_to_braille(U'b'), u8"⠃");
    assert_braille_result_eq(xer::braille::alpha_to_braille(U'c'), u8"⠉");
    assert_braille_result_eq(xer::braille::alpha_to_braille(U'j'), u8"⠚");
    assert_braille_result_eq(xer::braille::alpha_to_braille(U'k'), u8"⠅");
    assert_braille_result_eq(xer::braille::alpha_to_braille(U'w'), u8"⠺");
    assert_braille_result_eq(xer::braille::alpha_to_braille(U'z'), u8"⠵");
}

void test_alpha_to_braille_uppercase()
{
    assert_braille_result_eq(xer::braille::alpha_to_braille(U'A'), u8"⠁");
    assert_braille_result_eq(xer::braille::alpha_to_braille(U'J'), u8"⠚");
    assert_braille_result_eq(xer::braille::alpha_to_braille(U'W'), u8"⠺");
    assert_braille_result_eq(xer::braille::alpha_to_braille(U'Z'), u8"⠵");
}

void test_alpha_to_braille_rejects_non_alpha()
{
    assert_invalid_argument(xer::braille::alpha_to_braille(U'0'));
    assert_invalid_argument(xer::braille::alpha_to_braille(U'-'));
    assert_invalid_argument(xer::braille::alpha_to_braille(U'あ'));
}

void test_digit_to_braille()
{
    assert_braille_result_eq(xer::braille::digit_to_braille(U'1'), u8"⠁");
    assert_braille_result_eq(xer::braille::digit_to_braille(U'2'), u8"⠃");
    assert_braille_result_eq(xer::braille::digit_to_braille(U'3'), u8"⠉");
    assert_braille_result_eq(xer::braille::digit_to_braille(U'9'), u8"⠊");
    assert_braille_result_eq(xer::braille::digit_to_braille(U'0'), u8"⠚");
}

void test_digit_to_braille_rejects_non_digit()
{
    assert_invalid_argument(xer::braille::digit_to_braille(U'a'));
    assert_invalid_argument(xer::braille::digit_to_braille(U'-'));
    assert_invalid_argument(xer::braille::digit_to_braille(U'１'));
}

void test_alnum_to_braille()
{
    assert_braille_result_eq(xer::braille::alnum_to_braille(U'a'), u8"⠁");
    assert_braille_result_eq(xer::braille::alnum_to_braille(U'A'), u8"⠁");
    assert_braille_result_eq(xer::braille::alnum_to_braille(U'z'), u8"⠵");
    assert_braille_result_eq(xer::braille::alnum_to_braille(U'1'), u8"⠁");
    assert_braille_result_eq(xer::braille::alnum_to_braille(U'0'), u8"⠚");
}

void test_alnum_to_braille_rejects_non_alnum()
{
    assert_invalid_argument(xer::braille::alnum_to_braille(U'-'));
    assert_invalid_argument(xer::braille::alnum_to_braille(U' '));
    assert_invalid_argument(xer::braille::alnum_to_braille(U'あ'));
}

void test_punct_to_braille()
{
    assert_braille_result_eq(xer::braille::punct_to_braille(U','), u8"⠂");
    assert_braille_result_eq(xer::braille::punct_to_braille(U';'), u8"⠆");
    assert_braille_result_eq(xer::braille::punct_to_braille(U':'), u8"⠒");
    assert_braille_result_eq(xer::braille::punct_to_braille(U'.'), u8"⠲");
    assert_braille_result_eq(xer::braille::punct_to_braille(U'!'), u8"⠖");
    assert_braille_result_eq(xer::braille::punct_to_braille(U'('), u8"⠶");
    assert_braille_result_eq(xer::braille::punct_to_braille(U')'), u8"⠶");
    assert_braille_result_eq(xer::braille::punct_to_braille(U'?'), u8"⠦");
    assert_braille_result_eq(xer::braille::punct_to_braille(U'“'), u8"⠦");
    assert_braille_result_eq(xer::braille::punct_to_braille(U'*'), u8"⠔");
    assert_braille_result_eq(xer::braille::punct_to_braille(U'”'), u8"⠴");
    assert_braille_result_eq(xer::braille::punct_to_braille(U'\''), u8"⠄");
    assert_braille_result_eq(xer::braille::punct_to_braille(U'-'), u8"⠤");
    assert_braille_result_eq(xer::braille::punct_to_braille(U'‐'), u8"⠤");
}

void test_punct_to_braille_rejects_unsupported_input()
{
    assert_invalid_argument(xer::braille::punct_to_braille(U'a'));
    assert_invalid_argument(xer::braille::punct_to_braille(U'1'));
    assert_invalid_argument(xer::braille::punct_to_braille(U' '));
    assert_invalid_argument(xer::braille::punct_to_braille(U'"'));
    assert_invalid_argument(xer::braille::punct_to_braille(U'。'));
}


void test_japanese_punct_to_braille()
{
    assert_braille_result_eq(xer::braille::japanese_punct_to_braille(U'。'), u8"⠲");
    assert_braille_result_eq(xer::braille::japanese_punct_to_braille(U'、'), u8"⠰");
    assert_braille_result_eq(xer::braille::japanese_punct_to_braille(U'？'), u8"⠢");
    assert_braille_result_eq(xer::braille::japanese_punct_to_braille(U'！'), u8"⠖");
    assert_braille_result_eq(xer::braille::japanese_punct_to_braille(U'・'), u8"⠂");
    assert_braille_result_eq(xer::braille::japanese_punct_to_braille(U'「'), u8"⠤");
    assert_braille_result_eq(xer::braille::japanese_punct_to_braille(U'」'), u8"⠤");
    assert_braille_result_eq(xer::braille::japanese_punct_to_braille(U'『'), u8"⠰⠤");
    assert_braille_result_eq(xer::braille::japanese_punct_to_braille(U'』'), u8"⠰⠤");
    assert_braille_result_eq(xer::braille::japanese_punct_to_braille(U'（'), u8"⠶");
    assert_braille_result_eq(xer::braille::japanese_punct_to_braille(U'）'), u8"⠶");
    assert_braille_result_eq(xer::braille::japanese_punct_to_braille(U'…'), u8"⠄⠄⠄");
}

void test_japanese_punct_to_braille_rejects_unsupported_input()
{
    assert_invalid_argument(xer::braille::japanese_punct_to_braille(U'a'));
    assert_invalid_argument(xer::braille::japanese_punct_to_braille(U'1'));
    assert_invalid_argument(xer::braille::japanese_punct_to_braille(U' '));
}


void test_kana_to_braille_basic_hiragana()
{
    assert_braille_result_eq(xer::braille::kana_to_braille(U'あ'), u8"⠁");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'い'), u8"⠃");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'う'), u8"⠉");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'え'), u8"⠋");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'お'), u8"⠊");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'か'), u8"⠡");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'し'), u8"⠳");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'つ'), u8"⠝");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'の'), u8"⠎");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'も'), u8"⠾");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'や'), u8"⠌");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ゆ'), u8"⠬");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'よ'), u8"⠜");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'れ'), u8"⠛");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'わ'), u8"⠄");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'を'), u8"⠔");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ん'), u8"⠴");
}

void test_kana_to_braille_basic_katakana()
{
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ア'), u8"⠁");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'キ'), u8"⠣");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ス'), u8"⠹");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'テ'), u8"⠟");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ニ'), u8"⠇");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'フ'), u8"⠭");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'メ'), u8"⠿");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ロ'), u8"⠚");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ン'), u8"⠴");
}

void test_kana_to_braille_historic_kana_and_marks()
{
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ゐ'), u8"⠆");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ゑ'), u8"⠖");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ヰ'), u8"⠆");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ヱ'), u8"⠖");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ー'), u8"⠒");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'っ'), u8"⠂");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ッ'), u8"⠂");
}

void test_kana_to_braille_voiced()
{
    assert_braille_result_eq(xer::braille::kana_to_braille(U'が'), u8"⠐⠡");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ぎ'), u8"⠐⠣");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ざ'), u8"⠐⠱");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'じ'), u8"⠐⠳");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'だ'), u8"⠐⠕");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ぢ'), u8"⠐⠗");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'づ'), u8"⠐⠝");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ば'), u8"⠐⠥");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ぼ'), u8"⠐⠮");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ガ'), u8"⠐⠡");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ゾ'), u8"⠐⠺");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ド'), u8"⠐⠞");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ブ'), u8"⠐⠭");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ヴ'), u8"⠐⠉");
}

void test_kana_to_braille_semi_voiced()
{
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ぱ'), u8"⠠⠥");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ぴ'), u8"⠠⠧");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ぷ'), u8"⠠⠭");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ぺ'), u8"⠠⠯");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ぽ'), u8"⠠⠮");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'パ'), u8"⠠⠥");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ピ'), u8"⠠⠧");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'プ'), u8"⠠⠭");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ペ'), u8"⠠⠯");
    assert_braille_result_eq(xer::braille::kana_to_braille(U'ポ'), u8"⠠⠮");
}

void test_kana_to_braille_rejects_unsupported_input()
{
    assert_invalid_argument(xer::braille::kana_to_braille(U'a'));
    assert_invalid_argument(xer::braille::kana_to_braille(U'1'));
    assert_invalid_argument(xer::braille::kana_to_braille(U'ゃ'));
    assert_invalid_argument(xer::braille::kana_to_braille(U'ャ'));
    assert_invalid_argument(xer::braille::kana_to_braille(U'ぁ'));
}

void test_kana_text_to_braille_basic_text()
{
    assert_braille_text_eq(
        xer::braille::kana_text_to_braille(u8"かな カナ"),
        u8"⠡⠅ ⠡⠅");
}

void test_kana_text_to_braille_combines_yoon()
{
    assert_braille_text_eq(
        xer::braille::kana_text_to_braille(u8"きゃ きゅ きょ"),
        u8"⠈⠡ ⠈⠩ ⠈⠪");

    assert_braille_text_eq(
        xer::braille::kana_text_to_braille(u8"しゃ ちゅ にょ りゃ"),
        u8"⠈⠱ ⠈⠝ ⠈⠎ ⠈⠑");
}

void test_kana_text_to_braille_combines_voiced_and_semivoiced_yoon()
{
    assert_braille_text_eq(
        xer::braille::kana_text_to_braille(u8"ぎゃ じゅ びょ ぴゅ"),
        u8"⠘⠡ ⠘⠹ ⠘⠮ ⠨⠭");
}

void test_kana_text_to_braille_combines_special_sounds()
{
    assert_braille_text_eq(
        xer::braille::kana_text_to_braille(u8"ファ フィ フェ フォ"),
        u8"⠢⠥ ⠢⠧ ⠢⠯ ⠢⠮");

    assert_braille_text_eq(
        xer::braille::kana_text_to_braille(u8"ティ ディ チェ シェ ジェ"),
        u8"⠈⠗ ⠘⠗ ⠈⠟ ⠈⠻ ⠘⠻");

    assert_braille_text_eq(
        xer::braille::kana_text_to_braille(u8"ウィ ウェ ウォ クァ グァ トゥ ドゥ"),
        u8"⠢⠃ ⠢⠋ ⠢⠊ ⠢⠡ ⠲⠡ ⠢⠝ ⠲⠝");

    assert_braille_text_eq(
        xer::braille::kana_text_to_braille(u8"ヴァ ヴィ ヴ ヴェ ヴォ"),
        u8"⠲⠥ ⠲⠧ ⠐⠉ ⠲⠯ ⠲⠮");
}

void test_kana_text_to_braille_converts_japanese_punctuation()
{
    assert_braille_text_eq(
        xer::braille::kana_text_to_braille(u8"あ、い。う？え！「お」"),
        u8"⠁⠰⠃⠲⠉⠢⠋⠖⠤⠊⠤");
}

void test_kana_text_to_braille_rejects_unsupported_input()
{
    assert_string_invalid_argument(xer::braille::kana_text_to_braille(u8"ゃ"));
    assert_string_invalid_argument(xer::braille::kana_text_to_braille(u8"あゃ"));
    assert_string_invalid_argument(xer::braille::kana_text_to_braille(u8"abc"));
}

} // namespace

auto main() -> int
{
    test_alpha_to_braille_lowercase();
    test_alpha_to_braille_uppercase();
    test_alpha_to_braille_rejects_non_alpha();
    test_digit_to_braille();
    test_digit_to_braille_rejects_non_digit();
    test_alnum_to_braille();
    test_alnum_to_braille_rejects_non_alnum();
    test_punct_to_braille();
    test_punct_to_braille_rejects_unsupported_input();
    test_japanese_punct_to_braille();
    test_japanese_punct_to_braille_rejects_unsupported_input();
    test_kana_text_to_braille_basic_text();
    test_kana_text_to_braille_combines_yoon();
    test_kana_text_to_braille_combines_voiced_and_semivoiced_yoon();
    test_kana_text_to_braille_combines_special_sounds();
    test_kana_text_to_braille_converts_japanese_punctuation();
    test_kana_text_to_braille_rejects_unsupported_input();
    test_kana_to_braille_basic_hiragana();
    test_kana_to_braille_basic_katakana();
    test_kana_to_braille_historic_kana_and_marks();
    test_kana_to_braille_voiced();
    test_kana_to_braille_semi_voiced();
    test_kana_to_braille_rejects_unsupported_input();
    return 0;
}

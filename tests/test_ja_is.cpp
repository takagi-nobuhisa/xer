#include <string_view>

#include <xer/assert.h>
#include <xer/ja.h>

namespace {

void test_is_hiragana()
{
    xer_assert(xer::ja::is_hiragana(U'あ'));
    xer_assert(xer::ja::is_hiragana(U'ゖ'));
    xer_assert(xer::ja::is_hiragana(U'ゝ'));
    xer_assert(xer::ja::is_hiragana(U'ゞ'));
    xer_assert(xer::ja::is_hiragana(U'ー'));
    xer_assert_not(xer::ja::is_hiragana(U'ア'));
    xer_assert_not(xer::ja::is_hiragana(U'A'));
}

void test_is_katakana()
{
    xer_assert(xer::ja::is_katakana(U'ア'));
    xer_assert(xer::ja::is_katakana(U'ヶ'));
    xer_assert(xer::ja::is_katakana(U'ㇰ'));
    xer_assert(xer::ja::is_katakana(U'ヽ'));
    xer_assert(xer::ja::is_katakana(U'ヾ'));
    xer_assert(xer::ja::is_katakana(U'ー'));
    xer_assert(xer::ja::is_katakana(U'ｶ'));
    xer_assert(xer::ja::is_katakana(U'ｰ'));
    xer_assert_not(xer::ja::is_katakana(U'あ'));
    xer_assert_not(xer::ja::is_katakana(U'A'));
}

void test_is_kana()
{
    xer_assert(xer::ja::is_kana(U'あ'));
    xer_assert(xer::ja::is_kana(U'ア'));
    xer_assert(xer::ja::is_kana(U'ｱ'));
    xer_assert(xer::ja::is_kana(U'ー'));
    xer_assert(xer::ja::is_kana(U'ｰ'));
    xer_assert(xer::ja::is_kana(U'ゝ'));
    xer_assert(xer::ja::is_kana(U'ヾ'));
    xer_assert_not(xer::ja::is_kana(U'漢'));
    xer_assert_not(xer::ja::is_kana(U'A'));
}

void test_is_kanji()
{
    xer_assert(xer::ja::is_kanji(U'日'));
    xer_assert(xer::ja::is_kanji(U'本'));
    xer_assert(xer::ja::is_kanji(U'𠀋'));
    xer_assert_not(xer::ja::is_kanji(U'あ'));
    xer_assert_not(xer::ja::is_kanji(U'A'));
}

void test_is_japanese_punctuation()
{
    xer_assert(xer::ja::is_japanese_punctuation(U'、'));
    xer_assert(xer::ja::is_japanese_punctuation(U'。'));
    xer_assert(xer::ja::is_japanese_punctuation(U'「'));
    xer_assert(xer::ja::is_japanese_punctuation(U'！'));
    xer_assert(xer::ja::is_japanese_punctuation(U'…'));
    xer_assert_not(xer::ja::is_japanese_punctuation(U'.'));
    xer_assert_not(xer::ja::is_japanese_punctuation(U'A'));
}

void test_is_japanese()
{
    xer_assert(xer::ja::is_japanese(U'あ'));
    xer_assert(xer::ja::is_japanese(U'ア'));
    xer_assert(xer::ja::is_japanese(U'漢'));
    xer_assert(xer::ja::is_japanese(U'。'));
    xer_assert_not(xer::ja::is_japanese(U'A'));
    xer_assert_not(xer::ja::is_japanese(U'1'));
}


void test_contains_hiragana()
{
    const auto plain = xer::ja::contains_hiragana(u8"abcあいう");
    xer_assert(plain.has_value());
    xer_assert(plain.value());

    const auto prolonged = xer::ja::contains_hiragana(u8"ABCー");
    xer_assert(prolonged.has_value());
    xer_assert(prolonged.value());

    const auto none = xer::ja::contains_hiragana(u8"ABC漢字");
    xer_assert(none.has_value());
    xer_assert_not(none.value());

    const auto empty = xer::ja::contains_hiragana(u8"");
    xer_assert(empty.has_value());
    xer_assert_not(empty.value());
}

void test_contains_katakana()
{
    const auto plain = xer::ja::contains_katakana(u8"abcアイウ");
    xer_assert(plain.has_value());
    xer_assert(plain.value());

    const auto halfwidth = xer::ja::contains_katakana(u8"abcｶﾀｶﾅ");
    xer_assert(halfwidth.has_value());
    xer_assert(halfwidth.value());

    const auto none = xer::ja::contains_katakana(u8"ABC漢字");
    xer_assert(none.has_value());
    xer_assert_not(none.value());

    const auto empty = xer::ja::contains_katakana(u8"");
    xer_assert(empty.has_value());
    xer_assert_not(empty.value());
}

void test_contains_kana()
{
    const auto plain = xer::ja::contains_kana(u8"abcかな");
    xer_assert(plain.has_value());
    xer_assert(plain.value());

    const auto katakana = xer::ja::contains_kana(u8"abcカナ");
    xer_assert(katakana.has_value());
    xer_assert(katakana.value());

    const auto none = xer::ja::contains_kana(u8"ABC漢字");
    xer_assert(none.has_value());
    xer_assert_not(none.value());

    const auto empty = xer::ja::contains_kana(u8"");
    xer_assert(empty.has_value());
    xer_assert_not(empty.value());
}

void test_contains_kanji()
{
    const auto plain = xer::ja::contains_kanji(u8"abc日本語");
    xer_assert(plain.has_value());
    xer_assert(plain.value());

    const auto none = xer::ja::contains_kanji(u8"abcかなカナ。");
    xer_assert(none.has_value());
    xer_assert_not(none.value());

    const auto empty = xer::ja::contains_kanji(u8"");
    xer_assert(empty.has_value());
    xer_assert_not(empty.value());
}

void test_contains_japanese()
{
    const auto hiragana = xer::ja::contains_japanese(u8"helloあ");
    xer_assert(hiragana.has_value());
    xer_assert(hiragana.value());

    const auto katakana = xer::ja::contains_japanese(u8"helloア");
    xer_assert(katakana.has_value());
    xer_assert(katakana.value());

    const auto kanji = xer::ja::contains_japanese(u8"hello漢");
    xer_assert(kanji.has_value());
    xer_assert(kanji.value());

    const auto punctuation = xer::ja::contains_japanese(u8"hello。");
    xer_assert(punctuation.has_value());
    xer_assert(punctuation.value());

    const auto none = xer::ja::contains_japanese(u8"hello😀");
    xer_assert(none.has_value());
    xer_assert_not(none.value());

    const auto empty = xer::ja::contains_japanese(u8"");
    xer_assert(empty.has_value());
    xer_assert_not(empty.value());
}

void test_contains_japanese_invalid_utf8()
{
    const char8_t invalid_bytes[] = {
        static_cast<char8_t>(0xE3),
        static_cast<char8_t>(0x81),
        u8'\0',
    };

    const auto result = xer::ja::contains_japanese(
        std::u8string_view(invalid_bytes, 2));
    xer_assert_not(result.has_value());
}

void test_is_all_hiragana()
{
    const auto plain = xer::ja::is_all_hiragana(u8"こんにちは");
    xer_assert(plain.has_value());
    xer_assert(plain.value());

    const auto practical = xer::ja::is_all_hiragana(u8"こーゝゞ");
    xer_assert(practical.has_value());
    xer_assert(practical.value());

    const auto mixed = xer::ja::is_all_hiragana(u8"こんにちはア");
    xer_assert(mixed.has_value());
    xer_assert_not(mixed.value());

    const auto empty = xer::ja::is_all_hiragana(u8"");
    xer_assert(empty.has_value());
    xer_assert_not(empty.value());
}

void test_is_all_katakana()
{
    const auto plain = xer::ja::is_all_katakana(u8"コンニチハ");
    xer_assert(plain.has_value());
    xer_assert(plain.value());

    const auto practical = xer::ja::is_all_katakana(u8"コーヽヾ");
    xer_assert(practical.has_value());
    xer_assert(practical.value());

    const auto halfwidth = xer::ja::is_all_katakana(u8"ｺﾝﾆﾁﾊｰ");
    xer_assert(halfwidth.has_value());
    xer_assert(halfwidth.value());

    const auto mixed = xer::ja::is_all_katakana(u8"コンニチハあ");
    xer_assert(mixed.has_value());
    xer_assert_not(mixed.value());

    const auto empty = xer::ja::is_all_katakana(u8"");
    xer_assert(empty.has_value());
    xer_assert_not(empty.value());
}

void test_is_all_kana()
{
    const auto mixed_kana = xer::ja::is_all_kana(u8"こんにちはコンニチハーゝヾ");
    xer_assert(mixed_kana.has_value());
    xer_assert(mixed_kana.value());

    const auto with_punctuation = xer::ja::is_all_kana(u8"こんにちは。");
    xer_assert(with_punctuation.has_value());
    xer_assert_not(with_punctuation.value());

    const auto with_kanji = xer::ja::is_all_kana(u8"今日は");
    xer_assert(with_kanji.has_value());
    xer_assert_not(with_kanji.value());

    const auto empty = xer::ja::is_all_kana(u8"");
    xer_assert(empty.has_value());
    xer_assert_not(empty.value());
}

void test_is_all_kana_invalid_utf8()
{
    const char8_t invalid_bytes[] = {
        static_cast<char8_t>(0xE3),
        static_cast<char8_t>(0x81),
        u8'\0',
    };

    const auto result = xer::ja::is_all_kana(
        std::u8string_view(invalid_bytes, 2));
    xer_assert_not(result.has_value());
}

} // namespace

auto main() -> int
{
    test_is_hiragana();
    test_is_katakana();
    test_is_kana();
    test_is_kanji();
    test_is_japanese_punctuation();
    test_is_japanese();
    test_contains_hiragana();
    test_contains_katakana();
    test_contains_kana();
    test_contains_kanji();
    test_contains_japanese();
    test_contains_japanese_invalid_utf8();
    test_is_all_hiragana();
    test_is_all_katakana();
    test_is_all_kana();
    test_is_all_kana_invalid_utf8();

    return 0;
}

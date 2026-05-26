#include <xer/assert.h>
#include <xer/ja.h>

namespace {

void test_is_hiragana()
{
    xer_assert(xer::ja::is_hiragana(U'あ'));
    xer_assert(xer::ja::is_hiragana(U'ゖ'));
    xer_assert_not(xer::ja::is_hiragana(U'ア'));
    xer_assert_not(xer::ja::is_hiragana(U'A'));
}

void test_is_katakana()
{
    xer_assert(xer::ja::is_katakana(U'ア'));
    xer_assert(xer::ja::is_katakana(U'ヶ'));
    xer_assert(xer::ja::is_katakana(U'ㇰ'));
    xer_assert(xer::ja::is_katakana(U'ｶ'));
    xer_assert_not(xer::ja::is_katakana(U'あ'));
    xer_assert_not(xer::ja::is_katakana(U'A'));
}

void test_is_kana()
{
    xer_assert(xer::ja::is_kana(U'あ'));
    xer_assert(xer::ja::is_kana(U'ア'));
    xer_assert(xer::ja::is_kana(U'ｱ'));
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

} // namespace

auto main() -> int
{
    test_is_hiragana();
    test_is_katakana();
    test_is_kana();
    test_is_kanji();
    test_is_japanese_punctuation();
    test_is_japanese();

    return 0;
}

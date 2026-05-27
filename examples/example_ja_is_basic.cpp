#include <xer/ja.h>
#include <xer/stdio.h>

// XER_EXAMPLE_BEGIN: ja_is_basic
//
// This example classifies Japanese code points and UTF-8 strings.
//
// Expected output:
// あ: hiragana
// ア: katakana
// 漢: kanji
// 。: Japanese punctuation
// hello日本語: contains Japanese
// こんにちはー: all kana

auto main() -> int
{
    if (xer::ja::is_hiragana(U'あ')) {
        if (!xer::printf(u8"あ: hiragana\n")) {
            return 1;
        }
    }

    if (xer::ja::is_katakana(U'ア')) {
        if (!xer::printf(u8"ア: katakana\n")) {
            return 1;
        }
    }

    if (xer::ja::is_kanji(U'漢')) {
        if (!xer::printf(u8"漢: kanji\n")) {
            return 1;
        }
    }

    if (xer::ja::is_japanese_punctuation(U'。')) {
        if (!xer::printf(u8"。: Japanese punctuation\n")) {
            return 1;
        }
    }

    const auto contains_japanese = xer::ja::contains_japanese(u8"hello日本語");
    if (!contains_japanese.has_value()) {
        return 1;
    }

    if (contains_japanese.value()) {
        if (!xer::printf(u8"hello日本語: contains Japanese\n")) {
            return 1;
        }
    }

    const auto all_kana = xer::ja::is_all_kana(u8"こんにちはー");
    if (!all_kana.has_value()) {
        return 1;
    }

    if (all_kana.value()) {
        if (!xer::printf(u8"こんにちはー: all kana\n")) {
            return 1;
        }
    }

    return 0;
}

// XER_EXAMPLE_END: ja_is_basic

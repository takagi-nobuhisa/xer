#include <xer/ja.h>
#include <xer/stdio.h>

// XER_EXAMPLE_BEGIN: ja_is_basic
//
// This example classifies Japanese code points.
//
// Expected output:
// あ: hiragana
// ア: katakana
// 漢: kanji
// 。: Japanese punctuation

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

    return 0;
}

// XER_EXAMPLE_END: ja_is_basic

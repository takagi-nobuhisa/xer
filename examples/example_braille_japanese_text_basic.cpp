// XER_EXAMPLE_BEGIN: braille_japanese_text_basic
//
// This example converts kana text that contains Japanese punctuation and
// special kana sequences to Japanese braille text.
//
// Expected output:
// sentence: ⠈⠡⠑⠩⠕⠒⠰⠐⠕⠜⠲
// foreign sounds: ⠲⠮ ⠢⠥ ⠢⠧ ⠢⠯ ⠢⠮
// punctuation: ⠰⠤⠇⠨⠴⠐⠡⠒⠰⠴⠢⠖

#include <string>

#include <xer/braille.h>
#include <xer/diag.h>

auto main() -> int
{
    const auto sentence = xer::ja::kana_text_to_braille(u8"キャラクター、だよ。");
    const auto foreign_sounds = xer::ja::kana_text_to_braille(u8"ヴォ ファ フィ フェ フォ");
    const auto punctuation = xer::ja::kana_text_to_braille(u8"「にほんごてんじ」？！");

    if (!xer_print(u8"sentence", sentence)) {
        return 1;
    }
    if (!xer_print(u8"foreign sounds", foreign_sounds)) {
        return 1;
    }
    if (!xer_print(u8"punctuation", punctuation)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: braille_japanese_text_basic

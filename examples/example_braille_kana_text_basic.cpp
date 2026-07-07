// XER_EXAMPLE_BEGIN: braille_kana_text_basic
//
// This example converts kana text to Japanese braille text. Unlike
// kana_to_braille, kana_text_to_braille can combine two-kana sequences such as
// きゃ, じゅ, and ぴゅ.
//
// Expected output:
// basic text: ⠡⠅ ⠡⠅
// yoon: ⠈⠡ ⠈⠩ ⠈⠪
// voiced yoon: ⠘⠡ ⠘⠹ ⠘⠮ ⠨⠭

#include <string>

#include <xer/braille.h>
#include <xer/diagnostics.h>

auto main() -> int
{
    const auto basic_text = xer::ja::kana_text_to_braille(u8"かな カナ");
    const auto yoon = xer::ja::kana_text_to_braille(u8"きゃ きゅ きょ");
    const auto voiced_yoon = xer::ja::kana_text_to_braille(u8"ぎゃ じゅ びょ ぴゅ");

    if (!xer_print(u8"basic text", basic_text)) {
        return 1;
    }
    if (!xer_print(u8"yoon", yoon)) {
        return 1;
    }
    if (!xer_print(u8"voiced yoon", voiced_yoon)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: braille_kana_text_basic

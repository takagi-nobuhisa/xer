#include <xer/ja.h>
#include <xer/stdio.h>

// XER_EXAMPLE_BEGIN: ja_to_basic
//
// This example converts between hiragana and fullwidth katakana.
//
// Expected output:
// hiragana: かたかなとゔ
// katakana: ヒラガナトヴ

auto main() -> int
{
    const auto hiragana = xer::ja::to_hiragana(u8"カタカナとヴ");
    if (!hiragana.has_value()) {
        return 1;
    }

    const auto katakana = xer::ja::to_katakana(u8"ひらがなトゔ");
    if (!katakana.has_value()) {
        return 1;
    }

    if (!xer::printf(u8"hiragana: %@\n", *hiragana)) {
        return 1;
    }

    if (!xer::printf(u8"katakana: %@\n", *katakana)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: ja_to_basic

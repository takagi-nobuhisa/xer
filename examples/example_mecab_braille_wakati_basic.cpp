#include <xer/mecab.h>
#include <xer/stdio.h>

// XER_EXAMPLE_BEGIN: mecab_braille_wakati_basic
//
// This example invokes MeCab, converts the parsed tokens to kana wakachi-gaki
// text, and then converts the same tokens to Japanese braille wakachi-gaki
// text.
//
// MeCab must be installed and available from PATH. The input text intentionally
// avoids punctuation because this low-level braille helper currently accepts
// kana text and ASCII spaces only.

auto main() -> int
{
    const auto tokens = xer::ja::mecab_parse(u8"私は猫です");
    if (!tokens.has_value()) {
        return 1;
    }

    xer::ja::mecab_kana_options options;
    options.kind = xer::ja::mecab_kana_kind::hiragana;

    const auto kana = xer::ja::mecab_kana_wakati(*tokens, options);
    const auto braille = xer::ja::mecab_braille_wakati(*tokens, options);
    if (!braille.has_value()) {
        return 1;
    }

    if (!xer::printf(u8"kana:    %@\n", kana)) {
        return 1;
    }
    if (!xer::printf(u8"braille: %@\n", *braille)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: mecab_braille_wakati_basic

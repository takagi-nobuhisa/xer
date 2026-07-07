// XER_EXAMPLE_BEGIN: mecab_braille_translate_basic
//
// This example invokes MeCab and translates Japanese text that contains
// ASCII alphanumeric fragments to braille.
//
// MeCab must be installed and available from PATH.

#include <xer/mecab.h>
#include <xer/diagnostics.h>


auto main() -> int
{
    // The standard braille conversion supports the basic Grade 1 English
    // punctuation handled by xer::braille::punct_to_braille. Characters such
    // as '+' are information-processing braille territory, so this example
    // uses only alphanumeric text and a hyphenated ASCII fragment.
    constexpr auto text = u8"XERはABC123でUTF-8を扱います。";

    xer::ja::mecab_kana_options kana_options;
    kana_options.kind = xer::ja::mecab_kana_kind::hiragana;

    const auto braille = xer::ja::mecab_braille_translate(text, {}, kana_options);
    if (!braille.has_value()) {
        return 1;
    }

    if (!xer_print(u8"input", text)) {
        return 1;
    }
    if (!xer_print(u8"braille", *braille)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: mecab_braille_translate_basic

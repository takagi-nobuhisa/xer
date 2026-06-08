// XER_EXAMPLE_BEGIN: mecab_ip_braille_translate_basic
//
// This example invokes MeCab and translates Japanese text that contains
// ASCII alphanumeric and punctuation fragments to information-processing
// braille.
//
// MeCab must be installed and available from PATH.

#include <xer/mecab.h>
#include <xer/diag.h>


auto main() -> int
{
    constexpr auto text = u8"XERはC++23でUTF-8を扱います。";

    xer::ja::mecab_kana_options kana_options;
    kana_options.kind = xer::ja::mecab_kana_kind::hiragana;

    const auto braille = xer::ja::mecab_ip_braille_translate(text, {}, kana_options);
    if (!braille.has_value()) {
        return 1;
    }

    if (!xer_print(u8"input", text)) {
        return 1;
    }
    if (!xer_print(u8"ip braille", *braille)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: mecab_ip_braille_translate_basic

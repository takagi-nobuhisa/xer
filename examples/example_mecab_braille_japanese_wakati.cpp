// XER_EXAMPLE_BEGIN: mecab_braille_japanese_wakati
//
// This example invokes MeCab and converts the parsed tokens to Japanese
// braille wakachi-gaki text. Japanese punctuation is kept close to the
// preceding phrase instead of being separated by an extra space.
//
// MeCab must be installed and available from PATH.

#include <xer/mecab.h>
#include <xer/diag.h>


auto main() -> int
{
    const auto tokens = xer::ja::mecab_parse(u8"私はキャラクターです。猫もいます。");
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

    if (!xer_print(u8"kana", kana)) {
        return 1;
    }
    if (!xer_print(u8"braille", *braille)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: mecab_braille_japanese_wakati

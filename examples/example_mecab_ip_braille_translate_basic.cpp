// XER_EXAMPLE_BEGIN: mecab_ip_braille_translate_basic
//
// This example invokes MeCab and translates Japanese text that contains
// ASCII alphanumeric and punctuation fragments to information-processing
// braille.
//
// MeCab must be installed and available from PATH.

#include <xer/mecab.h>
#include <xer/stdio.h>

namespace {

auto print_line(std::u8string_view label, std::u8string_view value) -> bool
{
    return xer::printf(u8"%@%@\n", label, value).has_value();
}

} // namespace

auto main() -> int
{
    constexpr auto text = u8"XERはC++23でUTF-8を扱います。";

    xer::mecab_kana_options kana_options;
    kana_options.kind = xer::mecab_kana_kind::hiragana;

    const auto braille = xer::mecab_ip_braille_translate(text, {}, kana_options);
    if (!braille.has_value()) {
        return 1;
    }

    if (!print_line(u8"input:      ", text)) {
        return 1;
    }
    if (!print_line(u8"ip braille: ", *braille)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: mecab_ip_braille_translate_basic

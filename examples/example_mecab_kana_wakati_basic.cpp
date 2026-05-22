#include <xer/mecab.h>
#include <xer/stdio.h>

// XER_EXAMPLE_BEGIN: mecab_kana_wakati_basic
//
// This example invokes MeCab through XER and prints kana wakachi-gaki text.
//
// The default kana mode is mixed: ordinary readings are converted to
// hiragana, while katakana-like source tokens keep katakana. Particle readings
// are pronunciation-oriented by default, so は, へ, and を become わ, え,
// and お in hiragana mode.

auto main() -> int
{
    const auto tokens = xer::mecab_parse(u8"私はコンピューターを学校へ持って行きます。");
    if (!tokens) {
        return 1;
    }

    if (!xer::printf(u8"mixed:    %@\n", xer::mecab_kana_wakati(*tokens))) {
        return 1;
    }

    xer::mecab_kana_options hiragana_options;
    hiragana_options.kind = xer::mecab_kana_kind::hiragana;

    if (!xer::printf(u8"hiragana: %@\n", xer::mecab_kana_wakati(*tokens, hiragana_options))) {
        return 1;
    }

    xer::mecab_kana_options katakana_options;
    katakana_options.kind = xer::mecab_kana_kind::katakana;

    if (!xer::printf(u8"katakana: %@\n", xer::mecab_kana_wakati(*tokens, katakana_options))) {
        return 1;
    }

    xer::mecab_kana_options surface_particle_options;
    surface_particle_options.particle_reading = false;

    if (!xer::printf(
            u8"raw particles: %@\n",
            xer::mecab_kana_wakati(*tokens, surface_particle_options))) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: mecab_kana_wakati_basic

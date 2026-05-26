#include <xer/ja.h>
#include <xer/stdio.h>

// XER_EXAMPLE_BEGIN: ja_kanji_basic
//
// This example checks Japanese kanji classes.
//
// Expected output:
// 日: kyouiku kanji
// 鬱: jouyou kanji
// 凜: usable in Japanese given names
// 亜: JIS level 1

auto main() -> int
{
    if (xer::ja::is_kyouiku_kanji(U'日')) {
        if (!xer::printf(u8"日: kyouiku kanji\n")) {
            return 1;
        }
    }

    if (xer::ja::is_jouyou_kanji(U'鬱')) {
        if (!xer::printf(u8"鬱: jouyou kanji\n")) {
            return 1;
        }
    }

    if (xer::ja::is_name_kanji(U'凜')) {
        if (!xer::printf(u8"凜: usable in Japanese given names\n")) {
            return 1;
        }
    }

    if (xer::ja::is_jis_level_1_kanji(U'亜')) {
        if (!xer::printf(u8"亜: JIS level 1\n")) {
            return 1;
        }
    }

    return 0;
}

// XER_EXAMPLE_END: ja_kanji_basic

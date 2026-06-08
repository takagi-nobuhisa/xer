// XER_EXAMPLE_BEGIN: romaji_basic
//
// This example converts hiragana and katakana text to romanized text.
//
// `romaji` uses macrons for long vowels.
// `romaji_alt` uses the alternate spelling that follows modern kana spelling.
//
// Expected output:
// romaji: tōkyō
// romaji_alt: toukyou
// katakana: kōhī

#include <string>
#include <string_view>

#include <xer/diag.h>
#include <xer/string.h>


auto main() -> int {
    const auto romaji =
        xer::strtoctrans(std::u8string_view(u8"とうきょう"),
                         xer::ctrans_id::romaji);
    if (!romaji.has_value()) {
        return 1;
    }

    if (!xer_print(u8"romaji", *romaji)) {
        return 1;
    }

    const auto romaji_alt =
        xer::strtoctrans(std::u8string_view(u8"とうきょう"),
                         xer::ctrans_id::romaji_alt);
    if (!romaji_alt.has_value()) {
        return 1;
    }

    if (!xer_print(u8"romaji_alt", *romaji_alt)) {
        return 1;
    }

    const auto katakana =
        xer::strtoctrans(std::u8string_view(u8"コーヒー"),
                         xer::ctrans_id::romaji);
    if (!katakana.has_value()) {
        return 1;
    }

    if (!xer_print(u8"katakana", *katakana)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: romaji_basic

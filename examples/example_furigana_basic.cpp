#include <xer/furigana.h>
#include <xer/stdio.h>

// XER_EXAMPLE_BEGIN: furigana_basic
//
// This example formats the same word and reading in two furigana styles.
//
// Expected output:
// html: <ruby>学校<rt>がっこう</rt></ruby>
// paren: 学校(がっこう)
// escaped html: <ruby>A&amp;B<rt>えー&amp;びー</rt></ruby>

auto main() -> int
{
    const auto html =
        xer::ja::to_furigana(u8"学校", u8"がっこう", xer::ja::ruby_html);
    const auto paren =
        xer::ja::to_furigana(u8"学校", u8"がっこう", xer::ja::ruby_paren);
    const auto escaped_html =
        xer::ja::to_furigana(u8"A&B", u8"えー&びー", xer::ja::ruby_html);

    if (!xer::printf(u8"html: %@\n", html)) {
        return 1;
    }

    if (!xer::printf(u8"paren: %@\n", paren)) {
        return 1;
    }

    if (!xer::printf(u8"escaped html: %@\n", escaped_html)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: furigana_basic

#include <xer/assert.h>
#include <xer/furigana.h>

namespace {

void test_to_furigana_html()
{
    const auto result =
        xer::ja::to_furigana(u8"学校", u8"がっこう", xer::ja::ruby_html);

    xer_assert_eq(
        result,
        u8"<ruby>学校<rt>がっこう</rt></ruby>");
}

void test_to_furigana_parenthesized()
{
    const auto result =
        xer::ja::to_furigana(u8"学校", u8"がっこう", xer::ja::ruby_paren);

    xer_assert_eq(result, u8"学校(がっこう)");
}

void test_to_furigana_html_escapes_base_text_and_reading()
{
    const auto result =
        xer::ja::to_furigana(
            u8"A&B<\"'>",
            u8"えー&びー<\"'>",
            xer::ja::ruby_html);

    xer_assert_eq(
        result,
        u8"<ruby>A&amp;B&lt;&quot;&#39;&gt;"
        u8"<rt>えー&amp;びー&lt;&quot;&#39;&gt;</rt></ruby>");
}

void test_to_furigana_parenthesized_does_not_escape()
{
    const auto result =
        xer::ja::to_furigana(
            u8"A&B<\"'>",
            u8"えー&びー<\"'>",
            xer::ja::ruby_paren);

    xer_assert_eq(result, u8"A&B<\"'>(えー&びー<\"'>)");
}

void test_to_furigana_accepts_empty_base_text_and_reading()
{
    const auto html =
        xer::ja::to_furigana(u8"", u8"", xer::ja::ruby_html);
    const auto paren =
        xer::ja::to_furigana(u8"", u8"", xer::ja::ruby_paren);

    xer_assert_eq(html, u8"<ruby><rt></rt></ruby>");
    xer_assert_eq(paren, u8"()");
}

} // namespace

auto main() -> int
{
    test_to_furigana_html();
    test_to_furigana_parenthesized();
    test_to_furigana_html_escapes_base_text_and_reading();
    test_to_furigana_parenthesized_does_not_escape();
    test_to_furigana_accepts_empty_base_text_and_reading();

    return 0;
}

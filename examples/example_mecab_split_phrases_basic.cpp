#include <xer/mecab.h>
#include <xer/stdio.h>

// XER_EXAMPLE_BEGIN: mecab_split_phrases_basic
//
// This example invokes MeCab through XER, splits the token sequence into
// bunsetsu-like phrases and symbol ranges, and prints each range.
//
// The exact tokenization depends on the installed MeCab dictionary. The phrase
// splitter is a practical heuristic based on MeCab feature fields; it is not a
// strict grammatical bunsetsu parser.

auto main() -> int
{
    const auto tokens = xer::ja::mecab_parse(u8"私は明日、学校へ行きます。");
    if (!tokens) {
        return 1;
    }

    const auto phrases = xer::ja::mecab_split_phrases(*tokens);

    for (const auto& phrase : phrases) {
        const auto kind = phrase.kind == xer::ja::mecab_phrase_kind::bunsetsu
            ? u8"文節"
            : u8"記号";

        if (!xer::printf(u8"%@\t", kind)) {
            return 1;
        }

        for (std::size_t i = 0; i < phrase.count; ++i) {
            const auto& token = (*tokens)[phrase.index + i];
            if (!xer::printf(u8"%@", token.surface)) {
                return 1;
            }
        }

        if (!xer::printf(u8"\n")) {
            return 1;
        }
    }

    return 0;
}

// XER_EXAMPLE_END: mecab_split_phrases_basic

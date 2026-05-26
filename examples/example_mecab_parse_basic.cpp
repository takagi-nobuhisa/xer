#include <xer/mecab.h>
#include <xer/stdio.h>

// XER_EXAMPLE_BEGIN: mecab_parse_basic
//
// This example invokes MeCab through XER and prints raw morphological tokens.
//
// The exact tokenization and feature strings depend on the installed MeCab
// dictionary. XER also exposes commonly used IPADIC-style feature fields
// through token.features.

auto main() -> int
{
    const auto tokens = xer::ja::mecab_parse(u8"私は猫です。");
    if (!tokens) {
        return 1;
    }

    for (const auto& token : *tokens) {
        const auto& 読み = token.features.読み.empty()
            ? token.surface
            : token.features.読み;

        if (!xer::printf(
                u8"表層=%@\t品詞=%@\t読み=%@\tfeature=%@\n",
                token.surface,
                token.features.品詞,
                読み,
                token.feature)) {
            return 1;
        }
    }

    return 0;
}

// XER_EXAMPLE_END: mecab_parse_basic

#include <xer/mecab.h>
#include <xer/stdio.h>

// XER_EXAMPLE_BEGIN: mecab_parse_basic
//
// This example invokes MeCab through XER and prints raw morphological tokens.
//
// The exact tokenization and feature strings depend on the installed MeCab
// dictionary.

auto main() -> int
{
    const auto tokens = xer::mecab_parse(u8"私は猫です。");
    if (!tokens) {
        return 1;
    }

    for (const auto& token : *tokens) {
        if (!xer::printf(
                u8"%@\t%@\n",
                token.surface,
                token.feature)) {
            return 1;
        }
    }

    return 0;
}

// XER_EXAMPLE_END: mecab_parse_basic

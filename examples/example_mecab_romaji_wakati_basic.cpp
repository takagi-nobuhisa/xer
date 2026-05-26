#include <xer/mecab.h>
#include <xer/stdio.h>

// XER_EXAMPLE_BEGIN: mecab_romaji_wakati_basic
//
// This example invokes MeCab through XER and converts the parsed token sequence
// into romaji wakachi-gaki. The conversion first builds kana readings from
// MeCab features, then passes each bunsetsu-like phrase through strtoctrans.
// Symbols are kept as their original surface text.

namespace {

[[nodiscard]]
auto print_error(std::u8string_view label, const xer::error<>& error) -> bool
{
    return xer::printf(
        u8"%@ failed: error code %@\n",
        label,
        static_cast<int>(error.code)).has_value();
}

[[nodiscard]]
auto print_romaji(
    std::u8string_view label,
    std::span<const xer::ja::mecab_token> tokens,
    const xer::ja::mecab_romaji_options& options = {}) -> bool
{
    const auto text = xer::ja::mecab_romaji_wakati(tokens, options);
    if (!text) {
        return print_error(label, text.error()) && false;
    }

    return xer::printf(u8"%@: %@\n", label, *text).has_value();
}

} // namespace

auto main() -> int
{
    const auto tokens = xer::ja::mecab_parse(u8"私は猫です。");
    if (!tokens) {
        if (!print_error(u8"mecab_parse", tokens.error())) {
            return 1;
        }
        return 1;
    }

    if (!print_romaji(u8"romaji", *tokens)) {
        return 1;
    }

    const xer::ja::mecab_romaji_options alt_options {
        .romaji = xer::ctrans_id::romaji_alt,
    };
    if (!print_romaji(u8"romaji_alt", *tokens, alt_options)) {
        return 1;
    }

    const xer::ja::mecab_romaji_options surface_particle_options {
        .kana = xer::ja::mecab_kana_options {
            .particle_reading = false,
        },
    };
    if (!print_romaji(u8"particle surface", *tokens, surface_particle_options)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: mecab_romaji_wakati_basic

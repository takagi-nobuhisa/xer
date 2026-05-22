// XER_EXAMPLE_BEGIN: braille_chars_basic
//
// This example converts single English letters, digits, and punctuation marks
// to braille cells. Mode indicators such as the alphabetic indicator and the
// numeric indicator are added explicitly by the caller.
//
// Expected output:
// alphabetic word: ⠰⠭⠑⠗
// numeric value: ⠼⠁⠃⠉
// alnum cells: ⠭⠑⠗⠁⠃⠉
// punctuation: ⠂⠆⠒⠲⠖⠶⠦⠔⠴⠄⠤

#include <array>
#include <string>
#include <string_view>

#include <xer/braille.h>
#include <xer/stdio.h>

namespace {

auto append_braille(
    std::u8string& output,
    const xer::result<std::u8string_view>& value) -> bool
{
    if (!value.has_value()) {
        return false;
    }

    output += *value;
    return true;
}

auto append_alpha(std::u8string& output, char32_t c) -> bool
{
    return append_braille(output, xer::braille::alpha_to_braille(c));
}

auto append_digit(std::u8string& output, char32_t c) -> bool
{
    return append_braille(output, xer::braille::digit_to_braille(c));
}

auto append_alnum(std::u8string& output, char32_t c) -> bool
{
    return append_braille(output, xer::braille::alnum_to_braille(c));
}

auto append_punct(std::u8string& output, char32_t c) -> bool
{
    return append_braille(output, xer::braille::punct_to_braille(c));
}

} // namespace

auto main() -> int
{
    std::u8string alphabetic_word{xer::braille::alphabetic_indicator};
    for (const auto c : std::array{U'x', U'e', U'r'}) {
        if (!append_alpha(alphabetic_word, c)) {
            return 1;
        }
    }

    std::u8string numeric_value{xer::braille::numeric_indicator};
    for (const auto c : std::array{U'1', U'2', U'3'}) {
        if (!append_digit(numeric_value, c)) {
            return 1;
        }
    }

    std::u8string alnum_cells;
    for (const auto c : std::array{U'x', U'e', U'r', U'1', U'2', U'3'}) {
        if (!append_alnum(alnum_cells, c)) {
            return 1;
        }
    }

    std::u8string punctuation;
    for (const auto c : std::array{
             U',', U';', U':', U'.', U'!', U'(', U'?', U'*', U'”', U'\'',
             U'-'}) {
        if (!append_punct(punctuation, c)) {
            return 1;
        }
    }

    if (!xer::printf(u8"alphabetic word: %@\n", alphabetic_word).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"numeric value: %@\n", numeric_value).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"alnum cells: %@\n", alnum_cells).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"punctuation: %@\n", punctuation).has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: braille_chars_basic

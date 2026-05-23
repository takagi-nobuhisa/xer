// XER_EXAMPLE_BEGIN: braille_auto_mode_basic
//
// This example converts short ASCII fragments to braille while automatically
// emitting alphabetic, capital, numeric, and information-processing indicators.
//
// Expected output:
// standard input: xer 123 X AB!
// standard braille: ⠰⠭⠑⠗ ⠼⠁⠃⠉ ⠠⠭ ⠠⠠⠁⠃⠖
// information input: x>=10 && y!=0
// information braille: ⠰⠭⠢⠢⠒⠒⠼⠁⠚ ⠯⠯ ⠰⠽⠖⠒⠒⠼⠚

#include <string>
#include <string_view>

#include <xer/braille.h>
#include <xer/stdio.h>

namespace {

auto print_conversion(
    std::u8string_view input_label,
    std::u8string_view braille_label,
    std::u8string_view input,
    const xer::result<std::u8string>& braille) -> bool
{
    if (!braille.has_value()) {
        return false;
    }

    return xer::printf(u8"%@%@\n%@%@\n", input_label, input, braille_label, *braille).has_value();
}

} // namespace

auto main() -> int
{
    constexpr auto standard_input = std::u8string_view(u8"xer 123 X AB!");
    constexpr auto information_input = std::u8string_view(u8"x>=10 && y!=0");

    const auto standard_braille = xer::braille::alnum_punct_text_to_braille(standard_input);
    const auto information_braille = xer::braille::ip_alnum_punct_text_to_braille(information_input);

    if (!print_conversion(
            u8"standard input: ",
            u8"standard braille: ",
            standard_input,
            standard_braille)) {
        return 1;
    }

    if (!print_conversion(
            u8"information input: ",
            u8"information braille: ",
            information_input,
            information_braille)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: braille_auto_mode_basic

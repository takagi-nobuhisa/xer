// XER_EXAMPLE_BEGIN: braille_symbols_basic
//
// This example builds small braille strings from braille indicator constants.
//
// Expected output:
// numeric: ⠼⠁⠃⠉
// alphabetic: ⠰⠁⠃⠉
// capital: ⠠⠁
// double capital: ⠠⠠⠁⠃⠉
// information lowercase: ⠰⠁⠃⠉
// information uppercase: ⠠⠁
// information numeric: ⠼⠁⠃⠉

#include <string>
#include <string_view>

#include <xer/braille.h>
#include <xer/stdio.h>

namespace {

[[nodiscard]] auto print_label_and_text(std::u8string_view label,
                                        std::u8string_view text) -> bool {
    std::u8string line(label);
    line.append(text);

    return xer::puts(line).has_value();
}

[[nodiscard]] auto concat(std::u8string_view prefix,
                          std::u8string_view body) -> std::u8string {
    std::u8string result(prefix);
    result.append(body);
    return result;
}

} // namespace

auto main() -> int {
    constexpr auto digits = std::u8string_view(u8"⠁⠃⠉");
    constexpr auto letters = std::u8string_view(u8"⠁⠃⠉");

    if (!print_label_and_text(
            u8"numeric: ",
            concat(xer::braille::numeric_indicator, digits))) {
        return 1;
    }

    if (!print_label_and_text(
            u8"alphabetic: ",
            concat(xer::braille::alphabetic_indicator, letters))) {
        return 1;
    }

    if (!print_label_and_text(
            u8"capital: ",
            concat(xer::braille::capital_indicator, u8"⠁"))) {
        return 1;
    }

    if (!print_label_and_text(
            u8"double capital: ",
            concat(xer::braille::double_capital_indicator, letters))) {
        return 1;
    }

    namespace ip = xer::braille::information_processing;

    if (!print_label_and_text(u8"information lowercase: ",
                              concat(ip::lowercase_indicator, letters))) {
        return 1;
    }

    if (!print_label_and_text(u8"information uppercase: ",
                              concat(ip::uppercase_indicator, u8"⠁"))) {
        return 1;
    }

    if (!print_label_and_text(u8"information numeric: ",
                              concat(ip::numeric_indicator, digits))) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: braille_symbols_basic

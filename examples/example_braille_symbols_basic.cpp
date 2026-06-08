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
#include <xer/diag.h>

namespace {

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

    if (!xer_print(u8"numeric", concat(xer::braille::numeric_indicator, digits))) {
        return 1;
    }

    if (!xer_print(u8"alphabetic", concat(xer::braille::alphabetic_indicator, letters))) {
        return 1;
    }

    if (!xer_print(u8"capital", concat(xer::braille::capital_indicator, u8"⠁"))) {
        return 1;
    }

    if (!xer_print(u8"double capital", concat(xer::braille::double_capital_indicator, letters))) {
        return 1;
    }

    if (!xer_print(u8"information lowercase", concat(xer::braille::ip_lowercase_indicator, letters))) {
        return 1;
    }

    if (!xer_print(u8"information uppercase", concat(xer::braille::ip_uppercase_indicator, u8"⠁"))) {
        return 1;
    }

    if (!xer_print(u8"information numeric", concat(xer::braille::ip_numeric_indicator, digits))) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: braille_symbols_basic

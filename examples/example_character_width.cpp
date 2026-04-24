// XER_EXAMPLE_BEGIN: character_width
//
// This example checks and converts fullwidth and halfwidth characters.
//
// Expected output:
// A is halfwidth
// Ａ is fullwidth
// fullwidth: ＡＢＣ１２３！　ガパ
// halfwidth: ABC123! ｶﾞﾊﾟ
// toctrans halfwidth kana: ｶ
// strtoctrans halfwidth kana: ｶﾞ

#include <string>
#include <string_view>

#include <xer/ctype.h>
#include <xer/stdio.h>
#include <xer/string.h>

namespace {

[[nodiscard]] auto print_label_and_text(std::u8string_view label,
                                        std::u8string_view text) -> bool {
    std::u8string line(label);
    line.append(text);

    return xer::puts(line).has_value();
}

} // namespace

auto main() -> int {
    if (xer::isctype(U'A', xer::ctype_id::halfwidth)) {
        if (!xer::puts(u8"A is halfwidth").has_value()) {
            return 1;
        }
    }

    if (xer::isctype(U'Ａ', xer::ctype_id::fullwidth)) {
        if (!xer::puts(u8"Ａ is fullwidth").has_value()) {
            return 1;
        }
    }

    const auto fullwidth =
        xer::strtoctrans(std::u8string_view(u8"ABC123! ｶﾞﾊﾟ"),
                         xer::ctrans_id::fullwidth);
    if (!fullwidth.has_value()) {
        return 1;
    }

    if (!print_label_and_text(u8"fullwidth: ", *fullwidth)) {
        return 1;
    }

    const auto halfwidth =
        xer::strtoctrans(std::u8string_view(u8"ＡＢＣ１２３！　ガパ"),
                         xer::ctrans_id::halfwidth);
    if (!halfwidth.has_value()) {
        return 1;
    }

    if (!print_label_and_text(u8"halfwidth: ", *halfwidth)) {
        return 1;
    }

    // toctrans converts one code point to one code point, so the dakuten in
    // U'ガ' cannot be represented in the result and is dropped.
    const auto halfwidth_char =
        xer::toctrans(U'ガ', xer::ctrans_id::halfwidth_kana);
    if (!halfwidth_char.has_value()) {
        return 1;
    }

    if (*halfwidth_char == U'ｶ') {
        if (!xer::puts(u8"toctrans halfwidth kana: ｶ").has_value()) {
            return 1;
        }
    }

    // strtoctrans works on a string, so it can represent the dakuten as an
    // additional halfwidth sound mark.
    const auto halfwidth_string =
        xer::strtoctrans(std::u8string_view(u8"ガ"),
                         xer::ctrans_id::halfwidth_kana);
    if (!halfwidth_string.has_value()) {
        return 1;
    }

    if (!print_label_and_text(u8"strtoctrans halfwidth kana: ",
                              *halfwidth_string)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: character_width

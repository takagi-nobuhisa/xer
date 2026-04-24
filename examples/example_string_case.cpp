// XER_EXAMPLE_BEGIN: string_case
//
// This example converts UTF-8 strings to lowercase and uppercase.
//
// Expected output:
// lowercase: hello, xer!
// uppercase: HELLO, XER!
// latin1 uppercase: ÀÁÄÖÜ ẞ

#include <string>
#include <string_view>

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
    const auto lower = xer::strtolower(u8"HeLLo, XER!");
    if (!lower.has_value()) {
        return 1;
    }

    if (!print_label_and_text(u8"lowercase: ", *lower)) {
        return 1;
    }

    const auto upper = xer::strtoupper(std::u8string_view(u8"Hello, xer!"));
    if (!upper.has_value()) {
        return 1;
    }

    if (!print_label_and_text(u8"uppercase: ", *upper)) {
        return 1;
    }

    const auto latin1_upper =
        xer::strtoctrans(std::u8string_view(u8"àáäöü ß"),
                         xer::ctrans_id::latin1_upper);
    if (!latin1_upper.has_value()) {
        return 1;
    }

    if (!print_label_and_text(u8"latin1 uppercase: ", *latin1_upper)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: string_case

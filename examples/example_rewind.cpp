// XER_EXAMPLE_BEGIN: rewind_basic
//
// This example rewinds a text stream and reads it again from the beginning.
//
// Expected output:
// first pass: abc
// second pass: abc

#include <string>
#include <string_view>

#include <xer/stdio.h>

namespace {

[[nodiscard]] auto print_label_and_text(std::u8string_view label,
                                        std::u8string_view text) -> bool {
    std::u8string line(label);
    line.append(text);

    return xer::puts(line).has_value();
}

[[nodiscard]] auto read_all(xer::text_stream& stream) -> xer::result<std::u8string> {
    std::u8string text;

    for (;;) {
        const auto c = xer::fgetc(stream);
        if (!c.has_value()) {
            if (c.error().code == xer::error_t::not_found) {
                break;
            }
            return std::unexpected(c.error());
        }

        text.push_back(static_cast<char8_t>(*c));
    }

    return text;
}

} // namespace

auto main() -> int {
    constexpr std::u8string_view input = u8"abc";

    auto stream = xer::stropen(input, "r");
    if (!stream.has_value()) {
        return 1;
    }

    const auto first = read_all(*stream);
    if (!first.has_value()) {
        return 1;
    }

    if (!print_label_and_text(u8"first pass: ", *first)) {
        return 1;
    }

    if (!xer::rewind(*stream).has_value()) {
        return 1;
    }

    const auto second = read_all(*stream);
    if (!second.has_value()) {
        return 1;
    }

    if (!print_label_and_text(u8"second pass: ", *second)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: rewind_basic

// XER_EXAMPLE_BEGIN: string_prefix_suffix
//
// This example checks whether a UTF-8 string starts or ends with a substring.
//
// Expected output:
// prefix matched
// suffix matched

#include <string_view>

#include <xer/stdio.h>
#include <xer/string.h>

auto main() -> int {
    constexpr std::u8string_view text = u8"hello.txt";

    if (xer::starts_with(text, u8"hello")) {
        if (!xer::puts(u8"prefix matched").has_value()) {
            return 1;
        }
    }

    if (xer::ends_with(text, u8".txt")) {
        if (!xer::puts(u8"suffix matched").has_value()) {
            return 1;
        }
    }

    return 0;
}

// XER_EXAMPLE_END: string_prefix_suffix

// XER_EXAMPLE_BEGIN: implode_basic
//
// This example joins multiple UTF-8 string pieces
// with a separator and prints the resulting string.
//
// Expected output:
// apple,orange,grape

#include <array>

#include <xer/stdio.h>
#include <xer/string.h>

auto main() -> int
{
    constexpr std::array<std::u8string_view, 3> parts = {
        u8"apple",
        u8"orange",
        u8"grape",
    };

    const auto joined = xer::implode(u8",", parts);
    if (!joined.has_value()) {
        return 1;
    }

    if (!xer::puts(*joined).has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: implode_basic

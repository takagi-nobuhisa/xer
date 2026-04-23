// XER_EXAMPLE_BEGIN: explode_basic
//
// This example splits a UTF-8 string by a separator
// and prints each resulting element on its own line.
//
// Expected output:
// apple
// orange
// grape

#include <xer/stdio.h>
#include <xer/string.h>

auto main() -> int
{
    const auto parts = xer::explode(u8",", u8"apple,orange,grape");
    if (!parts.has_value()) {
        return 1;
    }

    for (const auto& part : *parts) {
        if (!xer::puts(part).has_value()) {
            return 1;
        }
    }

    return 0;
}

// XER_EXAMPLE_END: explode_basic

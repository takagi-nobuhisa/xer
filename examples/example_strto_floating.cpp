// XER_EXAMPLE_BEGIN: strto_floating_basic
//
// This example parses a floating-point string by using xer::strto.
//
// Expected output:
// value = 32.5

#include <xer/stdio.h>
#include <xer/stdlib.h>

auto main() -> int
{
    const auto value = xer::strto<double>(u8"3.25e1");
    if (!value) {
        return 1;
    }

    if (!xer::printf(u8"value = %.1f\n", *value)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: strto_floating_basic

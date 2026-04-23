// XER_EXAMPLE_BEGIN: arithmetic_abs
//
// This example calculates the absolute value of a signed integer.
//
// Expected output:
// abs = 42

#include <xer/stdio.h>
#include <xer/stdlib.h>

auto main() -> int
{
    const auto value = xer::abs(-42);
    if (!value) {
        return 1;
    }

    if (!xer::printf(u8"abs = %d\n", *value)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: arithmetic_abs

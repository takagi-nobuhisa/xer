// XER_EXAMPLE_BEGIN: arithmetic_uabs
//
// This example calculates the non-negative absolute value
// and returns it as an unsigned integer.
//
// Expected output:
// uabs = 42

#include <xer/stdio.h>
#include <xer/stdlib.h>

auto main() -> int
{
    const auto value = xer::uabs(-42);
    if (!value) {
        return 1;
    }

    if (!xer::printf(u8"uabs = %u\n", *value)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: arithmetic_uabs

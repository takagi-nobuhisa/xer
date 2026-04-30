// XER_EXAMPLE_BEGIN: near_basic
//
// This example checks floating-point values with an explicit tolerance.
//
// Expected output:
// close
// different

#include <xer/arithmetic.h>
#include <xer/stdio.h>

auto main() -> int
{
    const double value = 0.1 + 0.2;

    if (!xer::puts(xer::is_close(value, 0.3, 1e-12) ? u8"close" : u8"different").has_value()) {
        return 1;
    }

    if (!xer::puts(xer::is_close(value, 0.31, 1e-12) ? u8"close" : u8"different").has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: near_basic

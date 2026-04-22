// XER_EXAMPLE_BEGIN: minmax_clamp
//
// This example uses xer::min, xer::max, and xer::clamp
// with mixed signed and unsigned integer types.
//
// Expected output:
// min = -3
// max = 10
// clamped = 10

#include <xer/arithmetic.h>
#include <xer/stdio.h>

auto main() -> int
{
    const auto minimum = xer::min(-3, 10u);
    if (!minimum) {
        return 1;
    }

    const auto maximum = xer::max(-3, 10u);
    if (!maximum) {
        return 1;
    }

    const auto clamped = xer::clamp(20, 0, 10u);
    if (!clamped) {
        return 1;
    }

    if (!xer::printf(u8"min = %d\n", *minimum)) {
        return 1;
    }

    if (!xer::printf(u8"max = %d\n", *maximum)) {
        return 1;
    }

    if (!xer::printf(u8"clamped = %d\n", *clamped)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: minmax_clamp

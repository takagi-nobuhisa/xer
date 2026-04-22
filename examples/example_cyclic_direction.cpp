// XER_EXAMPLE_BEGIN: cyclic_direction
//
// This example converts degree values to xer::cyclic,
// then calculates the counterclockwise distance, clockwise distance,
// and shortest signed difference.
//
// Expected output:
// from = 350
// to = 10
// ccw = 20
// cw = 340
// diff = 20

#include <xer/cyclic.h>
#include <xer/stdio.h>

auto main() -> int
{
    const auto from = xer::from_degree<float>(350.0f);
    const auto to = xer::from_degree<float>(10.0f);

    const auto ccw = xer::to_degree(from.ccw(to));
    const auto cw = xer::to_degree(from.cw(to));
    const auto diff = xer::to_degree(from.diff(to));

    if (!xer::printf(u8"from = %.0f\n", static_cast<double>(xer::to_degree(from)))) {
        return 1;
    }

    if (!xer::printf(u8"to = %.0f\n", static_cast<double>(xer::to_degree(to)))) {
        return 1;
    }

    if (!xer::printf(u8"ccw = %.0f\n", static_cast<double>(ccw))) {
        return 1;
    }

    if (!xer::printf(u8"cw = %.0f\n", static_cast<double>(cw))) {
        return 1;
    }

    if (!xer::printf(u8"diff = %.0f\n", static_cast<double>(diff))) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: cyclic_direction

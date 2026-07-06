#include <xer/quantity.h>
#include <xer/stdio.h>

auto main() -> int
{
    using namespace xer::units;

    // XER_EXAMPLE_BEGIN: quantity_sq_cb
    //
    // This example shows square and cube unit helpers.
    //
    // Expected output:
    // area = 12.5 m2
    // volume = 3 m3
    // acceleration = 9.8 m/sec2

    const auto area = 12.5 * m2;
    const auto volume = 3.0 * m3;
    const auto acceleration = 9.8 * m / sec2;

    if (!xer::printf(u8"area = %g m2\n", area.value(m2)).has_value()) {
        return 1;
    }

    if (!xer::printf(u8"volume = %g m3\n", volume.value(m3)).has_value()) {
        return 1;
    }

    if (!xer::printf(
            u8"acceleration = %g m/sec2\n",
            acceleration.value(m / sq(sec)))
             .has_value()) {
        return 1;
    }

    // XER_EXAMPLE_END: quantity_sq_cb

    return 0;
}

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
    // area = 12.5 m²
    // volume = 3 m³
    // acceleration = 9.8 m/sec²

    const auto area = 12.5 * m²;
    const auto volume = 3.0 * m³;
    const auto acceleration = 9.8 * m / sec²;

    if (!xer::printf(u8"area = %g m²\n", area.value(m²)).has_value()) {
        return 1;
    }

    if (!xer::printf(u8"volume = %g m³\n", volume.value(m³)).has_value()) {
        return 1;
    }

    if (!xer::printf(
            u8"acceleration = %g m/sec²\n",
            acceleration.value(m / sq(sec)))
             .has_value()) {
        return 1;
    }

    // XER_EXAMPLE_END: quantity_sq_cb

    return 0;
}

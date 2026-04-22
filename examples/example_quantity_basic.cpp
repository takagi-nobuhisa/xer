// XER_EXAMPLE_BEGIN: quantity_basic
//
// This example creates a length quantity by using a unit object,
// then reads the value in both the base unit and the original unit.
//
// Expected output:
// meters = 1500
// kilometers = 1.5

#include <xer/quantity.h>
#include <xer/stdio.h>

auto main() -> int
{
    using namespace xer::units;

    const auto length = 1.5 * km;

    if (!xer::printf(u8"meters = %.0f\n", static_cast<double>(length.value()))) {
        return 1;
    }

    if (!xer::printf(
            u8"kilometers = %.1f\n",
            static_cast<double>(length.value(km)))) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: quantity_basic

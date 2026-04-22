// XER_EXAMPLE_BEGIN: unit_composition
//
// This example composes units to create a speed quantity.
//
// Expected output:
// meters per second = 10

#include <xer/quantity.h>
#include <xer/stdio.h>

auto main() -> int
{
    using namespace xer::units;

    const auto speed = 10.0 * m / sec;

    if (!xer::printf(
            u8"meters per second = %.0f\n",
            static_cast<double>(speed.value(m / sec)))) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: unit_composition

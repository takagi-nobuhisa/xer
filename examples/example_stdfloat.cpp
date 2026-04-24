// XER_EXAMPLE_BEGIN: stdfloat_basic
//
// This example uses portable floating-point aliases and literals.
//
// Expected output:
// value is in range

#include <xer/stdfloat.h>
#include <xer/stdio.h>

auto main() -> int {
    using namespace xer::literals::floating_literals;

    const xer::float_least32_t lower = 1.0_fl32;
    const xer::float_least32_t upper = 2.0_fl32;
    const xer::float_least32_t value = 1.5_fl32;

    if (lower <= value && value <= upper) {
        if (!xer::puts(u8"value is in range").has_value()) {
            return 1;
        }
    }

    return 0;
}

// XER_EXAMPLE_END: stdfloat_basic

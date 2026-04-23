// XER_EXAMPLE_BEGIN: clock_basic
//
// This example gets the current process CPU time ticks by using xer::clock.
//
// Expected output:
// The program prints the current CPU time ticks.

#include <xer/stdio.h>
#include <xer/time.h>

auto main() -> int
{
    const auto ticks = xer::clock();
    if (!ticks) {
        return 1;
    }

    if (!xer::printf(
            u8"clock ticks = %lld\n",
            static_cast<long long>(*ticks))) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: clock_basic

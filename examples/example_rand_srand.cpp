// XER_EXAMPLE_BEGIN: rand_srand_basic
//
// This example seeds the pseudo-random generator twice with the same seed
// and shows that the generated sequence is reproducible.
//
// Expected output:
// same sequence

#include <cstdint>

#include <xer/stdio.h>
#include <xer/stdlib.h>

auto main() -> int
{
    xer::srand(UINT64_C(12345));

    const std::uint64_t r1 = xer::rand();
    const std::uint64_t r2 = xer::rand();

    xer::srand(UINT64_C(12345));

    const std::uint64_t r3 = xer::rand();
    const std::uint64_t r4 = xer::rand();

    if (r1 != r3) {
        return 1;
    }

    if (r2 != r4) {
        return 1;
    }

    if (!xer::puts(u8"same sequence")) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: rand_srand_basic

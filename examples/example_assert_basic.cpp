// XER_EXAMPLE_BEGIN: assert_basic
//
// This example shows the basic assertion macros provided by xer/assert.h.
//
// Expected output:
// assertions passed

#include <xer/assert.h>
#include <xer/stdio.h>

auto main() -> int
{
    xer_assert(true);
    xer_assert_not(false);
    xer_assert_eq(2 + 3, 5);
    xer_assert_ne(2 + 3, 4);
    xer_assert_lt(3, 5);

    if (!xer::puts(u8"assertions passed")) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: assert_basic

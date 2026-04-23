// XER_EXAMPLE_BEGIN: strto_integer_basic
//
// This example parses an integer string by using xer::strto.
//
// Expected output:
// value = 255

#include <xer/stdio.h>
#include <xer/stdlib.h>

auto main() -> int
{
    const auto value = xer::strto<int>(u8"0xff", 0);
    if (!value) {
        return 1;
    }

    if (!xer::printf(u8"value = %d\n", *value)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: strto_integer_basic

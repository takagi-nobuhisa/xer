// XER_EXAMPLE_BEGIN: stdout_puts
//
// This example writes a UTF-8 string to standard output by using xer::puts.
//
// Expected output:
// Hello, XER!

#include <xer/stdio.h>

auto main() -> int
{
    if (!xer::puts(u8"Hello, XER!")) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: stdout_puts

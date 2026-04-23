// XER_EXAMPLE_BEGIN: printf_basic
//
// This example formats an integer and a UTF-8 string
// and writes them to standard output by using xer::printf.
//
// Expected output:
// value = 42, text = hello

#include <xer/stdio.h>

auto main() -> int
{
    if (!xer::printf(u8"value = %d, text = %s\n", 42, u8"hello")) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: printf_basic

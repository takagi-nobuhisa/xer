// XER_EXAMPLE_BEGIN: strlen_basic
//
// This example gets the length of a UTF-8 string by using xer::strlen.
//
// Expected output:
// 5

#include <xer/stdio.h>
#include <xer/string.h>

auto main() -> int
{
    const auto length = xer::strlen(u8"hello");
    if (!length) {
        return 1;
    }

    if (!xer::printf(u8"%zu\n", *length).has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: strlen_basic

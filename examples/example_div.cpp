// XER_EXAMPLE_BEGIN: arithmetic_div
//
// This example divides two integers by using xer::div
// and prints both the quotient and the remainder.
//
// Expected output:
// quotient = 2
// remainder = 1

#include <xer/arithmetic.h>
#include <xer/stdio.h>

auto main() -> int
{
    const auto result = xer::div(7, 3);
    if (!result) {
        return 1;
    }

    if (!xer::printf(u8"quotient = %d\n", result->quot)) {
        return 1;
    }

    if (!xer::printf(u8"remainder = %d\n", result->rem)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: arithmetic_div

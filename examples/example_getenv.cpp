// XER_EXAMPLE_BEGIN: getenv_basic
//
// This example reads an environment variable by using xer::getenv.
//
// Expected output:
// The program prints the value of PATH if it exists.
// Otherwise, it prints "PATH is not set".

#include <xer/stdio.h>
#include <xer/stdlib.h>

auto main() -> int
{
    const auto value = xer::getenv(u8"PATH");
    if (!value) {
        if (!xer::puts(u8"PATH is not set")) {
            return 1;
        }

        return 0;
    }

    if (!xer::fputs(u8"PATH = ", xer_stdout)) {
        return 1;
    }

    if (!xer::puts(*value)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: getenv_basic

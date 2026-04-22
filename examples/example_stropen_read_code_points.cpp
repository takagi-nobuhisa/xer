// XER_EXAMPLE_BEGIN: stropen_read_code_points
//
// This example opens a UTF-8 string as a text_stream by using xer::stropen,
// reads code points one by one, and writes them to standard output.
//
// Expected output:
// AあB

#include <xer/stdio.h>

auto main() -> int
{
    auto stream = xer::stropen(u8"AあB", "r");
    if (!stream) {
        return 1;
    }

    for (;;) {
        auto ch = xer::fgetc(*stream);
        if (!ch) {
            break;
        }

        if (!xer::putchar(*ch)) {
            return 1;
        }
    }

    if (!xer::putchar(U'\n')) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: stropen_read_code_points

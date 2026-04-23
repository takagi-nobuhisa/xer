// XER_EXAMPLE_BEGIN: ungetc_basic
//
// This example reads one character, pushes it back with xer::ungetc,
// and reads it again.
//
// Expected output:
// first  = A
// again  = A
// next   = B

#include <xer/stdio.h>

auto main() -> int
{
    auto stream = xer::stropen(u8"AB", "r");
    if (!stream) {
        return 1;
    }

    const auto first = xer::fgetc(*stream);
    if (!first) {
        return 1;
    }

    if (*first != U'A') {
        return 1;
    }

    if (!xer::puts(u8"first  = A")) {
        return 1;
    }

    if (!xer::ungetc(*first, *stream)) {
        return 1;
    }

    const auto again = xer::fgetc(*stream);
    if (!again) {
        return 1;
    }

    if (*again != U'A') {
        return 1;
    }

    if (!xer::puts(u8"again  = A")) {
        return 1;
    }

    const auto next = xer::fgetc(*stream);
    if (!next) {
        return 1;
    }

    if (*next != U'B') {
        return 1;
    }

    if (!xer::puts(u8"next   = B")) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: ungetc_basic

// XER_EXAMPLE_BEGIN: stream_state_basic
//
// This example shows basic usage of xer::feof,
// xer::ferror, and xer::clearerr.
//
// Expected output:
// first  = A
// second = EOF
// feof   = true
// ferror = false
// cleared feof   = false
// cleared ferror = false

#include <xer/stdio.h>

auto main() -> int
{
    auto stream = xer::stropen(u8"A", "r");
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

    const auto second = xer::fgetc(*stream);
    if (second) {
        return 1;
    }

    if (!xer::puts(u8"second = EOF")) {
        return 1;
    }

    if (!xer::puts(xer::feof(*stream) ? u8"feof   = true" : u8"feof   = false")) {
        return 1;
    }

    if (!xer::puts(xer::ferror(*stream) ? u8"ferror = true" : u8"ferror = false")) {
        return 1;
    }

    xer::clearerr(*stream);

    if (!xer::puts(xer::feof(*stream) ? u8"cleared feof   = true" : u8"cleared feof   = false")) {
        return 1;
    }

    if (!xer::puts(xer::ferror(*stream) ? u8"cleared ferror = true" : u8"cleared ferror = false")) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: stream_state_basic

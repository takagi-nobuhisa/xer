// XER_EXAMPLE_BEGIN: text_stream_position_basic
//
// This example shows basic usage of xer::fgetpos and xer::fsetpos
// with xer::text_stream.
//
// Expected output:
// first  = A
// second = B
// again  = B

#include <xer/stdio.h>

auto main() -> int
{
    auto stream = xer::stropen(u8"ABC", "r");
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

    const auto position = xer::fgetpos(*stream);
    if (!position) {
        return 1;
    }

    const auto second = xer::fgetc(*stream);
    if (!second) {
        return 1;
    }

    if (*second != U'B') {
        return 1;
    }

    if (!xer::puts(u8"second = B")) {
        return 1;
    }

    if (!xer::fsetpos(*stream, *position)) {
        return 1;
    }

    const auto again = xer::fgetc(*stream);
    if (!again) {
        return 1;
    }

    if (*again != U'B') {
        return 1;
    }

    if (!xer::puts(u8"again  = B")) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: text_stream_position_basic

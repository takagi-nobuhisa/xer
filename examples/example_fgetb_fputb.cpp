// XER_EXAMPLE_BEGIN: fgetb_fputb_basic
//
// This example shows basic usage of xer::fputb and xer::fgetb.
//
// Expected output:
// written = 11 22 33
// read    = 11 22 33

#include <xer/stdio.h>

auto main() -> int
{
    auto stream = xer::tmpfile();
    if (!stream) {
        return 1;
    }

    if (!xer::fputb(std::byte {0x11}, *stream)) {
        return 1;
    }

    if (!xer::fputb(std::byte {0x22}, *stream)) {
        return 1;
    }

    if (!xer::fputb(std::byte {0x33}, *stream)) {
        return 1;
    }

    if (!xer::puts(u8"written = 11 22 33")) {
        return 1;
    }

    if (!xer::fseek(*stream, 0, xer::seek_set)) {
        return 1;
    }

    const auto b1 = xer::fgetb(*stream);
    if (!b1) {
        return 1;
    }

    const auto b2 = xer::fgetb(*stream);
    if (!b2) {
        return 1;
    }

    const auto b3 = xer::fgetb(*stream);
    if (!b3) {
        return 1;
    }

    if (!xer::printf(
            u8"read    = %02X %02X %02X\n",
            static_cast<unsigned int>(std::to_integer<unsigned char>(*b1)),
            static_cast<unsigned int>(std::to_integer<unsigned char>(*b2)),
            static_cast<unsigned int>(std::to_integer<unsigned char>(*b3)))) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: fgetb_fputb_basic

// XER_EXAMPLE_BEGIN: tmpfile_basic
//
// This example writes bytes to a temporary binary file,
// seeks back to the beginning, reads them again, and
// prints the values.
//
// Expected output:
// 10 20 30 40

#include <array>
#include <cstddef>

#include <xer/stdio.h>

auto main() -> int
{
    auto stream = xer::tmpfile();
    if (!stream) {
        return 1;
    }

    constexpr std::array<std::byte, 4> written = {
        std::byte {0x10},
        std::byte {0x20},
        std::byte {0x30},
        std::byte {0x40},
    };

    const auto write_result = xer::fwrite(std::span<const std::byte>(written), *stream);
    if (!write_result) {
        return 1;
    }

    if (*write_result != written.size()) {
        return 1;
    }

    if (!xer::fseek(*stream, 0, xer::seek_set)) {
        return 1;
    }

    std::array<std::byte, 4> read = {};

    const auto read_result = xer::fread(std::span<std::byte>(read), *stream);
    if (!read_result) {
        return 1;
    }

    if (*read_result != read.size()) {
        return 1;
    }

    if (!xer::printf(
            u8"%02X %02X %02X %02X\n",
            static_cast<unsigned int>(std::to_integer<unsigned char>(read[0])),
            static_cast<unsigned int>(std::to_integer<unsigned char>(read[1])),
            static_cast<unsigned int>(std::to_integer<unsigned char>(read[2])),
            static_cast<unsigned int>(std::to_integer<unsigned char>(read[3])))) {
        return 1;
    }

    if (!xer::fclose(*stream)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: tmpfile_basic

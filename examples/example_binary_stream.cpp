// XER_EXAMPLE_BEGIN: binary_stream_basic
//
// This example shows basic usage of xer::binary_stream.
//
// Expected output:
// 41 42 43 44

#include <array>
#include <cstddef>
#include <utility>

#include <xer/stdio.h>

auto main() -> int
{
    auto temporary = xer::tmpfile();
    if (!temporary) {
        return 1;
    }

    xer::binary_stream stream = std::move(*temporary);

    constexpr std::array<std::byte, 4> written = {
        std::byte {'A'},
        std::byte {'B'},
        std::byte {'C'},
        std::byte {'D'},
    };

    const auto write_result = xer::fwrite(std::span<const std::byte>(written), stream);
    if (!write_result) {
        return 1;
    }

    if (*write_result != written.size()) {
        return 1;
    }

    if (!xer::fseek(stream, 0, xer::seek_set)) {
        return 1;
    }

    std::array<std::byte, 4> read = {};

    const auto read_result = xer::fread(std::span<std::byte>(read), stream);
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

    if (!xer::fclose(stream)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: binary_stream_basic

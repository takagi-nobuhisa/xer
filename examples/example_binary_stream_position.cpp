// XER_EXAMPLE_BEGIN: binary_stream_position_basic
//
// This example shows basic usage of xer::fseek and xer::ftell
// with xer::binary_stream.
//
// Expected output:
// pos1 = 4
// pos2 = 1
// pos3 = 3

#include <array>
#include <cstddef>

#include <xer/stdio.h>

auto main() -> int
{
    auto stream_result = xer::tmpfile();
    if (!stream_result) {
        return 1;
    }

    xer::binary_stream stream = std::move(*stream_result);

    constexpr std::array<std::byte, 4> data = {
        std::byte {'A'},
        std::byte {'B'},
        std::byte {'C'},
        std::byte {'D'},
    };

    const auto written = xer::fwrite(std::span<const std::byte>(data), stream);
    if (!written) {
        return 1;
    }

    if (*written != data.size()) {
        return 1;
    }

    const auto pos1 = xer::ftell(stream);
    if (!pos1) {
        return 1;
    }

    if (!xer::printf(u8"pos1 = %llu\n", static_cast<unsigned long long>(*pos1))) {
        return 1;
    }

    if (!xer::fseek(stream, 1, xer::seek_set)) {
        return 1;
    }

    const auto pos2 = xer::ftell(stream);
    if (!pos2) {
        return 1;
    }

    if (!xer::printf(u8"pos2 = %llu\n", static_cast<unsigned long long>(*pos2))) {
        return 1;
    }

    if (!xer::fseek(stream, -1, xer::seek_end)) {
        return 1;
    }

    const auto pos3 = xer::ftell(stream);
    if (!pos3) {
        return 1;
    }

    if (!xer::printf(u8"pos3 = %llu\n", static_cast<unsigned long long>(*pos3))) {
        return 1;
    }

    if (!xer::fclose(stream)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: binary_stream_position_basic

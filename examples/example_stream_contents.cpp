/**
 * @file examples/example_stream_contents.cpp
 * @brief Example for xer::stream_get_contents and xer::stream_put_contents.
 */

// XER_EXAMPLE_BEGIN: stream_contents_basic
//
// This example writes text to a stream, rewinds it, and reads the remaining
// text from the current stream position.
//
// Expected output:
// full: hello XER
// part: XER

#include <xer/stdio.h>

auto main() -> int
{
    std::u8string buffer;

    auto stream = xer::stropen(buffer, "w+");
    if (!stream.has_value()) {
        return 1;
    }

    const auto written = xer::stream_put_contents(
        *stream,
        std::u8string_view(u8"hello XER"));
    if (!written.has_value()) {
        return 1;
    }

    const auto rewound = xer::rewind(*stream);
    if (!rewound.has_value()) {
        return 1;
    }

    const auto full = xer::stream_get_contents(*stream);
    if (!full.has_value()) {
        return 1;
    }

    if (!xer::printf(u8"full: %@\n", *full).has_value()) {
        return 1;
    }

    const auto seeked = xer::fsetpos(*stream, 6);
    if (!seeked.has_value()) {
        return 1;
    }

    const auto part = xer::stream_get_contents(*stream);
    if (!part.has_value()) {
        return 1;
    }

    if (!xer::printf(u8"part: %@\n", *part).has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: stream_contents_basic

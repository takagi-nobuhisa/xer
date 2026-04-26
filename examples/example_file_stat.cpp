// XER_EXAMPLE_BEGIN: file_stat_basic
//
// This example obtains file status information with fstat, lstat, and filesize.
//
// Expected output:
// filesize: 11
// lstat size: 11
// fstat size: 11

#include <cstddef>
#include <span>

#include <xer/stdio.h>

auto main() -> int
{
    const xer::path filename(u8"example_file_stat.txt");

    if (xer::file_exists(filename)) {
        if (!xer::remove(filename)) {
            return 1;
        }
    }

    {
        auto stream = xer::fopen(filename, "w");
        if (!stream) {
            return 1;
        }

        const char text[] = "hello stat\n";
        const std::span<const char> text_span(text, sizeof(text) - 1);
        const std::span<const std::byte> bytes = std::as_bytes(text_span);

        const auto written = xer::fwrite(bytes, *stream);
        if (!written) {
            return 1;
        }

        if (*written != bytes.size()) {
            return 1;
        }

        if (!xer::fclose(*stream)) {
            return 1;
        }
    }

    const auto size = xer::filesize(filename);
    if (!size) {
        return 1;
    }

    if (!xer::printf(u8"filesize: %@\n", *size)) {
        return 1;
    }

    const auto path_status = xer::lstat(filename);
    if (!path_status) {
        return 1;
    }

    if (!xer::printf(u8"lstat size: %@\n", path_status->size)) {
        return 1;
    }

    auto stream = xer::fopen(filename, "r");
    if (!stream) {
        return 1;
    }

    const auto stream_status = xer::fstat(*stream);
    if (!stream_status) {
        return 1;
    }

    if (!xer::printf(u8"fstat size: %@\n", stream_status->size)) {
        return 1;
    }

    if (!xer::fclose(*stream)) {
        return 1;
    }

    if (!xer::remove(filename)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: file_stat_basic

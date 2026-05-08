// XER_EXAMPLE_BEGIN: touch_basic
//
// This example creates a file with touch and then sets explicit file times.
//
// Expected output:
// mtime: 1000000000
// atime: 1000000001

#include <cstdint>

#include <xer/stdio.h>

auto main() -> int
{
    const xer::path filename(u8"example_touch_basic.txt");

    if (xer::file_exists(filename)) {
        if (!xer::remove(filename)) {
            return 1;
        }
    }

    if (!xer::touch(filename)) {
        return 1;
    }

    if (!xer::touch(filename, 1000000000.0, 1000000001.0)) {
        return 1;
    }

    const auto mtime = xer::filemtime(filename);
    if (!mtime) {
        return 1;
    }

    const auto atime = xer::fileatime(filename);
    if (!atime) {
        return 1;
    }

    if (!xer::printf(u8"mtime: %@\n", static_cast<std::int64_t>(*mtime))) {
        return 1;
    }

    if (!xer::printf(u8"atime: %@\n", static_cast<std::int64_t>(*atime))) {
        return 1;
    }

    if (!xer::remove(filename)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: touch_basic

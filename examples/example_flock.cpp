// XER_EXAMPLE_BEGIN: flock_basic
//
// This example uses flock to protect a file-backed binary stream.
//
// Expected output:
// locked
// unlocked

#include <cstddef>
#include <span>

#include <xer/stdio.h>

auto main() -> int
{
    const xer::path filename(u8"example_flock.txt");

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

        const char text[] = "hello\n";
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

    auto stream = xer::fopen(filename, "r+");
    if (!stream) {
        return 1;
    }

    if (!xer::flock(*stream, xer::lock_ex)) {
        return 1;
    }

    if (!xer::puts(u8"locked")) {
        return 1;
    }

    /*
     * Perform file operations that should be protected by the advisory lock
     * here. Other processes must also use the same locking convention.
     */

    if (!xer::flock(*stream, xer::lock_un)) {
        return 1;
    }

    if (!xer::puts(u8"unlocked")) {
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

// XER_EXAMPLE_END: flock_basic

// XER_EXAMPLE_BEGIN: copy_basic
//
// This example creates a small file, copies it with xer::copy,
// reads the copied file, and removes both files.
//
// Expected output:
// copied text = hello

#include <xer/path.h>
#include <xer/stdio.h>

auto main() -> int
{
    const xer::path from(u8"example_copy_from.txt");
    const xer::path to(u8"example_copy_to.txt");

    {
        auto stream = xer::fopen(from, "w", xer::encoding_t::utf8);
        if (!stream) {
            return 1;
        }

        if (!xer::fputs(u8"hello", *stream)) {
            return 1;
        }

        if (!xer::fclose(*stream)) {
            return 1;
        }
    }

    if (!xer::copy(from, to)) {
        return 1;
    }

    {
        auto stream = xer::fopen(to, "r", xer::encoding_t::utf8);
        if (!stream) {
            return 1;
        }

        const auto text = xer::fgets(*stream, false);
        if (!text) {
            return 1;
        }

        if (!xer::fputs(u8"copied text = ", xer_stdout)) {
            return 1;
        }

        if (!xer::puts(*text)) {
            return 1;
        }

        if (!xer::fclose(*stream)) {
            return 1;
        }
    }

    if (!xer::remove(from)) {
        return 1;
    }

    if (!xer::remove(to)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: copy_basic

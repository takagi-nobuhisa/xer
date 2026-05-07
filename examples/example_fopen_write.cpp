// XER_EXAMPLE_BEGIN: fopen_write
//
// This example opens a UTF-8 text file for writing by using xer::fopen,
// writes one line, and closes the stream explicitly.
//
// Expected output:
// (no standard output)
//
// Expected file content:
// Hello, file!

#include <xer/path.h>
#include <xer/stdio.h>

auto main() -> int
{
    const xer::path filename(u8"example_output.txt");

    auto stream = xer::fopen(filename, "w", xer::encoding_t::utf8);
    if (!stream) {
        return 1;
    }

    if (!xer::fputs(u8"Hello, file!\n", *stream)) {
        return 1;
    }

    if (!xer::fclose(*stream)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: fopen_write

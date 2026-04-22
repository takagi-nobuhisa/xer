// XER_EXAMPLE_BEGIN: result_error_check
//
// This example shows how to handle a xer::result failure from xer::fopen
// and print a human-readable message by using xer::strerror.
//
// Expected output:
// Failed to open file:
// No such file or directory

#include <xer/path.h>
#include <xer/stdio.h>
#include <xer/string.h>

auto main() -> int
{
    const xer::path filename(u8"this_file_should_not_exist.txt");

    auto stream = xer::fopen(filename, "r", xer::encoding_t::utf8);
    if (!stream) {
        if (!xer::puts(u8"Failed to open file:")) {
            return 1;
        }

        const auto message = xer::strerror(stream.error().code);
        if (!message) {
            return 1;
        }

        if (!xer::puts(*message)) {
            return 1;
        }

        return 0;
    }

    return 1;
}

// XER_EXAMPLE_END: result_error_check

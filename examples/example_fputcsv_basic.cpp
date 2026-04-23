// XER_EXAMPLE_BEGIN: fputcsv_basic
//
// This example writes one CSV record to a UTF-8 string
// by using xer::fputcsv and prints the generated record.
//
// Expected output:
// apple,"red,green","He said ""hi"""

#include <vector>

#include <xer/stdio.h>

auto main() -> int
{
    const std::vector<std::u8string> fields = {
        u8"apple",
        u8"red,green",
        u8"He said \"hi\"",
    };

    std::u8string output;
    auto stream = xer::stropen(output, "w");
    if (!stream) {
        return 1;
    }

    if (!xer::fputcsv(fields, *stream)) {
        return 1;
    }

    if (!xer::fclose(*stream)) {
        return 1;
    }

    if (!xer::printf(u8"%s", output.c_str())) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: fputcsv_basic

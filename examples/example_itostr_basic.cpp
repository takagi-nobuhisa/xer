// XER_EXAMPLE_BEGIN: itostr_basic
//
// This example converts an integer value to a UTF-8 string by using xer::itostr.
//
// Expected output:
// hex = ff

#include <string>

#include <xer/stdio.h>
#include <xer/stdlib.h>

auto main() -> int
{
    std::u8string text;
    const auto result = xer::itostr(255, text, 16);
    if (!result) {
        return 1;
    }

    if (!xer::printf(u8"hex = %s\n", text.c_str())) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: itostr_basic

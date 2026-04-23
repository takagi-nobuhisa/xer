// XER_EXAMPLE_BEGIN: strcpy_family_basic
//
// This example compares xer::strcpy, xer::strncpy,
// xer::strcat, and xer::strncat with std::u8string.
//
// Expected output:
// strcpy  = abc
// strncpy = abc
// strcat  = abcXYZ
// strncat = abcXY

#include <string_view>

#include <xer/stdio.h>
#include <xer/string.h>

auto main() -> int
{
    std::u8string strcpy_buffer;
    std::u8string strncpy_buffer;
    std::u8string strcat_buffer = u8"abc";
    std::u8string strncat_buffer = u8"abc";

    if (!xer::strcpy(strcpy_buffer, std::u8string_view(u8"abc"))) {
        return 1;
    }

    if (!xer::strncpy(strncpy_buffer, std::u8string_view(u8"abc"), 4)) {
        return 1;
    }

    if (!xer::strcat(strcat_buffer, std::u8string_view(u8"XYZ"))) {
        return 1;
    }

    if (!xer::strncat(strncat_buffer, std::u8string_view(u8"XYZ123"), 2)) {
        return 1;
    }

    if (!xer::printf(u8"strcpy  = %s\n", strcpy_buffer.c_str())) {
        return 1;
    }

    if (!xer::printf(u8"strncpy = %s\n", strncpy_buffer.c_str())) {
        return 1;
    }

    if (!xer::printf(u8"strcat  = %s\n", strcat_buffer.c_str())) {
        return 1;
    }

    if (!xer::printf(u8"strncat = %s\n", strncat_buffer.c_str())) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: strcpy_family_basic

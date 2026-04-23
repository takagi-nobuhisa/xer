// XER_EXAMPLE_BEGIN: strpos_family_basic
//
// This example compares xer::strpos and xer::strrpos.
//
// Expected output:
// strpos  = 2
// strrpos = 4

#include <xer/stdio.h>
#include <xer/string.h>

auto main() -> int
{
    const auto strpos_result = xer::strpos(u8"abcbcb", u8"cb");
    if (!strpos_result) {
        return 1;
    }

    const auto strrpos_result = xer::strrpos(u8"abcbcb", u8"cb");
    if (!strrpos_result) {
        return 1;
    }

    if (!xer::printf(u8"strpos  = %llu\n",
                     static_cast<unsigned long long>(*strpos_result))) {
        return 1;
    }

    if (!xer::printf(u8"strrpos = %llu\n",
                     static_cast<unsigned long long>(*strrpos_result))) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: strpos_family_basic

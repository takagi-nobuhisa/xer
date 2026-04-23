// XER_EXAMPLE_BEGIN: string_span_family_basic
//
// This example compares xer::strpbrk, xer::strspn, and xer::strcspn.
//
// Expected output:
// strpbrk = 3
// strspn  = 3
// strcspn = 3

#include <xer/stdio.h>
#include <xer/string.h>

auto main() -> int
{
    constexpr char8_t text[] = u8"abc123xyz";

    const auto strpbrk_result = xer::strpbrk(text, u8"0123456789");
    if (!strpbrk_result) {
        return 1;
    }

    const auto strspn_result = xer::strspn(text, u8"abc");
    if (!strspn_result) {
        return 1;
    }

    const auto strcspn_result = xer::strcspn(text, u8"0123456789");
    if (!strcspn_result) {
        return 1;
    }

    const auto strpbrk_pos =
        static_cast<unsigned long long>(*strpbrk_result - text);

    if (!xer::printf(u8"strpbrk = %llu\n", strpbrk_pos)) {
        return 1;
    }

    if (!xer::printf(u8"strspn  = %llu\n",
                     static_cast<unsigned long long>(*strspn_result))) {
        return 1;
    }

    if (!xer::printf(u8"strcspn = %llu\n",
                     static_cast<unsigned long long>(*strcspn_result))) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: string_span_family_basic

// XER_EXAMPLE_BEGIN: string_search_family_basic
//
// This example compares:
//
// - xer::strchr
// - xer::strrchr
// - xer::strstr
// - xer::strrstr
//
// Expected output:
// strchr  = 2
// strrchr = 4
// strstr  = 2
// strrstr = 4

#include <xer/stdio.h>
#include <xer/string.h>

auto main() -> int
{
    constexpr char8_t text[] = u8"abcbcb";

    const auto strchr_result = xer::strchr(text, u8'c');
    if (!strchr_result) {
        return 1;
    }

    const auto strrchr_result = xer::strrchr(text, u8'c');
    if (!strrchr_result) {
        return 1;
    }

    const auto strstr_result = xer::strstr(text, u8"cb");
    if (!strstr_result) {
        return 1;
    }

    const auto strrstr_result = xer::strrstr(text, u8"cb");
    if (!strrstr_result) {
        return 1;
    }

    const auto strchr_pos =
        static_cast<unsigned long long>(*strchr_result - text);
    const auto strrchr_pos =
        static_cast<unsigned long long>(*strrchr_result - text);
    const auto strstr_pos =
        static_cast<unsigned long long>(*strstr_result - text);
    const auto strrstr_pos =
        static_cast<unsigned long long>(*strrstr_result - text);

    if (!xer::printf(u8"strchr  = %llu\n", strchr_pos)) {
        return 1;
    }

    if (!xer::printf(u8"strrchr = %llu\n", strrchr_pos)) {
        return 1;
    }

    if (!xer::printf(u8"strstr  = %llu\n", strstr_pos)) {
        return 1;
    }

    if (!xer::printf(u8"strrstr = %llu\n", strrstr_pos)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: string_search_family_basic

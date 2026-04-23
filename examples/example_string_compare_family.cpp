// XER_EXAMPLE_BEGIN: string_compare_family_basic
//
// This example compares several string comparison functions:
//
// - xer::strcmp
// - xer::strncmp
// - xer::strcasecmp
// - xer::strncasecmp
// - xer::stricmp
// - xer::strnicmp
//
// Expected output:
// strcmp      = -1
// strncmp     = 0
// strcasecmp  = 0
// strncasecmp = 0
// stricmp     = 0
// strnicmp    = 0

#include <xer/stdio.h>
#include <xer/string.h>

auto main() -> int
{
    const auto strcmp_result = xer::strcmp(u8"AbC", u8"abc");
    if (!strcmp_result) {
        return 1;
    }

    const auto strncmp_result = xer::strncmp(u8"AbC", u8"AxZ", 1);
    if (!strncmp_result) {
        return 1;
    }

    const auto strcasecmp_result = xer::strcasecmp(u8"AbC", u8"abc");
    if (!strcasecmp_result) {
        return 1;
    }

    const auto strncasecmp_result = xer::strncasecmp(u8"AbC", u8"aBZ", 2);
    if (!strncasecmp_result) {
        return 1;
    }

    const auto stricmp_result =
        xer::stricmp(u8"Äbc", u8"äbc", xer::ctrans_id::latin1_lower);
    if (!stricmp_result) {
        return 1;
    }

    const auto strnicmp_result =
        xer::strnicmp(u8"ÄbZ", u8"äBc", 2, xer::ctrans_id::latin1_lower);
    if (!strnicmp_result) {
        return 1;
    }

    if (!xer::printf(u8"strcmp      = %d\n", *strcmp_result)) {
        return 1;
    }

    if (!xer::printf(u8"strncmp     = %d\n", *strncmp_result)) {
        return 1;
    }

    if (!xer::printf(u8"strcasecmp  = %d\n", *strcasecmp_result)) {
        return 1;
    }

    if (!xer::printf(u8"strncasecmp = %d\n", *strncasecmp_result)) {
        return 1;
    }

    if (!xer::printf(u8"stricmp     = %d\n", *stricmp_result)) {
        return 1;
    }

    if (!xer::printf(u8"strnicmp    = %d\n", *strnicmp_result)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: string_compare_family_basic

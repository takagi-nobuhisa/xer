#include <string_view>

#include <xer/stdio.h>
#include <xer/unicode.h>

// XER_TEST_FEATURES: icu

// XER_EXAMPLE_BEGIN: unicode_normalize_basic
//
// This example normalizes decomposed UTF-8 text to Unicode NFC.
//
// Expected output:
// before: が
// after : が
// normalized before: false
// normalized after : true

auto main() -> int
{
    constexpr std::u8string_view before = u8"か\u3099";

    const auto after = xer::normalize_nfc(before);
    if (!after.has_value()) {
        return 1;
    }

    const auto before_check = xer::is_normalized_nfc(before);
    const auto after_check = xer::is_normalized_nfc(*after);
    if (!before_check.has_value() || !after_check.has_value()) {
        return 1;
    }

    if (!xer::printf(u8"before: %@\n", before).has_value()) {
        return 1;
    }
    if (!xer::printf(u8"after : %@\n", *after).has_value()) {
        return 1;
    }
    if (!xer::printf(
        u8"normalized before: %@\n",
        *before_check ? u8"true" : u8"false").has_value()) {
        return 1;
    }
    if (!xer::printf(
        u8"normalized after : %@\n",
        *after_check ? u8"true" : u8"false").has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: unicode_normalize_basic

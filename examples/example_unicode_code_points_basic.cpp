#include <string_view>

#include <xer/stdio.h>
#include <xer/unicode.h>

// XER_TEST_FEATURES: icu

// XER_EXAMPLE_BEGIN: unicode_code_points_basic
//
// This example walks through UTF-8 text by Unicode code point.
//
// Expected output:
// offset=0 size=1 value=U+0041
// offset=1 size=3 value=U+3042
// offset=4 size=4 value=U+1F600

auto main() -> int
{
    constexpr std::u8string_view text = u8"Aあ😀";

    for (const auto& item : xer::code_points(text)) {
        if (!item.has_value()) {
            return 1;
        }

        if (!xer::printf(
            u8"offset=%llu size=%llu value=U+%04X\n",
            static_cast<unsigned long long>(item->offset),
            static_cast<unsigned long long>(item->size),
            static_cast<unsigned int>(item->value)).has_value()) {
            return 1;
        }
    }

    return 0;
}

// XER_EXAMPLE_END: unicode_code_points_basic

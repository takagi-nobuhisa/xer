#include <string_view>

#include <xer/stdio.h>
#include <xer/unicode.h>

// XER_TEST_FEATURES: icu

// XER_EXAMPLE_BEGIN: unicode_grapheme_clusters_basic
//
// This example walks through UTF-8 text by extended grapheme cluster.
//
// Expected output:
// offset=0 size=3
// offset=3 size=1
// offset=4 size=11

auto main() -> int
{
    constexpr std::u8string_view text = u8"A\u0301B👩‍💻";

    for (const auto& item : xer::grapheme_clusters(text)) {
        if (!item.has_value()) {
            return 1;
        }

        if (!xer::printf(
            u8"offset=%llu size=%llu\n",
            static_cast<unsigned long long>(item->offset),
            static_cast<unsigned long long>(item->size)).has_value()) {
            return 1;
        }
    }

    return 0;
}

// XER_EXAMPLE_END: unicode_grapheme_clusters_basic

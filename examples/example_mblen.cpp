// XER_EXAMPLE_BEGIN: mblen_basic
//
// This example shows basic usage of xer::mblen.
//
// Expected output:
// hiragana_a = 3
// empty      = 0

#include <xer/stdio.h>
#include <xer/stdlib.h>

auto main() -> int
{
    constexpr char8_t hiragana_a_text[] = u8"あ";
    constexpr char8_t empty_text[] = u8"";

    const auto hiragana_a = xer::mblen(hiragana_a_text, sizeof(hiragana_a_text));
    if (!hiragana_a) {
        return 1;
    }

    const auto empty = xer::mblen(empty_text, sizeof(empty_text));
    if (!empty) {
        return 1;
    }

    if (!xer::printf(
            u8"hiragana_a = %llu\n",
            static_cast<unsigned long long>(*hiragana_a))) {
        return 1;
    }

    if (!xer::printf(
            u8"empty      = %llu\n",
            static_cast<unsigned long long>(*empty))) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: mblen_basic

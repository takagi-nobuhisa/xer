#include <cstdint>

#include <xer/kansuji.h>
#include <xer/stdio.h>

// XER_EXAMPLE_BEGIN: kansuji_basic
//
// This example converts an integer to several Japanese numeric notations
// and parses a Kansuji text back into an integer.
//
// Expected output:
// k10  = 1234億5678万9012
// k十  = 千二百三十四億五千六百七十八万九千十二
// k一〇 = 一二三四億五六七八万九〇一二
// k拾  = 壱千弐百参拾四億五千六百七拾八万九千壱拾弐
// parsed = 1200340005

auto main() -> int
{
    constexpr std::uint64_t value = UINT64_C(123456789012);

    const auto text_k10 = xer::ja::to_kansuji(value, xer::ja::k10);
    const auto text_k十 = xer::ja::to_kansuji(value, xer::ja::k十);
    const auto text_k一〇 = xer::ja::to_kansuji(value, xer::ja::k一〇);
    const auto text_k拾 = xer::ja::to_kansuji(value, xer::ja::k拾);

    if (!xer::printf(u8"k10  = %@\n", text_k10)) {
        return 1;
    }

    if (!xer::printf(u8"k十  = %@\n", text_k十)) {
        return 1;
    }

    if (!xer::printf(u8"k一〇 = %@\n", text_k一〇)) {
        return 1;
    }

    if (!xer::printf(u8"k拾  = %@\n", text_k拾)) {
        return 1;
    }

    const auto parsed = xer::ja::from_kansuji(u8"十二億三十四万五");
    if (!parsed) {
        return 1;
    }

    if (!xer::printf(
            u8"parsed = %llu\n",
            static_cast<unsigned long long>(*parsed))) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: kansuji_basic

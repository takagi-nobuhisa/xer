// XER_EXAMPLE_BEGIN: memchr_basic
//
// This example compares xer::memchr and xer::memrchr.
//
// Expected output:
// memchr  = 1
// memrchr = 5

#include <array>
#include <cstddef>

#include <xer/stdio.h>
#include <xer/string.h>

auto main() -> int
{
    const std::array<std::byte, 6> data {
        std::byte {0x10},
        std::byte {0x20},
        std::byte {0x30},
        std::byte {0x40},
        std::byte {0x50},
        std::byte {0x20}
    };

    const auto first = xer::memchr(data, std::byte {0x20});
    if (!first) {
        return 1;
    }

    const auto last = xer::memrchr(data, std::byte {0x20});
    if (!last) {
        return 1;
    }

    const auto first_index = static_cast<unsigned long long>(*first - data.begin());
    const auto last_index = static_cast<unsigned long long>(*last - data.begin());

    if (!xer::printf(u8"memchr  = %llu\n", first_index)) {
        return 1;
    }

    if (!xer::printf(u8"memrchr = %llu\n", last_index)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: memchr_basic

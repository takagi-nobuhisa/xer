// XER_EXAMPLE_BEGIN: memcmp_basic
//
// This example shows basic usage of xer::memcmp.
//
// Expected output:
// equal   = 0
// less    = negative
// greater = positive

#include <array>
#include <cstddef>

#include <xer/stdio.h>
#include <xer/string.h>

auto main() -> int
{
    const std::array<std::byte, 4> a1 {
        std::byte {1}, std::byte {2}, std::byte {3}, std::byte {4}
    };
    const std::array<std::byte, 4> a2 {
        std::byte {1}, std::byte {2}, std::byte {3}, std::byte {4}
    };
    const std::array<std::byte, 4> b1 {
        std::byte {1}, std::byte {2}, std::byte {3}, std::byte {4}
    };
    const std::array<std::byte, 4> b2 {
        std::byte {1}, std::byte {2}, std::byte {9}, std::byte {4}
    };
    const std::array<std::byte, 4> c1 {
        std::byte {9}, std::byte {2}, std::byte {3}, std::byte {4}
    };
    const std::array<std::byte, 4> c2 {
        std::byte {1}, std::byte {2}, std::byte {3}, std::byte {4}
    };

    const auto equal_result = xer::memcmp(a1, a2);
    if (!equal_result) {
        return 1;
    }

    const auto less_result = xer::memcmp(b1, b2);
    if (!less_result) {
        return 1;
    }

    const auto greater_result = xer::memcmp(c1, c2);
    if (!greater_result) {
        return 1;
    }

    if (!xer::printf(u8"equal   = %d\n", *equal_result)) {
        return 1;
    }

    if (!xer::puts(*less_result < 0 ? u8"less    = negative" : u8"less    = unexpected")) {
        return 1;
    }

    if (!xer::puts(*greater_result > 0 ? u8"greater = positive" : u8"greater = unexpected")) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: memcmp_basic

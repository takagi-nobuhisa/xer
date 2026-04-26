/**
 * @file examples/example_str_replace.cpp
 * @brief Example for xer::str_replace.
 */

// XER_EXAMPLE_BEGIN: str_replace_basic
//
// This example replaces all non-overlapping occurrences of a UTF-8 substring.
//
// Expected output:
// I like XER.
// replaced: 2

#include <cstddef>

#include <xer/stdio.h>
#include <xer/string.h>

auto main() -> int
{
    std::size_t count = 0;

    const auto result = xer::str_replace(
        u8"C++",
        u8"XER",
        u8"I like C++. C++ is fun.",
        &count);

    if (!result.has_value()) {
        return 1;
    }

    if (!xer::puts(*result).has_value()) {
        return 1;
    }

    if (!xer::printf(u8"replaced: %@\n", count).has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: str_replace_basic

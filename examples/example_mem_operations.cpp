// XER_EXAMPLE_BEGIN: mem_operations_basic
//
// This example shows basic usage of xer::memcpy,
// xer::memmove, and xer::memset.
//
// Expected output:
// memcpy  = ABCD....
// memmove = 12123456
// memset  = zzzzzz

#include <array>
#include <cstddef>

#include <xer/stdio.h>
#include <xer/string.h>

namespace {

auto print_bytes(std::u8string_view label, const std::array<std::byte, 8>& data) -> int
{
    if (!xer::fputs(label, xer_stdout)) {
        return 1;
    }

    for (const auto value : data) {
        const unsigned int code = std::to_integer<unsigned int>(value);

        if (code >= static_cast<unsigned int>(' ') &&
            code <= static_cast<unsigned int>('~')) {
            if (!xer::putchar(static_cast<char32_t>(code))) {
                return 1;
            }
        } else {
            if (!xer::putchar(U'.')) {
                return 1;
            }
        }
    }

    if (!xer::puts(u8"")) {
        return 1;
    }

    return 0;
}

auto print_bytes6(std::u8string_view label, const std::array<std::byte, 6>& data) -> int
{
    if (!xer::fputs(label, xer_stdout)) {
        return 1;
    }

    for (const auto value : data) {
        const unsigned int code = std::to_integer<unsigned int>(value);

        if (!xer::putchar(static_cast<char32_t>(code))) {
            return 1;
        }
    }

    if (!xer::puts(u8"")) {
        return 1;
    }

    return 0;
}

} // namespace

auto main() -> int
{
    std::array<std::byte, 8> memcpy_buffer {
        std::byte {'.'}, std::byte {'.'}, std::byte {'.'}, std::byte {'.'},
        std::byte {'.'}, std::byte {'.'}, std::byte {'.'}, std::byte {'.'}
    };
    const std::array<std::byte, 4> memcpy_source {
        std::byte {'A'}, std::byte {'B'}, std::byte {'C'}, std::byte {'D'}
    };

    if (!xer::memcpy(memcpy_buffer, memcpy_source)) {
        return 1;
    }

    if (print_bytes(u8"memcpy  = ", memcpy_buffer) != 0) {
        return 1;
    }

    std::array<std::byte, 8> memmove_buffer {
        std::byte {'1'}, std::byte {'2'}, std::byte {'3'}, std::byte {'4'},
        std::byte {'5'}, std::byte {'6'}, std::byte {'7'}, std::byte {'8'}
    };

    if (!xer::memmove(memmove_buffer.data() + 2, 6, memmove_buffer.data(), 6)) {
        return 1;
    }

    if (print_bytes(u8"memmove = ", memmove_buffer) != 0) {
        return 1;
    }

    std::array<std::byte, 6> memset_buffer {
        std::byte {'a'}, std::byte {'b'}, std::byte {'c'},
        std::byte {'d'}, std::byte {'e'}, std::byte {'f'}
    };

    if (!xer::memset(memset_buffer, std::byte {'z'})) {
        return 1;
    }

    if (print_bytes6(u8"memset  = ", memset_buffer) != 0) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: mem_operations_basic

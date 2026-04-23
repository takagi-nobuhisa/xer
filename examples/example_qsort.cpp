// XER_EXAMPLE_BEGIN: qsort_basic
//
// This example shows two ways to use xer::qsort:
// a function pointer comparator and a capturing lambda comparator.
//
// Expected output:
// function pointer = 1 3 5 7 9
// capturing lambda = 9 7 5 3 1

#include <array>
#include <cstddef>

#include <xer/stdio.h>
#include <xer/stdlib.h>

namespace {

auto compare_int_ascending(const int* lhs, const int* rhs) -> int
{
    if (*lhs < *rhs) {
        return -1;
    }

    if (*lhs > *rhs) {
        return 1;
    }

    return 0;
}

auto print_values(std::u8string_view label, const std::array<int, 5>& values) -> int
{
    if (!xer::fputs(label, xer_stdout)) {
        return 1;
    }

    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index != 0) {
            if (!xer::fputs(u8" ", xer_stdout)) {
                return 1;
            }
        }

        if (!xer::printf(u8"%d", values[index])) {
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
    std::array values1 {7, 3, 9, 1, 5};

    xer::qsort(values1, compare_int_ascending);

    if (print_values(u8"function pointer = ", values1) != 0) {
        return 1;
    }

    std::array values2 {7, 3, 9, 1, 5};
    const int bias = 100;

    xer::qsort(
        values2,
        [bias](const int* lhs, const int* rhs) -> int {
            const int left = *lhs + bias;
            const int right = *rhs + bias;

            if (left > right) {
                return -1;
            }

            if (left < right) {
                return 1;
            }

            return 0;
        });

    if (print_values(u8"capturing lambda = ", values2) != 0) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: qsort_basic

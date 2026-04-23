// XER_EXAMPLE_BEGIN: bsearch_basic
//
// This example shows two ways to use xer::bsearch:
// a function pointer comparator and a capturing lambda comparator.
//
// Expected output:
// function pointer = 7
// capturing lambda = 7

#include <array>

#include <xer/stdio.h>
#include <xer/stdlib.h>

namespace {

auto compare_int(const int* lhs, const int* rhs) -> int
{
    if (*lhs < *rhs) {
        return -1;
    }

    if (*lhs > *rhs) {
        return 1;
    }

    return 0;
}

} // namespace

auto main() -> int
{
    constexpr std::array values {1, 3, 5, 7, 9};
    constexpr int key = 7;

    const auto result1 = xer::bsearch(&key, values, compare_int);
    if (!result1) {
        return 1;
    }

    if (!xer::printf(u8"function pointer = %d\n", **result1)) {
        return 1;
    }

    const int bias = 10;

    const auto result2 = xer::bsearch(
        &key,
        values,
        [bias](const int* lhs, const int* rhs) -> int {
            const int left = *lhs + bias;
            const int right = *rhs + bias;

            if (left < right) {
                return -1;
            }

            if (left > right) {
                return 1;
            }

            return 0;
        });

    if (!result2) {
        return 1;
    }

    if (!xer::printf(u8"capturing lambda = %d\n", **result2)) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: bsearch_basic

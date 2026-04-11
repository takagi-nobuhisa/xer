/**
 * @file tests/test_bsearch.cpp
 * @brief Runtime tests for xer/bits/bsearch.h.
 */

#include <array>
#include <cstddef>
#include <deque>
#include <span>
#include <vector>

#include <xer/assert.h>
#include <xer/bits/bsearch.h>
#include <xer/error.h>

namespace {

/**
 * @brief Comparator for int values.
 *
 * @param lhs Left operand.
 * @param rhs Right operand.
 * @return Negative if *lhs < *rhs, positive if *lhs > *rhs, otherwise zero.
 */
[[nodiscard]] constexpr int compare_int(const int* lhs, const int* rhs) noexcept
{
    if (*lhs < *rhs) {
        return -1;
    }

    if (*lhs > *rhs) {
        return 1;
    }

    return 0;
}

/**
 * @brief Comparator object for int values.
 */
struct int_compare {
    /**
     * @brief Compares two integers.
     *
     * @param lhs Left operand.
     * @param rhs Right operand.
     * @return Negative if *lhs < *rhs, positive if *lhs > *rhs, otherwise zero.
     */
    [[nodiscard]] constexpr int operator()(const int* lhs, const int* rhs) const noexcept
    {
        return compare_int(lhs, rhs);
    }
};

/**
 * @brief Sample record used for custom-type search tests.
 */
struct record {
    int key;
    int value;
};

/**
 * @brief Comparator for record values by key.
 *
 * @param lhs Left operand.
 * @param rhs Right operand.
 * @return Negative if lhs->key < rhs->key, positive if lhs->key > rhs->key, otherwise zero.
 */
[[nodiscard]] constexpr int compare_record_by_key(
    const record* lhs,
    const record* rhs) noexcept
{
    if (lhs->key < rhs->key) {
        return -1;
    }

    if (lhs->key > rhs->key) {
        return 1;
    }

    return 0;
}

/**
 * @brief Tests bsearch with a mutable pointer range.
 */
void test_bsearch_pointer_range()
{
    int values[] = {-3, -1, 0, 2, 4, 7, 9, 11};
    int key = 4;

    auto result = xer::bsearch(&key, values, std::size(values), compare_int);

    xer_assert(result.has_value());
    xer_assert_eq(*result.value(), 4);
    xer_assert_eq(result.value() - values, 4);
}

/**
 * @brief Tests bsearch with a const pointer range.
 */
void test_bsearch_const_pointer_range()
{
    const int values[] = {-3, -1, 0, 2, 4, 7, 9, 11};
    int key = 7;

    auto result = xer::bsearch(&key, values, std::size(values), compare_int);

    xer_assert(result.has_value());
    xer_assert_eq(*result.value(), 7);
    xer_assert_eq(result.value() - values, 5);
}

/**
 * @brief Tests bsearch with a built-in array overload.
 */
void test_bsearch_builtin_array()
{
    int values[] = {1, 3, 5, 7, 9, 11, 13};
    int key = 9;

    auto result = xer::bsearch(&key, values, int_compare{});

    xer_assert(result.has_value());
    xer_assert_eq(*result.value(), 9);
    xer_assert_eq(result.value() - values, 4);
}

/**
 * @brief Tests bsearch with std::array.
 */
void test_bsearch_std_array()
{
    std::array<int, 8> values = {-5, -2, 0, 1, 3, 6, 8, 10};
    int key = 6;

    auto result = xer::bsearch(&key, values, compare_int);

    xer_assert(result.has_value());
    xer_assert_eq(*result.value(), 6);
    xer_assert_eq(result.value() - values.begin(), 5);
}

/**
 * @brief Tests bsearch with const std::array.
 */
void test_bsearch_const_std_array()
{
    const std::array<int, 8> values = {-5, -2, 0, 1, 3, 6, 8, 10};
    int key = 1;

    auto result = xer::bsearch(&key, values, compare_int);

    xer_assert(result.has_value());
    xer_assert_eq(*result.value(), 1);
    xer_assert_eq(result.value() - values.begin(), 3);
}

/**
 * @brief Tests bsearch with std::vector.
 */
void test_bsearch_std_vector()
{
    std::vector<int> values = {-10, -4, -1, 0, 2, 5, 7, 12};
    int key = -1;

    auto result = xer::bsearch(&key, values, compare_int);

    xer_assert(result.has_value());
    xer_assert_eq(*result.value(), -1);
    xer_assert_eq(result.value() - values.begin(), 2);
}

/**
 * @brief Tests bsearch with const std::vector.
 */
void test_bsearch_const_std_vector()
{
    const std::vector<int> values = {-10, -4, -1, 0, 2, 5, 7, 12};
    int key = 12;

    auto result = xer::bsearch(&key, values, compare_int);

    xer_assert(result.has_value());
    xer_assert_eq(*result.value(), 12);
    xer_assert_eq(result.value() - values.begin(), 7);
}

/**
 * @brief Tests bsearch with std::deque.
 */
void test_bsearch_std_deque()
{
    std::deque<int> values = {2, 4, 6, 8, 10, 12, 14, 16};
    int key = 10;

    auto result = xer::bsearch(&key, values, compare_int);

    xer_assert(result.has_value());
    xer_assert_eq(*result.value(), 10);
    xer_assert_eq(result.value() - values.begin(), 4);
}

/**
 * @brief Tests bsearch with const std::deque.
 */
void test_bsearch_const_std_deque()
{
    const std::deque<int> values = {2, 4, 6, 8, 10, 12, 14, 16};
    int key = 16;

    auto result = xer::bsearch(&key, values, compare_int);

    xer_assert(result.has_value());
    xer_assert_eq(*result.value(), 16);
    xer_assert_eq(result.value() - values.begin(), 7);
}

/**
 * @brief Tests bsearch with std::span.
 */
void test_bsearch_span()
{
    std::array<int, 9> storage = {-7, -3, -1, 0, 1, 3, 5, 7, 9};
    std::span<int> values(storage);

    int key = 5;
    auto result = xer::bsearch(&key, values, compare_int);

    xer_assert(result.has_value());
    xer_assert_eq(*result.value(), 5);
    xer_assert_eq(result.value() - values.begin(), 6);
}

/**
 * @brief Tests bsearch with const std::span.
 */
void test_bsearch_const_span()
{
    const std::array<int, 9> storage = {-7, -3, -1, 0, 1, 3, 5, 7, 9};
    std::span<const int> values(storage);

    int key = -3;
    auto result = xer::bsearch(&key, values, compare_int);

    xer_assert(result.has_value());
    xer_assert_eq(*result.value(), -3);
    xer_assert_eq(result.value() - values.begin(), 1);
}

/**
 * @brief Tests bsearch with a lambda comparator.
 */
void test_bsearch_lambda_comparator()
{
    int values[] = {-8, -4, -2, 0, 3, 6, 9};
    int key = 3;

    auto result = xer::bsearch(
        &key,
        values,
        std::size(values),
        [](const int* lhs, const int* rhs) constexpr noexcept -> int {
            if (*lhs < *rhs) {
                return -1;
            }

            if (*lhs > *rhs) {
                return 1;
            }

            return 0;
        });

    xer_assert(result.has_value());
    xer_assert_eq(*result.value(), 3);
    xer_assert_eq(result.value() - values, 4);
}

/**
 * @brief Tests bsearch with a custom record type.
 */
void test_bsearch_custom_type()
{
    std::vector<record> values = {
        {0, 100},
        {2, 200},
        {4, 400},
        {6, 600},
        {8, 800},
    };

    record key {4, 0};
    auto result = xer::bsearch(&key, values, compare_record_by_key);

    xer_assert(result.has_value());
    xer_assert_eq(result.value()->key, 4);
    xer_assert_eq(result.value()->value, 400);
    xer_assert_eq(result.value() - values.begin(), 2);
}

/**
 * @brief Tests bsearch with duplicate values.
 *
 * Any equal element may be returned.
 */
void test_bsearch_duplicates()
{
    int values[] = {1, 2, 2, 2, 3, 4, 5};
    int key = 2;

    auto result = xer::bsearch(&key, values, std::size(values), compare_int);

    xer_assert(result.has_value());
    xer_assert_eq(*result.value(), 2);
    xer_assert(result.value() >= values + 1);
    xer_assert(result.value() <= values + 3);
}

/**
 * @brief Tests not_found for a pointer range.
 */
void test_bsearch_not_found_pointer()
{
    int values[] = {1, 3, 5, 7, 9};
    int key = 4;

    auto result = xer::bsearch(&key, values, std::size(values), compare_int);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::not_found);
}

/**
 * @brief Tests not_found for a random-access range.
 */
void test_bsearch_not_found_range()
{
    std::vector<int> values = {1, 3, 5, 7, 9};
    int key = 6;

    auto result = xer::bsearch(&key, values, compare_int);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::not_found);
}

/**
 * @brief Tests count == 0 for a pointer range.
 */
void test_bsearch_empty_pointer_range()
{
    int* values = nullptr;
    int key = 1;

    auto result = xer::bsearch(&key, values, 0, compare_int);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::not_found);
}

/**
 * @brief Tests an empty random-access range.
 */
void test_bsearch_empty_range()
{
    std::vector<int> values;
    int key = 1;

    auto result = xer::bsearch(&key, values, compare_int);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::not_found);
}

/**
 * @brief Tests invalid_argument for a null key in a pointer range.
 */
void test_bsearch_invalid_argument_null_key_pointer()
{
    int values[] = {1, 2, 3};

    auto result = xer::bsearch(
        static_cast<const int*>(nullptr),
        values,
        std::size(values),
        compare_int);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

/**
 * @brief Tests invalid_argument for a null base in a pointer range.
 */
void test_bsearch_invalid_argument_null_base_pointer()
{
    int key = 1;

    auto result = xer::bsearch(
        &key,
        static_cast<int*>(nullptr),
        3,
        compare_int);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

/**
 * @brief Tests invalid_argument for a null key in a random-access range.
 */
void test_bsearch_invalid_argument_null_key_range()
{
    std::vector<int> values = {1, 2, 3};

    auto result = xer::bsearch(
        static_cast<const int*>(nullptr),
        values,
        compare_int);

    xer_assert_not(result.has_value());
    xer_assert_eq(result.error().code, xer::error_t::invalid_argument);
}

} // namespace

/**
 * @brief Program entry point.
 *
 * @return Exit status.
 */
int main()
{
    test_bsearch_pointer_range();
    test_bsearch_const_pointer_range();
    test_bsearch_builtin_array();
    test_bsearch_std_array();
    test_bsearch_const_std_array();
    test_bsearch_std_vector();
    test_bsearch_const_std_vector();
    test_bsearch_std_deque();
    test_bsearch_const_std_deque();
    test_bsearch_span();
    test_bsearch_const_span();
    test_bsearch_lambda_comparator();
    test_bsearch_custom_type();
    test_bsearch_duplicates();
    test_bsearch_not_found_pointer();
    test_bsearch_not_found_range();
    test_bsearch_empty_pointer_range();
    test_bsearch_empty_range();
    test_bsearch_invalid_argument_null_key_pointer();
    test_bsearch_invalid_argument_null_base_pointer();
    test_bsearch_invalid_argument_null_key_range();

    return 0;
}

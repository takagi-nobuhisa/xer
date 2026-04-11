/**
 * @file tests/test_qsort.cpp
 * @brief Runtime tests for xer/bits/qsort.h.
 */

#include <array>
#include <cstddef>
#include <deque>
#include <span>
#include <vector>

#include <xer/assert.h>
#include <xer/bits/qsort.h>

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
 * @brief Sample record used for custom-type sorting tests.
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
 * @brief Asserts that the given integer sequence is sorted in nondecreasing order.
 *
 * @tparam Sequence Sequence type with std::size and operator[].
 * @param values Sequence to validate.
 */
template<typename Sequence>
void assert_sorted_ints(const Sequence& values)
{
    for (std::size_t i = 1; i < std::size(values); ++i) {
        xer_assert(values[i - 1] <= values[i]);
    }
}

/**
 * @brief Tests qsort with a raw pointer range.
 */
void test_qsort_pointer_range()
{
    int values[] = {5, 1, 4, 2, 3, 0, -1, 9, 8, 7, 6};
    xer::qsort(values, std::size(values), compare_int);
    assert_sorted_ints(values);
}

/**
 * @brief Tests qsort with duplicate values.
 */
void test_qsort_duplicates()
{
    int values[] = {4, 2, 4, 1, 3, 2, 1, 4, 0, 0, 3};
    xer::qsort(values, std::size(values), compare_int);

    assert_sorted_ints(values);

    xer_assert_eq(values[0], 0);
    xer_assert_eq(values[1], 0);
    xer_assert_eq(values[2], 1);
    xer_assert_eq(values[3], 1);
    xer_assert_eq(values[4], 2);
    xer_assert_eq(values[5], 2);
    xer_assert_eq(values[6], 3);
    xer_assert_eq(values[7], 3);
    xer_assert_eq(values[8], 4);
    xer_assert_eq(values[9], 4);
    xer_assert_eq(values[10], 4);
}

/**
 * @brief Tests qsort with a built-in array overload.
 */
void test_qsort_builtin_array()
{
    int values[] = {9, 7, 5, 3, 1, 2, 4, 6, 8, 0};
    xer::qsort(values, int_compare{});
    assert_sorted_ints(values);
}

/**
 * @brief Tests qsort with std::array.
 */
void test_qsort_std_array()
{
    std::array<int, 8> values = {8, 6, 4, 2, 7, 5, 3, 1};
    xer::qsort(values, compare_int);
    assert_sorted_ints(values);
}

/**
 * @brief Tests qsort with std::vector.
 */
void test_qsort_std_vector()
{
    std::vector<int> values = {10, -3, 7, 7, 1, 0, -5, 2, 9};
    xer::qsort(values, compare_int);
    assert_sorted_ints(values);
}

/**
 * @brief Tests qsort with std::span.
 */
void test_qsort_span()
{
    std::array<int, 10> values = {100, 9, 7, 5, 3, 1, 2, 4, 6, 8};
    std::span<int> subspan(values.data() + 1, values.size() - 1);

    xer::qsort(subspan, compare_int);

    xer_assert_eq(values[0], 100);

    for (std::size_t i = 2; i < values.size(); ++i) {
        xer_assert(values[i - 1] <= values[i]);
    }
}

/**
 * @brief Tests qsort with std::deque.
 */
void test_qsort_std_deque()
{
    std::deque<int> values = {12, 5, 9, 1, 7, 3, 11, 2, 10, 4, 8, 6};
    xer::qsort(values, compare_int);
    assert_sorted_ints(values);
}

/**
 * @brief Tests qsort with a lambda comparator.
 */
void test_qsort_lambda_comparator()
{
    int values[] = {3, -1, 2, -5, 0, 4, -2};

    xer::qsort(
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

    assert_sorted_ints(values);
}

/**
 * @brief Tests qsort with a custom record type.
 */
void test_qsort_custom_type()
{
    record values[] = {
        {4, 40},
        {1, 10},
        {3, 30},
        {2, 20},
        {1, 11},
        {4, 41},
        {0, 0},
    };

    xer::qsort(values, std::size(values), compare_record_by_key);

    for (std::size_t i = 1; i < std::size(values); ++i) {
        xer_assert(values[i - 1].key <= values[i].key);
    }
}

/**
 * @brief Tests empty ranges.
 */
void test_qsort_empty_ranges()
{
    {
        int* values = nullptr;
        xer::qsort(values, 0, compare_int);
    }

    {
        std::array<int, 0> values = {};
        xer::qsort(values, compare_int);
    }

    {
        std::vector<int> values;
        xer::qsort(values, compare_int);
    }

    {
        std::deque<int> values;
        xer::qsort(values, compare_int);
    }
}

/**
 * @brief Tests a single-element range.
 */
void test_qsort_single_element()
{
    int value = 42;
    xer::qsort(&value, 1, compare_int);
    xer_assert_eq(value, 42);
}

} // namespace

/**
 * @brief Program entry point.
 *
 * @return Exit status.
 */
int main()
{
    test_qsort_pointer_range();
    test_qsort_duplicates();
    test_qsort_builtin_array();
    test_qsort_std_array();
    test_qsort_std_vector();
    test_qsort_span();
    test_qsort_std_deque();
    test_qsort_lambda_comparator();
    test_qsort_custom_type();
    test_qsort_empty_ranges();
    test_qsort_single_element();

    return 0;
}

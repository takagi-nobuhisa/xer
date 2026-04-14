#include <limits>

#include <xer/arithmetic.h>
#include <xer/assert.h>
#include <xer/error.h>

namespace {

void test_in_range_same_integer_type()
{
    xer_assert(xer::in_range<int>(0));
    xer_assert(xer::in_range<int>(123));
    xer_assert(xer::in_range<int>(-456));
}

void test_in_range_signed_to_unsigned()
{
    xer_assert(xer::in_range<unsigned int>(0));
    xer_assert(xer::in_range<unsigned int>(123));
    xer_assert_not(xer::in_range<unsigned int>(-1));
    xer_assert_not(xer::in_range<unsigned int>(-100));
}

void test_in_range_unsigned_to_signed()
{
    xer_assert(xer::in_range<int>(0u));
    xer_assert(xer::in_range<int>(123u));
    xer_assert_not(
        xer::in_range<int>(std::numeric_limits<unsigned int>::max()));
}

void test_in_range_integer_boundaries()
{
    xer_assert(xer::in_range<signed char>(static_cast<int>(-128)));
    xer_assert(xer::in_range<signed char>(static_cast<int>(127)));
    xer_assert_not(xer::in_range<signed char>(static_cast<int>(-129)));
    xer_assert_not(xer::in_range<signed char>(static_cast<int>(128)));
}

void test_in_range_bool_value_is_allowed()
{
    xer_assert(xer::in_range<int>(false));
    xer_assert(xer::in_range<int>(true));
    xer_assert(xer::in_range<unsigned int>(false));
    xer_assert(xer::in_range<unsigned int>(true));
}

void test_in_range_floating_to_integer()
{
    xer_assert(xer::in_range<int>(0.0));
    xer_assert(xer::in_range<int>(1.5));
    xer_assert(xer::in_range<int>(-1.5));
    xer_assert_not(
        xer::in_range<int>(
            static_cast<double>(std::numeric_limits<int>::max()) * 2.0));
    xer_assert_not(
        xer::in_range<int>(
            static_cast<double>(std::numeric_limits<int>::lowest()) * 2.0));
}

void test_in_range_integer_to_floating()
{
    xer_assert(xer::in_range<float>(0));
    xer_assert(xer::in_range<float>(123));
    xer_assert(xer::in_range<double>(std::numeric_limits<int>::max()));
}

void test_in_range_floating_to_floating()
{
    xer_assert(xer::in_range<float>(0.0));
    xer_assert(xer::in_range<float>(123.5));
    xer_assert(xer::in_range<double>(std::numeric_limits<float>::max()));
    xer_assert_not(
        xer::in_range<float>(
            static_cast<double>(std::numeric_limits<float>::max()) * 2.0));
    xer_assert_not(
        xer::in_range<float>(
            static_cast<double>(std::numeric_limits<float>::lowest()) * 2.0));
}

void test_in_range_nan_is_false()
{
    xer_assert_not(
        xer::in_range<float>(std::numeric_limits<double>::quiet_NaN()));
    xer_assert_not(
        xer::in_range<int>(std::numeric_limits<double>::quiet_NaN()));
}

void test_in_range_positive_infinity_is_false()
{
    xer_assert_not(
        xer::in_range<float>(std::numeric_limits<double>::infinity()));
    xer_assert_not(
        xer::in_range<int>(std::numeric_limits<double>::infinity()));
}

void test_in_range_negative_infinity_is_false()
{
    xer_assert_not(
        xer::in_range<float>(-std::numeric_limits<double>::infinity()));
    xer_assert_not(
        xer::in_range<int>(-std::numeric_limits<double>::infinity()));
}

void test_in_range_result_success()
{
    const xer::result<int> value = 123;

    xer_assert(xer::in_range<short>(value));
    xer_assert(xer::in_range<unsigned char>(value));

    const xer::result<int> value2 = 1000;
    xer_assert_not(xer::in_range<unsigned char>(value2));
}

void test_in_range_result_bool_value_success()
{
    const xer::result<bool> value_true = true;
    xer_assert(xer::in_range<int>(value_true));

    const xer::result<bool> value_false = false;
    xer_assert(xer::in_range<unsigned int>(value_false));
}

void test_in_range_result_error_returns_false()
{
    const xer::result<int> value =
        std::unexpected(xer::make_error(xer::error_t::invalid_argument));

    xer_assert_not(xer::in_range<short>(value));
}

} // namespace

int main()
{
    test_in_range_same_integer_type();
    test_in_range_signed_to_unsigned();
    test_in_range_unsigned_to_signed();
    test_in_range_integer_boundaries();
    test_in_range_bool_value_is_allowed();
    test_in_range_floating_to_integer();
    test_in_range_integer_to_floating();
    test_in_range_floating_to_floating();
    test_in_range_nan_is_false();
    test_in_range_positive_infinity_is_false();
    test_in_range_negative_infinity_is_false();
    test_in_range_result_success();
    test_in_range_result_bool_value_success();
    test_in_range_result_error_returns_false();

    return 0;
}

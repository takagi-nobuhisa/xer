#include <xer/arithmetic.h>
#include <xer/assert.h>
#include <xer/error.h>

#include <limits>
#include <type_traits>

namespace {

auto test_sq_integer_success() -> void
{
    const auto value = xer::sq(12);

    xer_assert(value.has_value());
    xer_assert_eq(*value, 144);

    static_assert(std::same_as<decltype(value), const xer::result<xer::int64_t>>);
}

auto test_cb_integer_success() -> void
{
    const auto value = xer::cb(-3);

    xer_assert(value.has_value());
    xer_assert_eq(*value, -27);

    static_assert(std::same_as<decltype(value), const xer::result<xer::int64_t>>);
}

auto test_sq_unsigned_success() -> void
{
    const auto value = xer::sq(12u);

    xer_assert(value.has_value());
    xer_assert_eq(*value, 144);
}

auto test_cb_unsigned_success() -> void
{
    const auto value = xer::cb(3u);

    xer_assert(value.has_value());
    xer_assert_eq(*value, 27);
}

auto test_sq_narrow_integer_forwarding() -> void
{
    const auto value = xer::sq(static_cast<short>(11));

    xer_assert(value.has_value());
    xer_assert_eq(*value, 121);
}

auto test_cb_narrow_integer_forwarding() -> void
{
    const auto value = xer::cb(static_cast<unsigned char>(4));

    xer_assert(value.has_value());
    xer_assert_eq(*value, 64);
}

auto test_sq_integer_out_of_range() -> void
{
    const auto value = xer::sq(std::numeric_limits<xer::int64_t>::max());

    xer_assert_not(value.has_value());
    xer_assert_eq(value.error().code, xer::error_t::out_of_range);
}

auto test_cb_integer_out_of_range() -> void
{
    const auto value = xer::cb(2'100'000);

    xer_assert_not(value.has_value());
    xer_assert_eq(value.error().code, xer::error_t::out_of_range);
}

auto test_sq_floating_success() -> void
{
    const auto value = xer::sq(1.5);

    xer_assert(value.has_value());
    xer_assert_eq(*value, 2.25L);

    static_assert(std::same_as<decltype(value), const xer::result<long double>>);
}

auto test_cb_floating_success() -> void
{
    const auto value = xer::cb(2.0f);

    xer_assert(value.has_value());
    xer_assert_eq(*value, 8.0L);
}

auto test_sq_floating_domain_error() -> void
{
    const auto value = xer::sq(std::numeric_limits<double>::infinity());

    xer_assert_not(value.has_value());
    xer_assert_eq(value.error().code, xer::error_t::dom);
}

auto test_cb_floating_domain_error() -> void
{
    const auto value = xer::cb(std::numeric_limits<double>::quiet_NaN());

    xer_assert_not(value.has_value());
    xer_assert_eq(value.error().code, xer::error_t::dom);
}

auto test_result_argument_success() -> void
{
    const xer::result<int> value = 5;

    const auto squared = xer::sq(value);
    const auto cubed = xer::cb(value);

    xer_assert(squared.has_value());
    xer_assert(cubed.has_value());
    xer_assert_eq(*squared, 25);
    xer_assert_eq(*cubed, 125);
}

auto test_result_argument_error_propagation() -> void
{
    const xer::result<int> value = std::unexpected(
        xer::make_error(xer::error_t::invalid_argument));

    const auto squared = xer::sq(value);
    const auto cubed = xer::cb(value);

    xer_assert_not(squared.has_value());
    xer_assert_not(cubed.has_value());
    xer_assert_eq(squared.error().code, xer::error_t::invalid_argument);
    xer_assert_eq(cubed.error().code, xer::error_t::invalid_argument);
}

auto test_chained_arithmetic() -> void
{
    const auto value = xer::sq(xer::add(2, 3));

    xer_assert(value.has_value());
    xer_assert_eq(*value, 25);
}

} // namespace

auto main() -> int
{
    test_sq_integer_success();
    test_cb_integer_success();
    test_sq_unsigned_success();
    test_cb_unsigned_success();
    test_sq_narrow_integer_forwarding();
    test_cb_narrow_integer_forwarding();
    test_sq_integer_out_of_range();
    test_cb_integer_out_of_range();
    test_sq_floating_success();
    test_cb_floating_success();
    test_sq_floating_domain_error();
    test_cb_floating_domain_error();
    test_result_argument_success();
    test_result_argument_error_propagation();
    test_chained_arithmetic();

    return 0;
}

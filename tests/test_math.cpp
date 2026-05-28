#include <optional>

#include <xer/assert.h>
#include <xer/error.h>
#include <xer/math.h>

namespace {

void test_quadratic_two_real_roots()
{
    const auto roots = xer::quadratic(1.0, -5.0, 6.0);

    xer_assert(roots.has_value());
    xer_assert(roots->at(0).has_value());
    xer_assert(roots->at(1).has_value());
    xer_assert(xer::detail::equation_near(*roots->at(0), 2.0));
    xer_assert(xer::detail::equation_near(*roots->at(1), 3.0));
}

void test_quadratic_one_real_root()
{
    const auto roots = xer::quadratic(1.0, -2.0, 1.0);

    xer_assert(roots.has_value());
    xer_assert(roots->at(0).has_value());
    xer_assert_not(roots->at(1).has_value());
    xer_assert(xer::detail::equation_near(*roots->at(0), 1.0));
}

void test_quadratic_no_real_root()
{
    const auto roots = xer::quadratic(1.0, 0.0, 1.0);

    xer_assert(roots.has_value());
    xer_assert_not(roots->at(0).has_value());
    xer_assert_not(roots->at(1).has_value());
}

void test_quadratic_invalid_argument()
{
    const auto roots = xer::quadratic(0.0, 2.0, 1.0);

    xer_assert_not(roots.has_value());
    xer_assert(roots.error().code == xer::error_t::invalid_argument);
}

void test_cubic_three_real_roots()
{
    const auto roots = xer::cubic(1.0, -6.0, 11.0, -6.0);

    xer_assert(roots.has_value());
    xer_assert(roots->at(0).has_value());
    xer_assert(roots->at(1).has_value());
    xer_assert(roots->at(2).has_value());
    xer_assert(xer::detail::equation_near(*roots->at(0), 1.0));
    xer_assert(xer::detail::equation_near(*roots->at(1), 2.0));
    xer_assert(xer::detail::equation_near(*roots->at(2), 3.0));
}

void test_cubic_one_real_root()
{
    const auto roots = xer::cubic(1.0, 0.0, 0.0, -8.0);

    xer_assert(roots.has_value());
    xer_assert(roots->at(0).has_value());
    xer_assert_not(roots->at(1).has_value());
    xer_assert_not(roots->at(2).has_value());
    xer_assert(xer::detail::equation_near(*roots->at(0), 2.0));
}

void test_cubic_multiple_real_roots()
{
    const auto roots = xer::cubic(1.0, -3.0, 3.0, -1.0);

    xer_assert(roots.has_value());
    xer_assert(roots->at(0).has_value());
    xer_assert_not(roots->at(1).has_value());
    xer_assert_not(roots->at(2).has_value());
    xer_assert(xer::detail::equation_near(*roots->at(0), 1.0));
}

void test_cubic_invalid_argument()
{
    const auto roots = xer::cubic(0.0, 1.0, 2.0, 3.0);

    xer_assert_not(roots.has_value());
    xer_assert(roots.error().code == xer::error_t::invalid_argument);
}

} // namespace

auto main() -> int
{
    test_quadratic_two_real_roots();
    test_quadratic_one_real_root();
    test_quadratic_no_real_root();
    test_quadratic_invalid_argument();
    test_cubic_three_real_roots();
    test_cubic_one_real_root();
    test_cubic_multiple_real_roots();
    test_cubic_invalid_argument();
    return 0;
}

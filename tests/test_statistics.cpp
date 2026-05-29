#include <cmath>
#include <limits>
#include <sstream>
#include <vector>

#include <xer/assert.h>
#include <xer/error.h>
#include <xer/statistics.h>

namespace {

void assert_close(double actual, double expected, double epsilon = 1e-12)
{
    xer_assert(std::fabs(actual - expected) <= epsilon);
}

void test_mean_with_vector()
{
    const std::vector<int> values{1, 2, 3, 4};
    const auto value = xer::mean(values);

    xer_assert(value.has_value());
    assert_close(*value, 2.5);
}

void test_mean_with_initializer_list()
{
    const auto value = xer::mean({1.0, 2.0, 3.0});

    xer_assert(value.has_value());
    assert_close(*value, 2.0);
}

void test_variance_and_stddev()
{
    const std::vector<double> values{2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0};

    const auto v = xer::variance(values);
    const auto s = xer::stddev(values);

    xer_assert(v.has_value());
    xer_assert(s.has_value());
    assert_close(*v, 4.0);
    assert_close(*s, 2.0);
}

void test_sample_variance_and_stddev()
{
    const std::vector<double> values{2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0};

    const auto v = xer::sample_variance(values);
    const auto s = xer::sample_stddev(values);

    xer_assert(v.has_value());
    xer_assert(s.has_value());
    assert_close(*v, 32.0 / 7.0);
    assert_close(*s, std::sqrt(32.0 / 7.0));
}

void test_single_value()
{
    const std::vector<int> values{42};

    const auto m = xer::mean(values);
    const auto v = xer::variance(values);
    const auto s = xer::stddev(values);
    const auto sv = xer::sample_variance(values);
    const auto ss = xer::sample_stddev(values);

    xer_assert(m.has_value());
    xer_assert(v.has_value());
    xer_assert(s.has_value());
    assert_close(*m, 42.0);
    assert_close(*v, 0.0);
    assert_close(*s, 0.0);
    xer_assert_not(sv.has_value());
    xer_assert_not(ss.has_value());
    xer_assert(sv.error().code == xer::error_t::invalid_argument);
    xer_assert(ss.error().code == xer::error_t::invalid_argument);
}

void test_empty_range()
{
    const std::vector<int> values;
    const auto value = xer::mean(values);

    xer_assert_not(value.has_value());
    xer_assert(value.error().code == xer::error_t::invalid_argument);
}

void test_invalid_floating_value()
{
    const std::vector<double> values{1.0, std::numeric_limits<double>::infinity()};
    const auto value = xer::mean(values);

    xer_assert_not(value.has_value());
    xer_assert(value.error().code == xer::error_t::invalid_argument);
}

void test_input_range()
{
    std::istringstream input("1 2 3 4");
    auto values = std::views::istream<int>(input);
    const auto value = xer::mean(values);

    xer_assert(value.has_value());
    assert_close(*value, 2.5);
}

} // namespace

auto main() -> int
{
    test_mean_with_vector();
    test_mean_with_initializer_list();
    test_variance_and_stddev();
    test_sample_variance_and_stddev();
    test_single_value();
    test_empty_range();
    test_invalid_floating_value();
    test_input_range();
    return 0;
}

#include <cmath>
#include <limits>
#include <ranges>
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

void test_median_with_odd_count()
{
    const std::vector<int> values{5, 1, 9, 3, 7};
    const auto value = xer::median(values);

    xer_assert(value.has_value());
    assert_close(*value, 5.0);
}

void test_median_with_even_count()
{
    const auto value = xer::median({10.0, 2.0, 8.0, 4.0});

    xer_assert(value.has_value());
    assert_close(*value, 6.0);
}

void test_quantile_basic_values()
{
    const std::vector<double> values{10.0, 20.0, 30.0, 40.0, 50.0};

    const auto q0 = xer::quantile(values, 0.0);
    const auto q25 = xer::quantile(values, 0.25);
    const auto q50 = xer::quantile(values, 0.5);
    const auto q75 = xer::quantile(values, 0.75);
    const auto q100 = xer::quantile(values, 1.0);

    xer_assert(q0.has_value());
    xer_assert(q25.has_value());
    xer_assert(q50.has_value());
    xer_assert(q75.has_value());
    xer_assert(q100.has_value());
    assert_close(*q0, 10.0);
    assert_close(*q25, 20.0);
    assert_close(*q50, 30.0);
    assert_close(*q75, 40.0);
    assert_close(*q100, 50.0);
}

void test_quantile_with_interpolation()
{
    const auto q = xer::quantile({0.0, 10.0, 20.0, 30.0}, 0.25);

    xer_assert(q.has_value());
    assert_close(*q, 7.5);
}

void test_quantile_matches_median()
{
    const std::vector<int> values{4, 1, 3, 2};

    const auto q = xer::quantile(values, 0.5);
    const auto m = xer::median(values);

    xer_assert(q.has_value());
    xer_assert(m.has_value());
    assert_close(*q, *m);
}

void test_percentile_basic_values()
{
    const std::vector<double> values{10.0, 20.0, 30.0, 40.0, 50.0};

    const auto p25 = xer::percentile(values, 25.0);
    const auto p50 = xer::percentile(values, 50.0);
    const auto p75 = xer::percentile(values, 75.0);

    xer_assert(p25.has_value());
    xer_assert(p50.has_value());
    xer_assert(p75.has_value());
    assert_close(*p25, 20.0);
    assert_close(*p50, 30.0);
    assert_close(*p75, 40.0);
}

void test_percentile_with_interpolation()
{
    const auto p = xer::percentile({0.0, 10.0, 20.0, 30.0}, 25.0);

    xer_assert(p.has_value());
    assert_close(*p, 7.5);
}

void test_quantile_invalid_fraction()
{
    const std::vector<double> values{1.0, 2.0, 3.0};

    const auto low = xer::quantile(values, -0.01);
    const auto high = xer::quantile(values, 1.01);
    const auto nan = xer::quantile(values, std::numeric_limits<double>::quiet_NaN());

    xer_assert_not(low.has_value());
    xer_assert_not(high.has_value());
    xer_assert_not(nan.has_value());
    xer_assert(low.error().code == xer::error_t::invalid_argument);
    xer_assert(high.error().code == xer::error_t::invalid_argument);
    xer_assert(nan.error().code == xer::error_t::invalid_argument);
}

void test_percentile_invalid_value()
{
    const std::vector<double> values{1.0, 2.0, 3.0};

    const auto low = xer::percentile(values, -0.01);
    const auto high = xer::percentile(values, 100.01);
    const auto inf = xer::percentile(values, std::numeric_limits<double>::infinity());

    xer_assert_not(low.has_value());
    xer_assert_not(high.has_value());
    xer_assert_not(inf.has_value());
    xer_assert(low.error().code == xer::error_t::invalid_argument);
    xer_assert(high.error().code == xer::error_t::invalid_argument);
    xer_assert(inf.error().code == xer::error_t::invalid_argument);
}


void test_mode_with_single_mode()
{
    const std::vector<int> values{1, 2, 2, 3, 4};
    const auto modes = xer::mode(values);

    xer_assert(modes.has_value());
    xer_assert(modes->size() == 1);
    assert_close((*modes)[0], 2.0);
}

void test_mode_with_multiple_modes()
{
    const auto modes = xer::mode({3, 1, 2, 2, 3, 4});

    xer_assert(modes.has_value());
    xer_assert(modes->size() == 2);
    assert_close((*modes)[0], 2.0);
    assert_close((*modes)[1], 3.0);
}

void test_mode_without_repeated_value()
{
    const auto modes = xer::mode({1, 2, 3, 4});

    xer_assert(modes.has_value());
    xer_assert(modes->empty());
}

void test_mode_with_tolerance()
{
    const std::vector<double> values{10.0, 1.04, 2.0, 1.00, 2.04, 1.02};
    const auto modes = xer::mode(values, 0.05);

    xer_assert(modes.has_value());
    xer_assert(modes->size() == 1);
    assert_close((*modes)[0], 1.02);
}

void test_mode_with_invalid_tolerance()
{
    const auto modes = xer::mode({1.0, 1.0}, -0.01);

    xer_assert_not(modes.has_value());
    xer_assert(modes.error().code == xer::error_t::invalid_argument);
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
    const auto med = xer::median(values);
    const auto modes = xer::mode(values);
    const auto v = xer::variance(values);
    const auto s = xer::stddev(values);
    const auto sv = xer::sample_variance(values);
    const auto ss = xer::sample_stddev(values);

    xer_assert(m.has_value());
    xer_assert(med.has_value());
    xer_assert(modes.has_value());
    xer_assert(v.has_value());
    xer_assert(s.has_value());
    assert_close(*m, 42.0);
    assert_close(*med, 42.0);
    xer_assert(modes->empty());
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
    const auto mean = xer::mean(values);
    const auto median = xer::median(values);
    const auto quantile = xer::quantile(values, 0.5);
    const auto percentile = xer::percentile(values, 50.0);
    const auto modes = xer::mode(values);

    xer_assert_not(mean.has_value());
    xer_assert_not(median.has_value());
    xer_assert_not(quantile.has_value());
    xer_assert_not(percentile.has_value());
    xer_assert_not(modes.has_value());
    xer_assert(mean.error().code == xer::error_t::invalid_argument);
    xer_assert(median.error().code == xer::error_t::invalid_argument);
    xer_assert(quantile.error().code == xer::error_t::invalid_argument);
    xer_assert(percentile.error().code == xer::error_t::invalid_argument);
    xer_assert(modes.error().code == xer::error_t::invalid_argument);
}

void test_invalid_floating_value()
{
    const std::vector<double> values{1.0, std::numeric_limits<double>::infinity()};
    const auto value = xer::mean(values);
    const auto med = xer::median(values);
    const auto q = xer::quantile(values, 0.5);
    const auto p = xer::percentile(values, 50.0);
    const auto modes = xer::mode(values);

    xer_assert_not(value.has_value());
    xer_assert_not(med.has_value());
    xer_assert_not(q.has_value());
    xer_assert_not(p.has_value());
    xer_assert_not(modes.has_value());
    xer_assert(value.error().code == xer::error_t::invalid_argument);
    xer_assert(med.error().code == xer::error_t::invalid_argument);
    xer_assert(q.error().code == xer::error_t::invalid_argument);
    xer_assert(p.error().code == xer::error_t::invalid_argument);
    xer_assert(modes.error().code == xer::error_t::invalid_argument);
}

void test_input_range()
{
    std::istringstream input("1 2 3 4");
    auto values = std::views::istream<int>(input);
    const auto value = xer::mean(values);

    xer_assert(value.has_value());
    assert_close(*value, 2.5);
}

void test_median_with_input_range()
{
    std::istringstream input("4 1 3 2");
    auto values = std::views::istream<int>(input);
    const auto value = xer::median(values);

    xer_assert(value.has_value());
    assert_close(*value, 2.5);
}

void test_quantile_with_input_range()
{
    std::istringstream input("4 1 3 2");
    auto values = std::views::istream<int>(input);
    const auto value = xer::quantile(values, 0.5);

    xer_assert(value.has_value());
    assert_close(*value, 2.5);
}

void test_percentile_with_input_range()
{
    std::istringstream input("4 1 3 2");
    auto values = std::views::istream<int>(input);
    const auto value = xer::percentile(values, 50.0);

    xer_assert(value.has_value());
    assert_close(*value, 2.5);
}

} // namespace

auto main() -> int
{
    test_mean_with_vector();
    test_mean_with_initializer_list();
    test_median_with_odd_count();
    test_median_with_even_count();
    test_quantile_basic_values();
    test_quantile_with_interpolation();
    test_quantile_matches_median();
    test_percentile_basic_values();
    test_percentile_with_interpolation();
    test_quantile_invalid_fraction();
    test_percentile_invalid_value();
    test_mode_with_single_mode();
    test_mode_with_multiple_modes();
    test_mode_without_repeated_value();
    test_mode_with_tolerance();
    test_mode_with_invalid_tolerance();
    test_variance_and_stddev();
    test_sample_variance_and_stddev();
    test_single_value();
    test_empty_range();
    test_invalid_floating_value();
    test_input_range();
    test_median_with_input_range();
    test_quantile_with_input_range();
    test_percentile_with_input_range();
    return 0;
}

// XER_EXAMPLE_BEGIN: statistics_basic
//
// This example computes basic statistics for a small data set.
//
// Expected output:
// mean: 5
// median: 4.5
// quantile 25%: 4
// percentile 75: 5.5
// variance: 4
// stddev: 2
// sample_variance: 4.57143
// sample_stddev: 2.13809
// modes: 4
// tolerant modes: 1.02

#include <iostream>
#include <vector>

#include <xer/statistics.h>

namespace {

auto print_result(const char* label, const xer::result<double>& value) -> bool
{
    if (!value) {
        return false;
    }

    std::cout << label << ": " << *value << "\n";
    return true;
}

auto print_modes(
    const char* label,
    const xer::result<std::vector<double>>& modes) -> bool
{
    if (!modes) {
        return false;
    }

    std::cout << label << ":";
    for (const double value : *modes) {
        std::cout << ' ' << value;
    }
    std::cout << "\n";
    return true;
}

} // namespace

auto main() -> int
{
    const std::vector<double> values{2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0};
    const std::vector<double> near_values{1.00, 1.02, 1.04, 2.0, 2.04};

    if (!print_result("mean", xer::mean(values)) ||
        !print_result("median", xer::median(values)) ||
        !print_result("quantile 25%", xer::quantile(values, 0.25)) ||
        !print_result("percentile 75", xer::percentile(values, 75.0)) ||
        !print_result("variance", xer::variance(values)) ||
        !print_result("stddev", xer::stddev(values)) ||
        !print_result("sample_variance", xer::sample_variance(values)) ||
        !print_result("sample_stddev", xer::sample_stddev(values)) ||
        !print_modes("modes", xer::mode(values)) ||
        !print_modes("tolerant modes", xer::mode(near_values, 0.05))) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: statistics_basic

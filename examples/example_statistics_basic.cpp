// XER_EXAMPLE_BEGIN: statistics_basic
//
// This example computes basic statistics for a small data set.
//
// Expected output:
// mean: 5
// sum: 40
// product: 201600
// median: 4.5
// quantile 25%: 4
// percentile 75: 5.5
// variance: 4
// stddev: 2
// sample_variance: 4.57143
// sample_stddev: 2.13809
// modes: 4
// tolerant modes: 1.02
// geometric_mean: 4
// harmonic_mean: 2.28571

#include <iostream>
#include <vector>

#include <xer/diagnostics.h>
#include <xer/statistics.h>

namespace {

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
    const std::vector<double> mean_values{1.0, 4.0, 16.0};

    if (!xer_print(u8"mean", xer::mean(values)) ||
        !xer_print(u8"sum", xer::sum(values)) ||
        !xer_print(u8"product", xer::product(values)) ||
        !xer_print(u8"median", xer::median(values)) ||
        !xer_print(u8"quantile 25%", xer::quantile(values, 0.25)) ||
        !xer_print(u8"percentile 75", xer::percentile(values, 75.0)) ||
        !xer_print(u8"variance", xer::variance(values)) ||
        !xer_print(u8"stddev", xer::stddev(values)) ||
        !xer_print(u8"sample_variance", xer::sample_variance(values)) ||
        !xer_print(u8"sample_stddev", xer::sample_stddev(values)) ||
        !print_modes("modes", xer::mode(values)) ||
        !print_modes("tolerant modes", xer::mode(near_values, 0.05)) ||
        !xer_print(u8"geometric_mean", xer::geometric_mean(mean_values)) ||
        !xer_print(u8"harmonic_mean", xer::harmonic_mean(mean_values))) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: statistics_basic

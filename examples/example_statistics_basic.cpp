// XER_EXAMPLE_BEGIN: statistics_basic
//
// This example computes basic statistics for a small data set.
//
// Expected output:
// mean: 5
// variance: 4
// stddev: 2
// sample_variance: 4.57143
// sample_stddev: 2.13809

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

} // namespace

auto main() -> int
{
    const std::vector<double> values{2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0};

    if (!print_result("mean", xer::mean(values)) ||
        !print_result("variance", xer::variance(values)) ||
        !print_result("stddev", xer::stddev(values)) ||
        !print_result("sample_variance", xer::sample_variance(values)) ||
        !print_result("sample_stddev", xer::sample_stddev(values))) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: statistics_basic

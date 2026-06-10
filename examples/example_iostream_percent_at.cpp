#include <iostream>
#include <sstream>
#include <string>

#include <xer/color.h>
#include <xer/cyclic.h>
#include <xer/interval.h>
#include <xer/iostream.h>
#include <xer/path.h>
#include <xer/stdio.h>

// XER_EXAMPLE_BEGIN: iostream_percent_at
//
// This example formats xer value types with %@.
// It also reads selected xer value types through their ordinary iostream
// extraction operators.
//
// Expected output:
// Formatted: path=logs/output.txt angle=0.25 gain=1 color=rgb(1, 0.5, 0)
// Stream input: logs/output.txt, 0.75

auto main() -> int
{
    const auto path = xer::path(u8"logs\\output.txt");
    const auto angle = xer::cyclic<double>(1.25);
    const auto gain = xer::interval<double>(1.25);
    const auto color = xer::rgb(1.0f, 0.5f, 0.0f);

    std::u8string formatted;
    if (!xer::sprintf(
             formatted,
             u8"path=%@ angle=%@ gain=%@ color=%@",
             path,
             angle,
             gain,
             color)) {
        return 1;
    }

    if (!xer::printf(u8"Formatted: %s\n", formatted.c_str())) {
        return 1;
    }

    std::istringstream input("logs/output.txt -0.25");
    xer::path scanned_path;
    xer::cyclic<double> scanned_angle;

    input >> scanned_path >> scanned_angle;
    if (!input) {
        return 1;
    }

    std::cout << "Stream input: " << scanned_path << ", " << scanned_angle
              << '\n';

    return 0;
}

// XER_EXAMPLE_END: iostream_percent_at

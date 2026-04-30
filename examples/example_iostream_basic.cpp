#include <iostream>
#include <sstream>

#include <xer/cyclic.h>
#include <xer/interval.h>
#include <xer/iostream.h>
#include <xer/path.h>

// XER_EXAMPLE_BEGIN: iostream_basic
//
// This example uses iostream operators for selected XER value types.
// These operators are mainly intended to support XER diagnostics and `%@`
// formatting, but they can also be used directly when iostreams are convenient.
//
// Expected output:
// Path: work/file.txt
// Cyclic: 0.25
// Interval: 1
// Read: logs/output.txt, 0.75, 0.5

auto main() -> int
{
    const auto path = xer::path(u8"work\\file.txt");
    const auto angle = xer::cyclic<double>(1.25);
    const auto gain = xer::interval<double>(1.25);

    std::cout << "Path: " << path << '\n';
    std::cout << "Cyclic: " << angle << '\n';
    std::cout << "Interval: " << gain << '\n';

    std::istringstream input("logs\\output.txt -0.25 0.5");
    xer::path read_path;
    xer::cyclic<double> read_angle;
    xer::interval<double> read_gain;

    input >> read_path >> read_angle >> read_gain;
    if (!input) {
        return 1;
    }

    std::cout << "Read: " << read_path << ", " << read_angle << ", "
              << read_gain << '\n';

    return 0;
}

// XER_EXAMPLE_END: iostream_basic

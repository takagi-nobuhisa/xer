#include <iostream>
#include <sstream>

#include <xer/color.h>
#include <xer/cyclic.h>
#include <xer/interval.h>
#include <xer/iostream.h>
#include <xer/matrix.h>
#include <xer/path.h>
#include <xer/quantity.h>

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
// Quantity: 1500
// Matrix: [[1, 2], [3, 4]]
// Color: rgb(1, 0.5, 0)
// Read: logs/output.txt, 0.75, 0.5, 2.5

auto main() -> int
{
    using namespace xer::units;

    const auto path = xer::path(u8"work\\file.txt");
    const auto angle = xer::cyclic<double>(1.25);
    const auto gain = xer::interval<double>(1.25);
    const auto distance = 1.5 * km;
    const auto transform = xer::matrix<double, 2, 2>(1.0, 2.0, 3.0, 4.0);
    const auto color = xer::rgb(1.0f, 0.5f, 0.0f);

    std::cout << "Path: " << path << '\n';
    std::cout << "Cyclic: " << angle << '\n';
    std::cout << "Interval: " << gain << '\n';
    std::cout << "Quantity: " << distance << '\n';
    std::cout << "Matrix: " << transform << '\n';
    std::cout << "Color: " << color << '\n';

    std::istringstream input("logs\\output.txt -0.25 0.5 2.5");
    xer::path read_path;
    xer::cyclic<double> read_angle;
    xer::interval<double> read_gain;
    xer::quantity<double, xer::units::length_dim> read_distance;

    input >> read_path >> read_angle >> read_gain >> read_distance;
    if (!input) {
        return 1;
    }

    std::cout << "Read: " << read_path << ", " << read_angle << ", "
              << read_gain << ", " << read_distance << '\n';

    return 0;
}

// XER_EXAMPLE_END: iostream_basic

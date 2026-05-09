// XER_EXAMPLE_BEGIN: image_geometry_io
//
// This example uses the geometry helper constructors and the iostream
// operators for point, size, and rect.
//
// Expected output:
// point = (2, 3)
// size = {4, 5}
// rect = (2, 3) {4, 5}
// parsed = (10, 20) {30, 40}

#include <sstream>

#include <xer/image.h>
#include <xer/iostream.h>
#include <xer/stdio.h>

namespace {

auto print_geometry(
    const xer::image::point& p,
    const xer::image::size& s,
    const xer::image::rect& r) -> bool
{
    return xer::printf(u8"point = %@\n", p).has_value() &&
           xer::printf(u8"size = %@\n", s).has_value() &&
           xer::printf(u8"rect = %@\n", r).has_value();
}

} // namespace

auto main() -> int
{
    const auto p = xer::image::point(2, 3);
    const auto s = xer::image::size(4, 5);
    const auto r = xer::image::rect(p, s);

    if (!print_geometry(p, s, r)) {
        return 1;
    }

    std::istringstream input("(10, 20) {30, 40}");
    xer::image::rect parsed;
    input >> parsed;
    if (!input) {
        return 1;
    }

    return xer::printf(u8"parsed = %@\n", parsed).has_value() ? 0 : 1;
}

// XER_EXAMPLE_END: image_geometry_io

#include <iostream>

#include <xer/color.h>
#include <xer/iostream.h>

// XER_EXAMPLE_BEGIN: color_conversion
//
// This example follows the main color conversion relationships.
//
// Expected output:
// Source RGB: rgb(0.25, 0.5, 0.75)
// CMY: cmy(0.75, 0.5, 0.25)
// RGB from CMY: rgb(0.25, 0.5, 0.75)
// HSV: hsv(0.583333, 0.666667, 0.75)
// RGB from HSV: rgb(0.25, 0.5, 0.75)
// XYZ: xyz(0.191803, 0.201624, 0.523052)
// RGB from XYZ: rgb(0.25, 0.5, 0.75)
// Lab: lab(52.018, 0.0952846, -39.3692)
// XYZ from Lab: xyz(0.191803, 0.201624, 0.523052)
// Luv: luv(52.018, -28.3934, -60.8235)
// XYZ from Luv: xyz(0.191803, 0.201624, 0.523052)
// Gray: gray(0.4649)

auto main() -> int
{
    const auto source = xer::rgb(0.25f, 0.5f, 0.75f);

    std::cout << "Source RGB: " << source << '\n';

    const auto cmy = xer::to_cmy(source);
    std::cout << "CMY: " << cmy << '\n';
    std::cout << "RGB from CMY: " << xer::to_rgb(cmy) << '\n';

    const auto hsv = xer::to_hsv(source);
    std::cout << "HSV: " << hsv << '\n';
    std::cout << "RGB from HSV: " << xer::to_rgb(hsv) << '\n';

    const auto xyz = xer::to_xyz(source);
    std::cout << "XYZ: " << xyz << '\n';
    std::cout << "RGB from XYZ: " << xer::to_rgb(xyz) << '\n';

    const auto lab = xer::to_lab(xyz);
    std::cout << "Lab: " << lab << '\n';
    std::cout << "XYZ from Lab: " << xer::to_xyz(lab) << '\n';

    const auto luv = xer::to_luv(xyz);
    std::cout << "Luv: " << luv << '\n';
    std::cout << "XYZ from Luv: " << xer::to_xyz(luv) << '\n';

    std::cout << "Gray: " << xer::to_gray(source) << '\n';

    return 0;
}

// XER_EXAMPLE_END: color_conversion

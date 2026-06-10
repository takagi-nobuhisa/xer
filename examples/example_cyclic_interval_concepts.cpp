#include <iostream>

#include <xer/color.h>
#include <xer/cyclic.h>
#include <xer/interval.h>

// XER_EXAMPLE_BEGIN: cyclic_interval_concepts
//
// This example compares xer::interval and xer::cyclic.
//
// Expected output:
// interval(1.25) = 1
// interval(-0.25) = 0
// cyclic(1.25) = 0.25
// cyclic(-0.25) = 0.75
// 450 degrees as cyclic = 90
// RGB red component = 1
// HSV hue = 0.25

auto main() -> int
{
    const auto high = xer::interval<float>(1.25f);
    const auto low = xer::interval<float>(-0.25f);

    std::cout << "interval(1.25) = " << high.value() << '\n';
    std::cout << "interval(-0.25) = " << low.value() << '\n';

    const auto wrapped_high = xer::cyclic<float>(1.25f);
    const auto wrapped_low = xer::cyclic<float>(-0.25f);

    std::cout << "cyclic(1.25) = " << wrapped_high.value() << '\n';
    std::cout << "cyclic(-0.25) = " << wrapped_low.value() << '\n';

    const auto right_angle = xer::from_degree<float>(450.0f);
    std::cout << "450 degrees as cyclic = " << xer::to_degree(right_angle)
              << '\n';

    const auto rgb = xer::rgb(1.25f, 0.5f, -0.25f);
    const auto hsv = xer::hsv(1.25f, 0.5f, 1.0f);

    std::cout << "RGB red component = " << rgb.r.value() << '\n';
    std::cout << "HSV hue = " << hsv.h.value() << '\n';

    return 0;
}

// XER_EXAMPLE_END: cyclic_interval_concepts

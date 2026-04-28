// XER_EXAMPLE_BEGIN: interval_basic
//
// This example shows basic use of bounded interval values.
//
// Expected output:
// r = 1
// g = 0.5
// b = 0
// brightness = 0.75
// gain = 0
// dimmed = 0.375

#include <xer/interval.h>
#include <xer/stdio.h>

auto main() -> int
{
    using component = xer::interval<float>;

    const auto r = component(1.25f);
    const auto g = component(0.5f);
    const auto b = component(-0.25f);

    if (!xer::printf(u8"r = %g\n", static_cast<double>(r.value()))
             .has_value()) {
        return 1;
    }
    if (!xer::printf(u8"g = %g\n", static_cast<double>(g.value()))
             .has_value()) {
        return 1;
    }
    if (!xer::printf(u8"b = %g\n", static_cast<double>(b.value()))
             .has_value()) {
        return 1;
    }

    auto brightness = component(0.5f);
    brightness += 0.25f;

    if (!xer::printf(
             u8"brightness = %g\n",
             static_cast<double>(brightness.value()))
             .has_value()) {
        return 1;
    }

    using gain = xer::interval<float, -1.0f, 1.0f>;

    const auto center = gain::from_ratio(0.5f);

    if (!xer::printf(u8"gain = %g\n", static_cast<double>(center.value()))
             .has_value()) {
        return 1;
    }

    const auto dimmed = 0.5f * brightness;

    if (!xer::printf(u8"dimmed = %g\n", static_cast<double>(dimmed.value()))
             .has_value()) {
        return 1;
    }

    return 0;
}

// XER_EXAMPLE_END: interval_basic

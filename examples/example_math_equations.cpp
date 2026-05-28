// XER_EXAMPLE_BEGIN: math_equations
//
// This example computes a triangle area, converts coordinates, and solves quadratic and cubic equations for real roots.
//
// Expected output:
// heron: 6
// polar: 5
// cartesian: 3 4
// quadratic: 2 3
// cubic: 1 2 3

#include <iostream>

#include <xer/math.h>

namespace {

template<class T, std::size_t N>
auto print_roots(const char* label, const std::array<std::optional<T>, N>& roots) -> void
{
    std::cout << label << ':';

    for (const auto& root : roots) {
        if (root) {
            std::cout << ' ' << *root;
        }
    }

    std::cout << '\n';
}

} // namespace

auto main() -> int
{
    const auto area = xer::heron(3.0, 4.0, 5.0);
    if (!area) {
        return 1;
    }

    std::cout << "heron: " << *area << '\n';


    const auto p = xer::to_polar(xer::vec<double>{3.0, 4.0});
    std::cout << "polar: " << p.r << '\n';

    const auto v = xer::to_cartesian(p);
    std::cout << "cartesian: " << std::round(v.x) << ' ' << std::round(v.y) << '\n';

    const auto q = xer::quadratic(1.0, -5.0, 6.0);
    if (!q) {
        return 1;
    }

    print_roots("quadratic", *q);

    const auto c = xer::cubic(1.0, -6.0, 11.0, -6.0);
    if (!c) {
        return 1;
    }

    print_roots("cubic", *c);

    return 0;
}

// XER_EXAMPLE_END: math_equations

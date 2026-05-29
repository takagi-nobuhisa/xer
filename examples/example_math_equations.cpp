// XER_EXAMPLE_BEGIN: math_equations
//
// This example computes a triangle area, converts coordinates, and solves quadratic and cubic equations for real roots.
//
// Expected output:
// heron: 6
// sin: 1
// atan2: 0.25
// polar: 5
// cartesian: 3 4
// dot: 11
// length: 5
// distance: 5
// normalize: 0.6 0.8
// angle: 0.25
// rotate: 0 1
// cross: 0 0 1
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
    std::cout << "sin: " << std::round(xer::sin(0.25)) << '\n';
    std::cout << "atan2: " << xer::atan2(1.0, 0.0) << '\n';

    const auto p = xer::to_polar(xer::vec<double>{3.0, 4.0});
    std::cout << "polar: " << p.r << '\n';

    const auto v = xer::to_cartesian(p);
    std::cout << "cartesian: " << std::round(v.x) << ' ' << std::round(v.y) << '\n';

    const auto a = xer::vec<int>{1, 2};
    const auto b = xer::vec<int>{3, 4};
    std::cout << "dot: " << xer::dot(a, b) << '\n';
    std::cout << "length: " << xer::length(xer::vec<int>{3, 4}) << '\n';
    std::cout << "distance: " << xer::distance(xer::vec<int>{1, 2}, xer::vec<int>{4, 6}) << '\n';

    const auto n = xer::normalize(xer::vec<int>{3, 4});
    if (!n) {
        return 1;
    }
    std::cout << "normalize: " << n->x << ' ' << n->y << '\n';

    const auto angle = xer::angle(xer::vec<int>{1, 0}, xer::vec<int>{0, 1});
    if (!angle) {
        return 1;
    }
    std::cout << "angle: " << *angle << '\n';

    const auto rotated = xer::rotate(xer::vec<int>{1, 0}, xer::cyclic<double>(*angle));
    std::cout << "rotate: " << std::round(rotated.x) << ' ' << std::round(rotated.y) << '\n';

    const auto cross = xer::cross(xer::vec<int, 3>{1, 0, 0}, xer::vec<int, 3>{0, 1, 0});
    std::cout << "cross: " << cross.x << ' ' << cross.y << ' ' << cross.z << '\n';

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

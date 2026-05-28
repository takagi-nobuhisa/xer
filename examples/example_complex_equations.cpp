// XER_EXAMPLE_BEGIN: complex_equations
//
// This example solves equations for complex roots.
//
// Expected output:
// quadratic: (0,-1) (0,1)

#include <complex>
#include <iostream>

#include <xer/complex.h>

namespace {

template<class T>
auto clean_zero(T value) -> T
{
    return value == T{} ? T{} : value;
}

template<class T, std::size_t N>
auto print_complex_roots(const char* label, const std::array<std::complex<T>, N>& roots) -> void
{
    std::cout << label << ':';

    for (const auto& root : roots) {
        std::cout << " (" << clean_zero(root.real()) << ',' << clean_zero(root.imag()) << ')';
    }

    std::cout << '\n';
}

} // namespace

auto main() -> int
{
    const auto q = xer::cquadratic(1.0, 0.0, 1.0);
    if (!q) {
        return 1;
    }

    print_complex_roots("quadratic", *q);

    return 0;
}

// XER_EXAMPLE_END: complex_equations

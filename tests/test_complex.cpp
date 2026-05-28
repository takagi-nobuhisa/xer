#include <complex>

#include <xer/assert.h>
#include <xer/complex.h>
#include <xer/error.h>
#include <xer/math.h>

namespace {

template<class T>
auto near_complex(std::complex<T> lhs, std::complex<T> rhs) -> bool
{
    return xer::detail::equation_near(lhs.real(), rhs.real())
        && xer::detail::equation_near(lhs.imag(), rhs.imag());
}

void test_cquadratic_real_roots()
{
    const auto roots = xer::cquadratic(1.0, -5.0, 6.0);

    xer_assert(roots.has_value());
    xer_assert(near_complex(roots->at(0), std::complex<double>{2.0, 0.0}));
    xer_assert(near_complex(roots->at(1), std::complex<double>{3.0, 0.0}));
}

void test_cquadratic_complex_roots()
{
    const auto roots = xer::cquadratic(1.0, 0.0, 1.0);

    xer_assert(roots.has_value());
    xer_assert(near_complex(roots->at(0), std::complex<double>{0.0, -1.0}));
    xer_assert(near_complex(roots->at(1), std::complex<double>{0.0, 1.0}));
}

void test_cquadratic_invalid_argument()
{
    const auto roots = xer::cquadratic(0.0, 2.0, 1.0);

    xer_assert_not(roots.has_value());
    xer_assert(roots.error().code == xer::error_t::invalid_argument);
}

void test_ccubic_real_roots()
{
    const auto roots = xer::ccubic(1.0, -6.0, 11.0, -6.0);

    xer_assert(roots.has_value());
    xer_assert(near_complex(roots->at(0), std::complex<double>{3.0, 0.0}));
    xer_assert(near_complex(roots->at(1), std::complex<double>{1.0, 0.0}));
    xer_assert(near_complex(roots->at(2), std::complex<double>{2.0, 0.0}));
}

void test_ccubic_one_real_two_complex_roots()
{
    const auto roots = xer::ccubic(1.0, 0.0, 0.0, -1.0);

    xer_assert(roots.has_value());
    xer_assert(near_complex(roots->at(0), std::complex<double>{1.0, 0.0}));
}

void test_ccubic_invalid_argument()
{
    const auto roots = xer::ccubic(0.0, 1.0, 2.0, 3.0);

    xer_assert_not(roots.has_value());
    xer_assert(roots.error().code == xer::error_t::invalid_argument);
}

} // namespace

auto main() -> int
{
    test_cquadratic_real_roots();
    test_cquadratic_complex_roots();
    test_cquadratic_invalid_argument();
    test_ccubic_real_roots();
    test_ccubic_one_real_two_complex_roots();
    test_ccubic_invalid_argument();
    return 0;
}

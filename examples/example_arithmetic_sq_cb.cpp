#include <xer/arithmetic.h>
#include <xer/stdio.h>

auto main() -> int
{
    // XER_EXAMPLE_BEGIN: arithmetic_sq_cb
    //
    // This example shows square and cube arithmetic helpers.
    //
    // Expected output:
    // sq(12) = 144
    // cb(-3) = -27
    // sq(1.5) = 2.25
    // sq(add(2, 3)) = 25

    const auto square = xer::sq(12);
    if (!square.has_value()) {
        return 1;
    }

    const auto cube = xer::cb(-3);
    if (!cube.has_value()) {
        return 1;
    }

    const auto floating_square = xer::sq(1.5);
    if (!floating_square.has_value()) {
        return 1;
    }

    const auto chained = xer::sq(xer::add(2, 3));
    if (!chained.has_value()) {
        return 1;
    }

    if (!xer::printf(u8"sq(12) = %lld\n", *square).has_value()) {
        return 1;
    }

    if (!xer::printf(u8"cb(-3) = %lld\n", *cube).has_value()) {
        return 1;
    }

    if (!xer::printf(u8"sq(1.5) = %Lg\n", *floating_square).has_value()) {
        return 1;
    }

    if (!xer::printf(u8"sq(add(2, 3)) = %lld\n", *chained).has_value()) {
        return 1;
    }

    // XER_EXAMPLE_END: arithmetic_sq_cb

    return 0;
}
